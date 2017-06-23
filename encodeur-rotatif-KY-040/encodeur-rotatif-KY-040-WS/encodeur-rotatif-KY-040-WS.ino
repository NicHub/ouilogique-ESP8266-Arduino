/*

LECTURE D’UN ENCODEUR ROTATIF KY-040
====================================

# DESCRIPTION DU PROGRAMME
Lecture d’un encodeur rotatif KY-040 et transmission des valeurs via WebSocket
https://www.youtube.com/watch?v=pPmwxcTyQ3I
http://ouilogique.com/files/esp8266-encodeur-rotatif-KY-040-websocket/esp8266-encodeur-rotatif-KY-040-websocket.jpg

Ce programme est basé sur celui d’Oleg Mazurov
https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino

# RÉFÉRENCE DE L’ENCODEUR
http://www.banggood.com/5Pcs-5V-KY-040-Rotary-Encoder-Module-For-Arduino-AVR-PIC-p-951151.html

# CONNEXIONS DE L’ENCODEUR KY-040 SUR ESP8266
    GND              ⇒   GND
    +                ⇒   +3.3V
    SW  (bouton)     ⇒   D3 (GPIO 0)
    DT  (encodeur)   ⇒   D2 (GPIO 4)
    CLK (encodeur)   ⇒   D1 (GPIO 5)

# MISE EN ROUTE
  - Éditer le fichier “wifisettings.json” en indiquant un SSID + PASSWORD valides en première position.
  - Flasher le répertoire “data” sur l’ESP avec la commande de l’IDE Arduino : Tools/ESP8266 Sketch Data Upload
  - Uploader ce programme
  - La mise à jour des fichiers peut se faire avec `curl` (voir ci-dessous)

# MISE À JOUR DES FICHIERS
  ip=$(ping -c 1 esp8266.local | gawk -F'[()]' '/PING/{print $2}'); echo "http://$ip/"
  curl -F "file=@websocket.js"        http://$ip/upload

# POUR NE PAS METTRE DE MOTS DE PASSE EN LIGNE
git update-index --assume-unchanged "wifisettings.json"

juin 2017, ouilogique.com

*/

#include "encoder.h"
#include "ws_functions.h"

const char* mDNSName = "esp8266";
extern ESP8266WebServer webServer;
extern WebSocketsServer webSocket;
char *jsonMsgTime;

extern void webSocketEvent( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
{
  Serial.print( "\nnum = " );
  Serial.print( num );

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
      // WSsendGPIOStates( num );
      webSocket.sendTXT( num, jsonMsgTime );
    }
    break;

  // Commande reçue
  case WStype_TEXT:
    break;
  }
}


void printCurrentTime()
{
  jsonMsgTime = getNTPTime();
  Serial.printf( "\tHeure et date (main) : %s\n\n", jsonMsgTime );
}


void initSerial()
{
  Serial.begin( 115200 );
  Serial.print( "\n\n\n\n\n\n\n\nDEMO ENCODEUR\n==============\n\n" );
}


void initGPIO()
{
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
}


void finSetup()
{
  Serial.print( "# FIN DE L'INITIALISATION\n" );
  Serial.printf( "\tNom        : %s.local\n", mDNSName );
  Serial.print( "\tAdresse IP : " );
  Serial.println( WiFi.localIP() );
  Serial.print( "\tURL        : http://" );
  Serial.print( WiFi.localIP() );
  Serial.print( "/\n\n##############\n\n" );
}


void setup()
{
  initSerial();
  initEncodeur();
  initGPIO();

 // Initialisation du système de fichiers
  initSystemeFichiers();

  // Test JSON
  parseJSONwifiSettingsFromFile();

  // Démarrage des services web
  scanNetwork();
  initServicesWeb();

  // Initialisation du service UDP
  udpInit();
  printCurrentTime();

  // Fin de l’initialisation
  finSetup();
}


void loop()
{
  webSocket.loop();
  webServer.handleClient();

  // Lecture et affichage des valeurs de l’encodeur.
  if( encodeurTourne )
  {
    encType val = lectureEncodeur( false );
    if( val.valid )
    {
      serialSendEncoder( val );
      wsSendEncoder( val );
    }
    encodeurTourne = false;
  }

  // Si le bouton est pressé, on remet le compteur à 0.
  if( boutonPresse )
  {
    encType val = lectureEncodeur( true );
    if( val.valid )
    {
      serialSendEncoder( val );
      wsSendEncoder( val );
    }
    digitalWrite( LED_BUILTIN, LOW );
    delay( 20 );
    digitalWrite( LED_BUILTIN, HIGH );
    boutonPresse = false;
  }

  yield();
}


void serialSendEncoder( encType encodeurVal )
{
  // Pour visionnement dans le traceur série de l’IDE Arduino (CMD-SHIFT-L)
  Serial.println( encodeurVal.encVal );
}


void wsSendEncoder( encType encodeurVal )
{
  static char jsonMsg[ 100 ];
  sprintf( jsonMsg, "{\"cpt\":{\"cpt1\":\"%d\",\"cpt2\":\"%d\"}}", encodeurVal.encVal, 0 );
  webSocket.broadcastTXT( jsonMsg );
}
