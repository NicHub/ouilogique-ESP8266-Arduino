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

# UPLOAD DES FICHIERS DU SERVEUR WEB DE L’ESP8266

# Avec esp8266fs
Voir https://github.com/esp8266/arduino-esp8266fs-plugin/

# Avec requête POST
Comme esp8266fs est excessivement lent, la librarie ws_functions offre
la possibilité de charger des fichiers sur l’ESP avec des requêtes
POST qui peuvent être exécutées directement depuis le terminal.

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


juin 2016, ouilogique.com

*/


// Le fichier WifiSettings.h doit être présent
// et contenir les instructions suivantes :
// const char* ssid     = "***";
// const char* password = "***";
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

  // Demande l’heure sur un serveur NTP
  udp.begin( localPort );
  jsonMsgTime = getNTPTime();
  Serial.printf( "L'HEURE DE DEMARRAGE EST : %s\n", jsonMsgTime );

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
