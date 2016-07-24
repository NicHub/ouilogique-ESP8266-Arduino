/*
*/

#ifndef WS_FUNCTIONS_H
#define WS_FUNCTIONS_H



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
extern const char* mDNSName;


void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length );






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






























// Envoie l’état des GPIO au format JSON
void WSsendGPIOStates( uint8_t num )
{
  char jsonMsg[ 120 ];
  sprintf( jsonMsg,
    "{\"GPIO\":{\"0\":\"%d\",\"1\":\"%d\",\"2\":\"%d\",\"3\":\"%d\",\"4\":\"%d\",\"5\":\"%d\",\"9\":\"%d\",\"10\":\"%d\",\"12\":\"%d\",\"13\":\"%d\",\"14\":\"%d\",\"15\":\"%d\",\"16\":\"%d\"}}",
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
















/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 updated for the ESP8266 12 Apr 2015
 by Ivan Grokhotkov

 This code is in the public domain.

 */

 #include <WiFiUdp.h>

 unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
// const char* ntpServerName = "time.nist.gov";
// const char* ntpServerName = "ntp.metas.ch";
const char* ntpServerName = "time.apple.com";


const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;




// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

char *getNTPTime()
{
  // Réponse à envoyer à l’ESP
  static char jsonMsg[ 32 ] = "-";

  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    unsigned long TimeZoneUTC = 2;
    sprintf( jsonMsg,
      "{\"TIME\":\"%02d:%02d:%02d\"}",
      ( epoch % 86400L ) / 3600 + TimeZoneUTC,
      ( epoch %   3600 ) / 60,
      ( epoch %     60 )
    );
    Serial.printf( "L'HEURE EST : %s\n", jsonMsg );

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  return jsonMsg;
}



#endif //  WS_FUNCTIONS_H
