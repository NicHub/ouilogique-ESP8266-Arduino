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
#include <ArduinoJson.h>


#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <FS.h>

String activeSSID;
String activePASSWORD;


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
static const uint8_t RESET_PIN = 5; // GPIO 5;
#endif
#define LED_SET   LOW
#define LED_CLEAR HIGH
extern const char* mDNSName;


void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length );





void onFaitUnePause( unsigned long attente )
{
  unsigned long Tf = millis() + attente;
  Serial.printf( "# ON FAIT UNE PAUSE DE %d ms\n\t", attente );
  while( millis() < Tf )
  {
    static byte count = 0;
    if( ++count > 100 )
    {
      Serial.print( "\n\t" );
      count = 1;
    }
    Serial.print( "." );
    Serial.flush();
    digitalWrite( LED_BLUE,  LED_SET );
    delay( 1 );
    digitalWrite( LED_BLUE,  LED_CLEAR );
    delay( 99 );
  }
  Serial.print( "\n\n" );
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

String getContentType( String filename )
{
  if( webServer.hasArg( "download" ) )    return "application/octet-stream";
  else if( filename.endsWith( ".htm"  ) ) return "text/html";
  else if( filename.endsWith( ".html" ) ) return "text/html";
  else if( filename.endsWith( ".css"  ) ) return "text/css";
  else if( filename.endsWith( ".js"   ) ) return "application/javascript";
  else if( filename.endsWith( ".png"  ) ) return "image/png";
  else if( filename.endsWith( ".gif"  ) ) return "image/gif";
  else if( filename.endsWith( ".jpg"  ) ) return "image/jpeg";
  else if( filename.endsWith( ".ico"  ) ) return "image/x-icon";
  else if( filename.endsWith( ".xml"  ) ) return "text/xml";
  else if( filename.endsWith( ".pdf"  ) ) return "application/x-pdf";
  else if( filename.endsWith( ".zip"  ) ) return "application/x-zip";
  else if( filename.endsWith( ".gz"   ) ) return "application/x-gzip";
  else if( filename.endsWith( ".ini"  ) ) return "text/plain";
  else return "text/plain";
}

bool handleFileRead(String path)
{
  Serial.println( "handleFileRead: " + path );
  if( path.endsWith( "/" ) ) path += "index.html";
  String contentType = getContentType( path );
  String pathWithGz = path + ".gz";
  if( SPIFFS.exists( pathWithGz ) || SPIFFS.exists( path ) )
  {
    if( SPIFFS.exists( pathWithGz ) )
      path += ".gz";
    File file = SPIFFS.open( path, "r" );
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
  char jsonMsg[ 147 ];
  sprintf( jsonMsg,
    "{\"GPIO\":{\"GPIO0\":%d,\"GPIO1\":%d,\"GPIO2\":%d,\"GPIO3\":%d,\"GPIO4\":%d,\"GPIO5\":%d,\"GPIO9\":%d,\"GPIO10\":%d,\"GPIO12\":%d,\"GPIO13\":%d,\"GPIO14\":%d,\"GPIO15\":%d,\"GPIO16\":%d}}",
    digitalRead(  0 ),
    digitalRead(  1 ),
    digitalRead(  2 ),
    digitalRead(  3 ),
    digitalRead(  4 ),
    digitalRead(  5 ),
    // Pas de GPIO 6, 7, 8
    digitalRead(  9 ),
    digitalRead( 10 ),
    // Pas de GPIO 11
    digitalRead( 12 ),
    digitalRead( 13 ),
    digitalRead( 14 ),
    digitalRead( 15 ),
    digitalRead( 16 )
  );

  Serial.printf( "%s\n", jsonMsg );
  webSocket.sendTXT( num, jsonMsg );

}

void printESPInfo()
{
  Serial.print( "# ESP INFO\n" );

  //ESP.getVcc() ⇒ may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. ⇒ https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#user-content-esp-specific-apis
  Serial.printf( "\tESP.getFreeHeap()              : %d\n",   ESP.getFreeHeap() );   //  returns the free heap size.
  Serial.printf( "\tESP.getChipId()                : 0x%X\n", ESP.getChipId() );   //  returns the ESP8266 chip ID as a 32-bit integer.
  Serial.printf( "\tESP.getSdkVersion()            : %d\n",   ESP.getSdkVersion() );
  Serial.printf( "\tESP.getBootVersion()           : %d\n",   ESP.getBootVersion() );
  Serial.printf( "\tESP.getBootMode()              : %d\n",   ESP.getBootMode() );
  Serial.printf( "\tESP.getCpuFreqMHz()            : %d\n",   ESP.getCpuFreqMHz() );
  Serial.printf( "\tESP.getFlashChipId()           : 0x%X\n", ESP.getFlashChipId() );
  Serial.printf( "\tESP.getFlashChipRealSize()     : %d\n",   ESP.getFlashChipRealSize() );
  Serial.printf( "\tESP.getFlashChipSize()         : %d\n",   ESP.getFlashChipSize() );  //returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  Serial.printf( "\tESP.getFlashChipSpeed()        : %d\n",   ESP.getFlashChipSpeed() ); // returns the flash chip frequency, in Hz.
  Serial.printf( "\tESP.getFlashChipMode()         : %d\n",   ESP.getFlashChipMode() );
  Serial.printf( "\tESP.getFlashChipSizeByChipId() : 0x%X\n", ESP.getFlashChipSizeByChipId() );
  Serial.printf( "\tESP.getSketchSize()            : %d\n",   ESP.getSketchSize() );
  Serial.printf( "\tESP.getFreeSketchSpace()       : %d\n",   ESP.getFreeSketchSpace() );
  Serial.printf( "\tESP.getCycleCount()            : %d\n",   ESP.getCycleCount() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.

  Serial.print( "\n\n" );
  Serial.flush(); // Pour attendre que tous les caractères soient envoyés
}

void initSystemeFichiers()
{
  Serial.print( "# INITIALISATION DU SYSTEME DE FICHIERS\n" );
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir( "/" );
    while( dir.next() )
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf(
        "\t%s, %s\n",
        fileName.c_str(), formatBytes(fileSize).c_str()
      );
    }
    Serial.print( "\n" );
  }
}

void inverseBubbleSortIndexes( int inputArray[], int indexes[], int arraySize )
{
  for( int i=0; i<arraySize; i++ )
    indexes[ i ] = i;

  for( int i=0; i<arraySize-1; i++ )
  {
    for( int j=0; j<arraySize-1-i; j++ )
    {
      if( inputArray[ j ] < inputArray[ j+1 ] )
      {
        int temp = inputArray[ j+1 ];
        inputArray[ j+1 ] = inputArray[ j ];
        inputArray[ j ] = temp;
        int tempI = indexes[ j+1 ];
        indexes[ j+1 ] = indexes[ j ];
        indexes[ j ] = tempI;
      }
    }
  }
}

void scanNetwork()
{
  Serial.println( "# SCAN DU RESEAU" );
  WiFi.disconnect();

  // WiFi.scanNetworks will return the number of networks found
  int nbNetworkFound = WiFi.scanNetworks();
  if( nbNetworkFound == 0 )
    Serial.println( "\tno networks found" );
  else
  {
    Serial.print( "\t## " );
    Serial.print( nbNetworkFound );
    Serial.println( " NETWORKS FOUND" );

    // Trie les SSID par ordre croissant de force de signal
    int RSSIarray[ nbNetworkFound ];
    int indexes[ nbNetworkFound ];
    for( int i=0; i<nbNetworkFound; i++ )
      RSSIarray[ i ] = WiFi.RSSI( i );
    inverseBubbleSortIndexes( RSSIarray, indexes, nbNetworkFound );

    for( int i=0; i<nbNetworkFound; i++ )
    {
      // Print SSID and RSSI for each network found
      Serial.print( "\t" );
      if( i<9 ) Serial.print( " " );
      Serial.print( i + 1 );
      Serial.print( ": (" );
      Serial.print( WiFi.RSSI( indexes[ i ] ) );
      Serial.print( " dB) " );
      Serial.print( WiFi.SSID( indexes[ i ] ) );
      Serial.println( ( WiFi.encryptionType( indexes[ i ] ) == ENC_TYPE_NONE ) ? " (unprotected)" : "" );
      Serial.flush();
    }
  }
  Serial.println( "" );
}

void initServicesWeb()
{
  // Initialisation de la connexion WiFi
  Serial.println( "# INITIALISATION DE LA CONNEXION WIFI" );

  byte nbChar;
  nbChar = activeSSID.length();
  char ssid[ nbChar + 1 ];
  ssid[ nbChar + 1 ] = '\0';
  activeSSID.toCharArray( ssid, nbChar+1 );
  nbChar = activePASSWORD.length();
  char password[ nbChar + 1 ];
  password[ nbChar + 1 ] = '\0';
  activePASSWORD.toCharArray( password, nbChar+1 );

  Serial.println( "\tactiveSSID initWeb     : " + String( ssid ) );
  Serial.println( "\tactivePASSWORD initWeb : " + String( password ) );

  WiFi.persistent( false );
  WiFi.mode( WIFI_STA );
  WiFiMulti.addAP( ssid, password );

  Serial.print( "\t" );
  while( WiFiMulti.run() != WL_CONNECTED )
  {
    static byte count = 0;
    if( ++count > 100 )
    {
      Serial.print( "\n\t" );
      count = 1;
    }
    Serial.print( "." );
    Serial.flush();
    digitalWrite( LED_RED,  LED_SET );
    delay( 1 );
    digitalWrite( LED_RED,  LED_CLEAR );
    delay( 99 );
  }
  Serial.flush();

  // Démarrage du serveur WebSocket
  Serial.print( "\n# DEMARRAGE DU SERVEUR WEBSOCKET\n" );
  webSocket.begin();
  webSocket.onEvent( webSocketEvent );

  // Démarrage du mDNS (multicast Domain Name System)
  Serial.printf( "\tDemarrage du mDNS avec le nom '%s.local' .. ", mDNSName );
  if( MDNS.begin( mDNSName ) )
    { Serial.print( "OK\n" ); }
  else
    { Serial.print( "PAS OK\n" ); }
  Serial.flush();

  // Chargement du fichier “index.html”
  Serial.print( "\tChargement du fichier 'index.html'\n" );
  webServer.on( "/", []()
  {
    if( ! handleFileRead( "/index.html" ) ) webServer.send( 404, "text/plain", "FileNotFound" );
  });

  // Gestion de l’URL “/list”
  Serial.print( "\tGestion de l'URL '/list'\n" );
  webServer.on( "/list", HTTP_GET, handleFileList );

  // Gestion de l’URL “/upload”
  Serial.print( "\tGestion de l'URL '/upload'\n" );
  webServer.on( "/upload", HTTP_POST, [](){ webServer.send( 200, "text/plain", "" ); }, handleFileUpload );

  // Pour toutes les autres URL.
  Serial.print( "\tGestion des autres URL\n" );
  webServer.onNotFound([]()
  {
    if( !handleFileRead( webServer.uri() ) )
      webServer.send( 404, "text/plain", "FileNotFound" );
  });

  // Démarrage du serveur web
  Serial.print( "\tDemarrage du serveur web\n" );
  webServer.begin();

  // Ajout des services HTTP et WebSocket au mDNS
  Serial.print( "\tAjout des services HTTP et WebSocket au mDNS\n" );
  MDNS.addService( "http", "tcp", 80 );
  MDNS.addService( "ws",   "tcp", 81 );

  // Setup terminé
  Serial.print( "\tSetup OK\n\tAdresse IP : " );
  Serial.println( WiFi.localIP() );
  Serial.print( "\n" );
  Serial.flush();
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
      Serial.printf( "\tHeure et date        : %s\n", jsonMsg );
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
  Serial.println( "# LECTURE DE L’HEURE COURANTE SUR UN SERVEUR NTP" );
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);
  Serial.print( "\tntpServerName        : " );
  Serial.println( ntpServerName );
  Serial.print( "\ttimeServerIP         : " );
  Serial.println( timeServerIP );

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("\tTransmit NTP Request");
  sendNTPpacket(timeServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("\tReceive NTP Response");
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
  Serial.println("\tNo NTP Response :-(");
  return 0; // return 0 if unable to get the time
}


void udpInit() {
  Udp.begin(localPort);
  setSyncProvider(_getNtpTime);
}














/*

*/

bool parseJSONwifiSettingsFromFile()
{

  // Lit l’index des paramètres de la connexion Wifi active
  String path1 = String( "/wifisettingsactiveindex.ini" );
  Serial.println( "# LECTURE DES INFORMATIONS DE CONNEXION" );
  Serial.println( "\tfichier           : " + path1 );

  File cFile1 = SPIFFS.open( path1, "r" );
  String wifiSettingsActiveIndex = cFile1.readStringUntil( '\n' );
  cFile1.close();

  byte activeIndex = wifiSettingsActiveIndex.toInt();
  Serial.println( "\tindice actif      : " + String( activeIndex ) );

  // Lit la liste des paramètre de connexion Wifi
  String path = String( "/wifisettings.json" );
  Serial.println( "\tfichier           : " + path );

  File cFile = SPIFFS.open( path, "r" );
  String jsonWifiSettings = cFile.readStringUntil( ']' ) + "]";
  cFile.close();

  unsigned long nbChar = jsonWifiSettings.length();
  Serial.println( "\tnb caracteres lus : " + String( nbChar ) );

  // Voir https://bblanchon.github.io/ArduinoJson/
  const size_t bufferSize = JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(2) + 290;
  DynamicJsonBuffer jsonBuffer( bufferSize );
  char json[ nbChar+1 ];
  jsonWifiSettings.toCharArray( json, nbChar+1 );
  JsonArray& root = jsonBuffer.parseArray(json);

  String SSIDs[ 10 ]     = { root[ 0 ][ "SSID" ], root[ 1 ][ "SSID" ], root[ 2 ][ "SSID" ], root[ 3 ][ "SSID" ], root[ 4 ][ "SSID" ], root[ 5 ][ "SSID" ], root[ 6 ][ "SSID" ], root[ 7 ][ "SSID" ], root[ 8 ][ "SSID" ], root[ 9 ][ "SSID" ] };
  String PASSWORDs[ 10 ] = { root[ 0 ][ "PASSWORD" ], root[ 1 ][ "PASSWORD" ], root[ 2 ][ "PASSWORD" ], root[ 3 ][ "PASSWORD" ], root[ 4 ][ "PASSWORD" ], root[ 5 ][ "PASSWORD" ], root[ 6 ][ "PASSWORD" ], root[ 7 ][ "PASSWORD" ], root[ 8 ][ "PASSWORD" ], root[ 9 ][ "PASSWORD" ] };

  Serial.println( "\tSSID final        : " + String( SSIDs[ activeIndex ] ) );
  nbChar = SSIDs[ activeIndex ].length();
  Serial.println( "\tnbChar SSID final : " + String( nbChar ) + "\n" );

  activeSSID     = SSIDs[ activeIndex ];
  activePASSWORD = PASSWORDs[ activeIndex ];

  return true;
}







void wifiSettingsFileRead( uint8_t num )
{
  String path = String( "/wifisettings.json" );
  Serial.println( "wifiSettingsFileRead: " + path );
  webSocket.sendTXT( num, "{\"COUCOU\":\"c'estmoi\"}" );

  File cFile = SPIFFS.open( path, "r" );
  String s=cFile.readStringUntil( ']' ) + "]";
  Serial.println( s );
  webSocket.sendTXT( num, s );
  cFile.close();
}



#endif //  WS_FUNCTIONS_H
