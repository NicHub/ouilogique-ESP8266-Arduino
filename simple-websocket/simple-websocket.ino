/*

SIMPLE-WEBSOCKET.INO
====================

https://github.com/NicHub/ouilogique-ESP8266-Arduino/tree/master/simple-websocket

# BIBLIOTHÈQUES UTILISÉES
  WebSockets 2.0.4 de Markus Sattler
  https://github.com/Links2004/arduinoWebSockets.git
  Disponible dans le gestionnaire de bibliothèques de l’IDE Arduino
  (rechercher “WebSockets for Arduino (Server+ Client)”)

  Time 1.5.0 de Michael Margolis
  http://playground.arduino.cc/code/time
  Disponible dans le gestionnaire de bibliothèques de l’IDE Arduino
  (rechercher “Timekeeping functionality for Arduino”)

# MODULES TESTÉS
  ESP8266-12E Amica
  ESP8266-01

# NOTES
  Le fichier `WifiSettings.h` doit être créé manuellement à la racine du projet
  et contenir les instructions suivantes :
  const char* ssid     = "***";
  const char* password = "***";

  Pour ne pas mettre à jour le fichier `WifiSettings.h` dans Git, utiliser
  git update-index --skip-worktree WifiSettings.h
  Et pour le mettre à jour (pas recommandé)
  git update-index --no-skip-worktree WifiSettings.h

  Ce croquis est basé sur les exemples suivants :
  https://github.com/Links2004/arduinoWebSockets/blob/master/examples/WebSocketServer_LEDcontrol/WebSocketServer_LEDcontrol.ino
  https://github.com/AdySan/ESPSocket/blob/master/ESPSocket/ESPSocket.ino

# UPLOAD DES FICHIERS DU SERVEUR WEB DE L’ESP8266

## Avec des requêtes POST
  Il faut que l’ESP soit flashé avec `simple-websocket.ino` au préalable pour que ça fonctionne.
  Testé sur Mac OSX et Win 7 avec Cygwin.

    cd data
    ip=$(ping -c 1 esp8266.local | gawk -F'[()]' '/PING/{print $2}')
    echo $ip
    curl                                             \
        -F "file=@img1.jpg"        http://$ip/upload \
        -F "file=@img2.jpg"        http://$ip/upload \
        -F "file=@index.html"      http://$ip/upload \
        -F "file=@logo.png"        http://$ip/upload \
        -F "file=@style.css"       http://$ip/upload \
        -F "file=@websocket.js"    http://$ip/upload

## Avec esp8266fs
  Le fichiers doivent impérativement se trouver dans un répertoire appelé `data`.
  Cette façon de procéder est très lente et pose problème sous Win 7.
  La procédure d’installation et d’utilisation est ici :
  https://github.com/esp8266/arduino-esp8266fs-plugin/

# SUIVI DES MODIFICATIONS
2016-08-11, par NJ, Mise à jour des commentaires suite au test de JMP sur Win7 avec un ESP8266-01

juin 2016, ouilogique.com

*/

#include "WifiSettings.h"
#include "ws_functions.h"

const char* mDNSName = "esp8266";
extern ESP8266WebServer webServer;
extern WebSocketsServer webSocket;
char *jsonMsgTime;

extern void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
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
      webSocket.sendTXT( num, jsonMsgTime );
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

void setup()
{
  // Initialisation du port série
  Serial.begin( 115200 );
  Serial.print( "\n\nDEMO WEBSOCKET\n==============\n\n" );

  // Affichage de quelques caractéristiques de l’ESP8266
  printESPInfo();

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

  // Demande l’heure à un serveur NTP
  udpInit();
  jsonMsgTime = getNTPTime();
  Serial.printf( "HEURE DE DEMARRAGE DE L'ESP8266 : %s\n", jsonMsgTime );

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
