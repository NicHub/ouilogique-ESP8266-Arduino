/*

SIMPLE HTML INTERFACE
=====================

Ce croquis utilise la bibliothèque “arduinoWebSockets 2.0.2” de Markus Sattler.
https://github.com/Links2004/arduinoWebSockets.git
Cette bibliothèque peut être installée directement dans le gestionnaire de bibliothèque de l’IDE Arduino.

Ce croquis est basé sur l’exemple “WebSocketServer_LEDcontrol”
https://github.com/Links2004/arduinoWebSockets/tree/master/examples/WebSocketServer_LEDcontrol

# MICROCONTRÔLEUR
    ESP8266 Amica

juin 2016, ouilogique.com

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "WifiSettings.h"

static const char* HTML_DOC PROGMEM =
#include "index.html.h"
;

#define LEDrouge   D0 // GPIO 16;
#define LEDbleue   D4 // GPIO 2;

#define LEDallumee 0
#define LEDeteinte 1

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server    = ESP8266WebServer( 80 );
WebSocketsServer webSocket = WebSocketsServer( 81 );

void onFaitUnePause()
{
  USE_SERIAL.print( "On fait une pause de 4 s\n" );
  for( unsigned long i=0; i<10; i++ )
  {
    digitalWrite( LEDbleue, LEDallumee );
    delay( 100 );
    digitalWrite( LEDbleue, LEDeteinte );
    delay( 100 );
    USE_SERIAL.print( "." );
  }
  for( unsigned long i=0; i<10; i++ )
  {
    digitalWrite( LEDrouge, LEDallumee );
    delay( 100 );
    digitalWrite( LEDrouge, LEDeteinte );
    delay( 100 );
    USE_SERIAL.print( "." );
  }
}

void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
{
    IPAddress ip;

    switch( type )
    {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf( "[%u] Disconnected!\n", num );
      break;

    case WStype_CONNECTED:
      ip = webSocket.remoteIP( num );
      USE_SERIAL.printf( "[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload );
      // send message to client
      webSocket.sendTXT( num, "Connected !!!" );
      break;

    case WStype_TEXT:
      USE_SERIAL.printf( "[%u] get Text: %s\n", num, payload );
      if( payload[0] == '#' )
      {
        // we get RGB data

        // decode rgb data
        uint32_t rgb = (uint32_t) strtol( (const char *) &payload[1], NULL, 16 );

        // analogWrite(LEDrouge,  ((rgb >> 16) & 0xFF) );
        // analogWrite(LEDbleue, ((rgb >> 0) & 0xFF)  );

        long RR = ((rgb >> 16) & 0xFF);
        long BB = ((rgb >> 0) & 0xFF);

        Serial.print( "RR = " );
        Serial.println( RR );
        Serial.print( "BB = " );
        Serial.println( BB );
        if( RR > 127 )
          digitalWrite( LEDrouge, LEDallumee );
        else
          digitalWrite( LEDrouge, LEDeteinte );
        if( BB > 127 )
          digitalWrite( LEDbleue, LEDallumee );
        else
          digitalWrite( LEDbleue, LEDeteinte );
      }
      break;
    }
}

void setup()
{
  USE_SERIAL.begin( 115200 );
  USE_SERIAL.print( "\n\n\n\n###\nDEMARRAGE DE L'ESP8266\n" );

  pinMode( LEDrouge, OUTPUT );
  pinMode( LEDbleue, OUTPUT );
  onFaitUnePause();

  WiFiMulti.addAP( ssid, password );
  while( WiFiMulti.run() != WL_CONNECTED )
  {
    delay( 100 );
    USE_SERIAL.print( "." );
  }

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent( webSocketEvent );

  if( MDNS.begin( "esp8266" ) )
    { USE_SERIAL.println( "MDNS responder started" ); }

  // index.html
  server.on( "/", []()
    { server.send( 200, "text/html", HTML_DOC );
  });
  server.begin();

  // Add service to MDNS
  MDNS.addService( "http", "tcp", 80 );
  MDNS.addService( "ws",   "tcp", 81 );

  USE_SERIAL.print( "\nConnected! IP address: " );
  USE_SERIAL.println( WiFi.localIP() );
  USE_SERIAL.print( "\nSetup finished\n\n###\n\n" );
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}
