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

# NOTE
  Pour ne pas mettre à jour le fichier “WifiSettings.h” dans Git utiliser
  git update-index --skip-worktree WifiSettings.h
  Et pour le mettre à jour (pas recommandé)
  git update-index --no-skip-worktree WifiSettings.h

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
static const char* mDNSName = "esp8266";

#define LEDrouge   D0 // GPIO 16;
#define LEDbleue   D4 // GPIO  2;
#define Btn1       D5 // GPIO 14;
#define Btn2       D3 // GPIO  0;

#define LEDallumee 0
#define LEDeteinte 1
#define btn1Get ! digitalRead( Btn1 )
#define btn2Get ! digitalRead( Btn2 )
bool btn1LastState = false;
bool btn2LastState = false;

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server    = ESP8266WebServer( 80 );
WebSocketsServer webSocket = WebSocketsServer( 81 );

void onFaitUnePause( unsigned long attente )
{
  unsigned long Tf = millis() + attente;
  USE_SERIAL.flush();
  bool status = false;
  USE_SERIAL.printf( "On fait une pause de %d ms ", attente );
  while( millis() < Tf )
  {
    digitalWrite( LEDbleue,   status );
    digitalWrite( LEDrouge, ! status );
    status = ! status;
    if( status ) { USE_SERIAL.print( "." ); }
    delay( 60 );
  }
  USE_SERIAL.print( " OK\n" );
  USE_SERIAL.flush();
}

// void sendJsonStatus()
// {
//   IPAddress ip = webSocket.remoteIP( num );
//   char jsonMsg[ 50 ];
//   sprintf( jsonMsg,
//     "{\"GPIO16\":%d,"
//      "\"GPIO02\":%d,"
//      "\"GPIO14\":%d,"
//      "\"GPIO00\":%d}"
//      ,
//      !digitalRead( LEDrouge ),
//      !digitalRead( LEDbleue ),
//       btn1Get,
//       btn2Get
//   );

//   USE_SERIAL.printf( "%s\n", jsonMsg );
//   webSocket.sendTXT( num, jsonMsg );
// }

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
      // webSocket.sendTXT( num, "{\"Connexion\":\"OK\"}" );
      char jsonMsg[ 50 ];
      sprintf( jsonMsg,
        "{\"GPIO16\":%d,"
         "\"GPIO02\":%d}",
         !digitalRead( LEDrouge ),
         !digitalRead( LEDbleue ) );

      USE_SERIAL.printf( "%s\n", jsonMsg );
      webSocket.sendTXT( num, jsonMsg );


      break;

    case WStype_TEXT:
      USE_SERIAL.printf( "length : %d\n", length );
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

        USE_SERIAL.print( "RR = " );
        USE_SERIAL.println( RR );
        USE_SERIAL.print( "BB = " );
        USE_SERIAL.println( BB );
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

void printESPInfo()
{
  USE_SERIAL.printf( "ESP CHIP ID          : %d\n", ESP.getChipId()         );
  USE_SERIAL.printf( "ESP FREE HEAP        : %d\n", ESP.getFreeHeap()       );
  USE_SERIAL.printf( "ESP FLASH CHIP ID    : %d\n", ESP.getFlashChipId()    );
  USE_SERIAL.printf( "ESP FLASH CHIP SIZE  : %d\n", ESP.getFlashChipSize()  );
  USE_SERIAL.printf( "ESP FLASH CHIP SPEED : %d\n", ESP.getFlashChipSpeed() );
  USE_SERIAL.flush(); // Pour attendre que tous les caractères soient envoyés
}

void demarrageServicesWeb()
{
  // Démarrage de la connexion WiFi
  USE_SERIAL.print( "Demarrage de la connexion WiFi .." );
  WiFiMulti.addAP( ssid, password );
  while( WiFiMulti.run() != WL_CONNECTED )
  {
    USE_SERIAL.print( "." );
    USE_SERIAL.flush();
    delay( 100 );
  }
  USE_SERIAL.print( " OK\n" );

  // Démarrage du serveur webSocket
  USE_SERIAL.print( "Demarrage du serveur WebSocket\n" );
  webSocket.begin();
  webSocket.onEvent( webSocketEvent );

  // Démarrage du mDNS (multicast Domain Name System)
  USE_SERIAL.printf( "Demarrage du mDNS avec le nom '%s.local' .. ", mDNSName );
  if( MDNS.begin( mDNSName ) )
    { USE_SERIAL.print( "OK\n" ); }
  else
    { USE_SERIAL.print( "PAS OK\n" ); }

  // Chargement du fichier “index.html”
  USE_SERIAL.print( "Chargement du fichier 'index.html'\n" );
  server.on( "/", []()
    { server.send( 200, "text/html", HTML_DOC );
  });

  // Démarrage du serveur
  USE_SERIAL.print( "Demarrage du serveur\n" );
  server.begin();

  // Ajout des services HTTP et WebSocket au mDNS
  USE_SERIAL.print( "Ajout des services HTTP et WebSocket au mDNS\n" );
  MDNS.addService( "http", "tcp", 80 );
  MDNS.addService( "ws",   "tcp", 81 );
}

void setup()
{
  // Initialisation de la liaison série
  USE_SERIAL.begin( 115200 );
  USE_SERIAL.print( "\n\n\n\n###\n\nDEMARRAGE DE L'ESP8266\n\n" );

  // Initialisation des boutons
  pinMode( Btn1, INPUT_PULLUP );
  pinMode( Btn2, INPUT_PULLUP );

  // Initialisation des LED et attente
  pinMode( LEDrouge, OUTPUT );
  pinMode( LEDbleue, OUTPUT );
  onFaitUnePause( 4000 );

  // Démarrage des services web
  demarrageServicesWeb();

  // Affichage de quelques caractéristiques de l’ESP8266
  printESPInfo();

  // Fin de l’initialisation
  USE_SERIAL.print( "Fin de l'initialisation\n" );
  USE_SERIAL.print( "Adresse IP : " );
  USE_SERIAL.println( WiFi.localIP() );
  USE_SERIAL.printf( "Nom        : %s.local \n\n###\n\n", mDNSName );
}

void loop()
{
  webSocket.loop();
  server.handleClient();

  bool btn1CurrentState = btn1Get;
  bool btn2CurrentState = btn2Get;
  if( btn1CurrentState != btn1LastState )
  {
    USE_SERIAL.printf( "Bouton 1 = %d\n", btn1CurrentState );
    btn1LastState = btn1CurrentState;
  }
  if( btn2CurrentState != btn2LastState )
  {
    USE_SERIAL.printf( "Bouton 2 = %d\n", btn2CurrentState );
    btn2LastState = btn2CurrentState;
  }

  // if( btn1CurrentState != btn1LastState || btn2CurrentState != btn2LastState )
  // {
  //   sendJsonStatus();
  // }

}
