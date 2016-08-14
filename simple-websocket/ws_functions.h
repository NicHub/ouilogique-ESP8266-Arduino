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

#define ESP_MODULE_TYPE 'ESP-12E'
#if ESP_MODULE_TYPE == 'ESP-01'
static const uint8_t LED_RED  = 0; // GPIO 0;
static const uint8_t LED_BLUE = 2; // GPIO 2;
#elif ESP_MODULE_TYPE == 'ESP-12E'
static const uint8_t LED_RED  = D0; // GPIO 16;
static const uint8_t LED_BLUE = D4; // GPIO 2;
#endif
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
  //ESP.getVcc() ⇒ may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. ⇒ https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#user-content-esp-specific-apis
  Serial.printf( "ESP.getFreeHeap()              : %d\n",   ESP.getFreeHeap() );   //  returns the free heap size.
  Serial.printf( "ESP.getChipId()                : 0x%X\n", ESP.getChipId() );   //  returns the ESP8266 chip ID as a 32-bit integer.
  Serial.printf( "ESP.getSdkVersion()            : %d\n",   ESP.getSdkVersion() );
  Serial.printf( "ESP.getBootVersion()           : %d\n",   ESP.getBootVersion() );
  Serial.printf( "ESP.getBootMode()              : %d\n",   ESP.getBootMode() );
  Serial.printf( "ESP.getCpuFreqMHz()            : %d\n",   ESP.getCpuFreqMHz() );
  Serial.printf( "ESP.getFlashChipId()           : 0x%X\n", ESP.getFlashChipId() );
  Serial.printf( "ESP.getFlashChipRealSize()     : %d\n",   ESP.getFlashChipRealSize() );
  Serial.printf( "ESP.getFlashChipSize()         : %d\n",   ESP.getFlashChipSize() );  //returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  Serial.printf( "ESP.getFlashChipSpeed()        : %d\n",   ESP.getFlashChipSpeed() ); // returns the flash chip frequency, in Hz.
  Serial.printf( "ESP.getFlashChipMode()         : %d\n",   ESP.getFlashChipMode() );
  Serial.printf( "ESP.getFlashChipSizeByChipId() : 0x%X\n", ESP.getFlashChipSizeByChipId() );
  Serial.printf( "ESP.getSketchSize()            : %d\n",   ESP.getSketchSize() );
  Serial.printf( "ESP.getFreeSketchSpace()       : %d\n",   ESP.getFreeSketchSpace() );
  Serial.printf( "ESP.getCycleCount()            : %d\n",   ESP.getCycleCount() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.

  Serial.flush(); // Pour attendre que tous les caractères soient envoyés
}

void initSystemeFichiers()
{
  Serial.print( "Initialisation du systeme de fichiers\n" );
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
    Serial.print( "Fin de l'initialisation du systeme de fichiers\n" );
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
Exemple : TimeNTP_ESP8266WiFi.ino
*/
#include <TimeLib.h>
#include <WiFiUdp.h>


// NTP Servers:

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address

// Le serveur NTP d’Apple est le plus rapide de tous ceux que j’ai testés.
// http://www.pool.ntp.org/fr/
// const char* ntpServerName = "ch.pool.ntp.org";
// const char* ntpServerName = "time.nist.gov";
// const char* ntpServerName = "ntp.metas.ch";
const char* ntpServerName = "time.apple.com";

const int timeZone = 2;     // Central European Time

WiFiUDP Udp;
unsigned int localPort = 2390;  // local port to listen for UDP packets


time_t prevDisplay = 0; // when the digital clock was displayed

char *getNTPTime() {
  static char jsonMsg[ 32 ] = "-";
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      sprintf( jsonMsg,
        "{\"TIME\":\"%02d:%02d:%02d %02d-%02d-%04d\"}",
        hour(),
        minute(),
        second(),
        day(),
        month(),
        year()
      );
      Serial.printf( "L'HEURE ET LA DATE SONT : %s\n", jsonMsg );
    }
  }
  return jsonMsg;
}



/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


time_t _getNtpTime()
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);
  Serial.print( "ntpServerName = " );
  Serial.println( ntpServerName );
  Serial.print( "timeServerIP = " );
  Serial.println( timeServerIP );

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}


void udpInit() {
  Udp.begin(localPort);
  setSyncProvider(_getNtpTime);
}

#endif //  WS_FUNCTIONS_H
