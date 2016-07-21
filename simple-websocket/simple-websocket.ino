/*

  simple-websocket.ino

# Upload des fichiers sur l’ESP8266 avec requête POST
cd data
curl -F "image=@index.html" http://192.168.1.131/upload
curl -F "image=@img1.jpg" http://192.168.1.131/upload


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
ESP8266WebServer server    = ESP8266WebServer( 80 );
WebSocketsServer webSocket = WebSocketsServer( 81 );


const int LED_RED  = D0; // GPIO 16;
const int LED_BLUE = D4; // GPIO 2;
#define LED_ON  LOW
#define LED_OFF HIGH























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
  if(server.hasArg("download")) return "application/octet-stream";
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
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/upload") return;
  HTTPUpload& upload = server.upload();
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
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
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
  server.send(200, "text/json", output);
}

//
// Gestion du système de fichiers (fin)
//





























void clignoteLED()
{
  for( unsigned long i=0; i<2; i++ )
  {
    digitalWrite( LED_BLUE, LED_ON  );
    delay( 60 );
    digitalWrite( LED_BLUE, LED_OFF );
    delay( 60 );
  }

  for( unsigned long i=0; i<2; i++ )
  {
    digitalWrite( LED_RED, LED_ON  );
    delay( 60 );
    digitalWrite( LED_RED, LED_OFF );
    delay( 60 );
  }
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
      webSocket.sendTXT( num, "Connected" );
    }
    break;

  // Commande reçue
  case WStype_TEXT:
    Serial.printf( "length : %d\n", length );
    char msgConf[ length + 15 ];
    sprintf( msgConf, "[%u] get Text: %s\n", num, payload );
    Serial.print( msgConf );
    webSocket.sendTXT( num, msgConf );

    if(payload[0] == '#')
    {
      // we get RGB data

      // decode rgb data
      uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

      // analogWrite(LED_RED,  ((rgb >> 16) & 0xFF) );
      // analogWrite(LED_BLUE, ((rgb >> 0) & 0xFF)  );

      long RR = ((rgb >> 16) & 0xFF);
      long BB = ((rgb >> 0) & 0xFF);

      Serial.print( "RR = " );
      Serial.println( RR );
      Serial.print( "BB = " );
      Serial.println( BB );
      if( RR > 127 )
        digitalWrite( LED_RED, LOW  );
      else
        digitalWrite( LED_RED, HIGH );
      if( BB > 127 )
        digitalWrite( LED_BLUE, LOW  );
      else
        digitalWrite( LED_BLUE, HIGH );
    }
    break;
  }
}


void setup()
{
  // Initialisation du port série
  Serial.begin( 115200 );
  Serial.print( "\n\nDEMO WEBSOCKET\n==============\n\n" );

  // Initialisation des LED
  pinMode( LED_RED,  OUTPUT );
  pinMode( LED_BLUE, OUTPUT );
  digitalWrite( LED_BLUE, LED_OFF );
  digitalWrite( LED_RED,  LED_OFF );
  clignoteLED();

  // Initialisation du système de fichiers
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

  // Initialisation de la connexion WiFi
  WiFiMulti.addAP( ssid, password );
  while( WiFiMulti.run() != WL_CONNECTED )
  {
    delay( 100 );
    Serial.print( "." );
  }

  // Démarrage du serveur WebSocket
  webSocket.begin();
  webSocket.onEvent( webSocketEvent );

  // Démarrage du mDNS
  if( MDNS.begin( "esp8266" ) )
    { Serial.println( "\nmDNS OK\n" ); }

  // Sert le fichier index.html
  server.on( "/", []()
  {
    if(!handleFileRead("/index.html")) server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/list", HTTP_GET, handleFileList);

  server.on("/upload", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  // Pour toutes les autres URL.
  server.onNotFound([]()
  {
    if( !handleFileRead( server.uri() ) )
      server.send( 404, "text/plain", "FileNotFound" );
  });

  // Démarrage du serveur
  server.begin();

  // Ajoute les services au mDNS
  MDNS.addService( "http", "tcp", 80 );
  MDNS.addService( "ws",   "tcp", 81 );

  // Setup terminé
  Serial.print( "Setup OK\nAdresse IP : " );
  Serial.println( WiFi.localIP() );
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}


