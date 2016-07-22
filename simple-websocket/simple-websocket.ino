/*

SIMPLE-WEBSOCKET.INO
====================

  Ce croquis utilise la bibliothèque “arduinoWebSockets 2.0.2” de Markus Sattler.
  https://github.com/Links2004/arduinoWebSockets.git
  Cette bibliothèque peut être installée directement dans le gestionnaire de bibliothèque de l’IDE Arduino.

  Ce croquis est basé sur les exemples suivants :
  https://github.com/Links2004/arduinoWebSockets/blob/master/examples/WebSocketServer_LEDcontrol/WebSocketServer_LEDcontrol.ino
  https://github.com/AdySan/ESPSocket/blob/master/ESPSocket/ESPSocket.ino

# MICROCONTRÔLEUR
  ESP8266 Amica

# NOTE
  Pour ne pas mettre à jour le fichier “WifiSettings.h” dans Git utiliser
  git update-index --skip-worktree WifiSettings.h
  Et pour le mettre à jour (pas recommandé)
  git update-index --no-skip-worktree WifiSettings.h






# Upload des fichiers sur l’ESP8266 avec requête POST
cd data
curl -F "image=@index.html" http://192.168.1.131/upload
curl -F "image=@websocket.js" http://192.168.1.131/upload
curl -F "image=@img1.jpg" http://192.168.1.131/upload



juin 2016, ouilogique.com

*/

// Le fichier WifiSettings.h doit être présent
// et contenir les instructions suivantes :
// const char* ssid     = "***";
// const char* password = "***";
#include "WifiSettings.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <FS.h>



ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer webServer = ESP8266WebServer( 80 );
WebSocketsServer webSocket = WebSocketsServer( 81 );


const int LED_RED  = D0; // GPIO 16;
const int LED_BLUE = D4; // GPIO 2;
#define LED_SET   LOW
#define LED_CLEAR HIGH
static const char* mDNSName = "esp8266";








void onFaitUnePause( unsigned long attente )
{
  unsigned long Tf = millis() + attente;
  Serial.flush();
  bool status = false;
  Serial.printf( "On fait une pause de %d ms ", attente );
  while( millis() < Tf )
  {
    digitalWrite( LED_BLUE,  status );
    digitalWrite( LED_RED, ! status );
    status = ! status;
    if( status ) { Serial.print( "." ); }
    delay( 60 );
  }
  Serial.print( " OK\n" );
  Serial.flush();
}













//
// Gestion du système de fichiers
//

//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(webServer.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = webServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(webServer.uri() != "/upload") return;
  HTTPUpload& upload = webServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(webServer.args() == 0) return webServer.send(500, "text/plain", "BAD ARGS");
  String path = webServer.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return webServer.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return webServer.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  webServer.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(webServer.args() == 0)
    return webServer.send(500, "text/plain", "BAD ARGS");
  String path = webServer.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return webServer.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return webServer.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return webServer.send(500, "text/plain", "CREATE FAILED");
  webServer.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!webServer.hasArg("dir")) {webServer.send(500, "text/plain", "BAD ARGS"); return;}

  String path = webServer.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  webServer.send(200, "text/json", output);
}

//
// Gestion du système de fichiers (fin)
//





























void clignoteLED()
{
  for( unsigned long i=0; i<2; i++ )
  {
    digitalWrite( LED_BLUE, LED_SET  );
    delay( 60 );
    digitalWrite( LED_BLUE, LED_CLEAR );
    delay( 60 );
  }

  for( unsigned long i=0; i<2; i++ )
  {
    digitalWrite( LED_RED, LED_SET  );
    delay( 60 );
    digitalWrite( LED_RED, LED_CLEAR );
    delay( 60 );
  }
}

// Envoie l’état des GPIO au format JSON
void WSsendGPIOStates( uint8_t num )
{
  char jsonMsg[ 134 ];
  sprintf( jsonMsg,
    "{\"GPIO\":{\"G0\":\"%d\",\"G1\":\"%d\",\"G2\":\"%d\",\"G3\":\"%d\",\"G4\":\"%d\",\"G5\":\"%d\",\"G9\":\"%d\",\"G10\":\"%d\",\"G12\":\"%d\",\"G13\":\"%d\",\"G14\":\"%d\",\"G15\":\"%d\",\"G16\":\"%d\"}}",
    !digitalRead(  0 ),
    !digitalRead(  1 ),
    !digitalRead(  2 ),
    !digitalRead(  3 ),
    !digitalRead(  4 ),
    !digitalRead(  5 ),
    // Pas de GPIO 6, 7, 8
    !digitalRead(  9 ),
    !digitalRead( 10 ),
    // Pas de GPIO 11
    !digitalRead( 12 ),
    !digitalRead( 13 ),
    !digitalRead( 14 ),
    !digitalRead( 15 ),
    !digitalRead( 16 )
  );

  Serial.printf( "%s\n", jsonMsg );
  webSocket.sendTXT( num, jsonMsg );

}

void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
{
  switch( type )
  {

  // Déconnexion
  case WStype_DISCONNECTED:
    Serial.printf( "[%u] Disconnected!\n", num );
    break;

  // Connexion
  case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP( num );
      Serial.printf( "[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload );

      // send message to client
      WSsendGPIOStates( num );
    }
    break;

  // Commande reçue
  case WStype_TEXT:
    Serial.printf( "length : %d\n", length );
    char msgConf[ length + 15 ];
    sprintf( msgConf, "[%u] get Text: %s\n", num, payload );
    Serial.print( msgConf );
    // webSocket.sendTXT( num, msgConf );

    if( payload[0] == '#' )
    {
      // we get RGB data

      // decode rgb data
      uint32_t rgb = (uint32_t) strtol( (const char *) &payload[1], NULL, 16 );

      // analogWrite(LED_RED,  ((rgb >> 16) & 0xFF) );
      // analogWrite(LED_BLUE, ((rgb >> 0) & 0xFF)  );

      long RR = ((rgb >> 16) & 0xFF);
      long BB = ((rgb >> 0) & 0xFF);

      Serial.print( "RR = " );
      Serial.println( RR );
      Serial.print( "BB = " );
      Serial.println( BB );
      if( RR > 127 )
        digitalWrite( LED_RED, LED_SET  );
      else
        digitalWrite( LED_RED, LED_CLEAR );
      if( BB > 127 )
        digitalWrite( LED_BLUE, LED_SET  );
      else
        digitalWrite( LED_BLUE, LED_CLEAR );
      delay( 100 );
      WSsendGPIOStates( num );
    }
    break;
  }
}

void printESPInfo()
{
  Serial.printf( "ESP CHIP ID          : %d\n", ESP.getChipId()         );
  Serial.printf( "ESP FREE HEAP        : %d\n", ESP.getFreeHeap()       );
  Serial.printf( "ESP FLASH CHIP ID    : %d\n", ESP.getFlashChipId()    );
  Serial.printf( "ESP FLASH CHIP SIZE  : %d\n", ESP.getFlashChipSize()  );
  Serial.printf( "ESP FLASH CHIP SPEED : %d\n", ESP.getFlashChipSpeed() );
  Serial.flush(); // Pour attendre que tous les caractères soient envoyés
}

void initSystemeFichiers()
{
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir( "/" );
    while( dir.next() )
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf(
        "FS File: %s, size: %s\n",
        fileName.c_str(), formatBytes(fileSize).c_str()
      );
    }
    Serial.printf("\n");
  }
}

void initServicesWeb()
{
  // Initialisation de la connexion WiFi
  Serial.print( "Initialisation de la connexion WiFi .." );
  WiFiMulti.addAP( ssid, password );
  while( WiFiMulti.run() != WL_CONNECTED )
  {
    Serial.print( "." );
    Serial.flush();
    delay( 100 );
  }
  Serial.print( " OK\n" );

  // Démarrage du serveur WebSocket
  Serial.print( "Demarrage du serveur WebSocket\n" );
  webSocket.begin();
  webSocket.onEvent( webSocketEvent );

  // Démarrage du mDNS (multicast Domain Name System)
  Serial.printf( "Demarrage du mDNS avec le nom '%s.local' .. ", mDNSName );
  if( MDNS.begin( mDNSName ) )
    { Serial.print( "OK\n" ); }
  else
    { Serial.print( "PAS OK\n" ); }

  // Chargement du fichier “index.html”
  Serial.print( "Chargement du fichier 'index.html'\n" );
  webServer.on( "/", []()
  {
    if( ! handleFileRead( "/index.html" ) ) webServer.send( 404, "text/plain", "FileNotFound" );
  });

  // Gestion de l’URL “/list”
  Serial.print( "Gestion de l'URL '/list'\n" );
  webServer.on( "/list", HTTP_GET, handleFileList );

  // Gestion de l’URL “/upload
  Serial.print( "Gestion de l'URL '/upload'\n" );
  webServer.on( "/upload", HTTP_POST, [](){ webServer.send( 200, "text/plain", "" ); }, handleFileUpload );

  // Pour toutes les autres URL.
  Serial.print( "Gestion des autres URL\n" );
  webServer.onNotFound([]()
  {
    if( !handleFileRead( webServer.uri() ) )
      webServer.send( 404, "text/plain", "FileNotFound" );
  });

  // Démarrage du serveur web
  Serial.print( "Demarrage du serveur web\n" );
  webServer.begin();

  // Ajout des services HTTP et WebSocket au mDNS
  Serial.print( "Ajout des services HTTP et WebSocket au mDNS\n" );
  MDNS.addService( "http", "tcp", 80 );
  MDNS.addService( "ws",   "tcp", 81 );

  // Setup terminé
  Serial.print( "Setup OK\nAdresse IP : " );
  Serial.println( WiFi.localIP() );
}

void setup()
{
  // Initialisation du port série
  Serial.begin( 115200 );
  Serial.print( "\n\nDEMO WEBSOCKET\n==============\n\n" );

  // Initialisation des LED
  pinMode( LED_RED,  OUTPUT );
  pinMode( LED_BLUE, OUTPUT );
  digitalWrite( LED_BLUE, LED_CLEAR );
  digitalWrite( LED_RED,  LED_CLEAR );
  onFaitUnePause( 4000 );

  // Initialisation du système de fichiers
  initSystemeFichiers();

  // Démarrage des services web
  initServicesWeb();

  // Affichage de quelques caractéristiques de l’ESP8266
  printESPInfo();

  // Fin de l’initialisation
  Serial.print( "Fin de l'initialisation\n" );
  Serial.print( "Adresse IP : " );
  Serial.println( WiFi.localIP() );
  Serial.printf( "Nom        : %s.local \n\n###\n\n", mDNSName );
}

void loop()
{
  webSocket.loop();
  webServer.handleClient();
}
