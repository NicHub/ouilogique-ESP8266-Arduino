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

  ArduinoJson 5.9.0 de Benoît Blanchon
  https://bblanchon.github.io/ArduinoJson/
  Disponible dans le gestionnaire de bibliothèques de l’IDE Arduino
  (rechercher “ArduinoJson”)

# MODULES TESTÉS
  ESP8266-12E Amica
  ESP8266-01

# NOTES
  Le fichier wifisettings.json doit être édité pour contenir
  les SSID + mots de passes des réseaux auxquels on veut se connecter
  L’indice du réseau actif doit être enregistré dans “wifisettingsactiveindex.ini”.
  Pour ne pas mettre ce fichier dans git :
    git update-index --assume-unchanged wifisettings.json
  Pour le mettre à nouveau :
    git update-index --no-assume-unchanged wifisettings.json

  Dans la version précédente du programme (avant 15 juin 2017),
  il fallait créer manuellement le fichier `WifiSettings.h`
  à la racine du projet et indiquer les informations suivantes :
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

## Avec esp8266fs
  Le premier upload doit être réalisé avec `esp8266fs`,
  mais cette façon de procéder est très lente !
  Pour que les uploads soient plus rapide on utilisera
  pour la suite les requêtes POST avec le programme
  en ligne de commande `curl` (voir ci-dessous).

  La procédure d’installation et d’utilisation d’`esp8266fs` se trouve ici :
  https://github.com/esp8266/arduino-esp8266fs-plugin/

  Le fichiers doivent impérativement se trouver dans un répertoire appelé `data`
  se trouvant à la racine du projet.

  Il faut que le paramètre “Flash Size” dans le menu “Outils” de l’IDE Arduino soit réglé à une valeur
  plus petite ou égale à la quantité de flash disponible. Cette valeur peut être déterminée
  avec le programme get-esp8266-info.ino :
  https://github.com/NicHub/ouilogique-ESP8266-Arduino/blob/master/get-esp8266-info/get-esp8266-info.ino

## Avec des requêtes POST
  Il faut que l’ESP soit flashé avec `simple-websocket.ino` au préalable pour que ça fonctionne.
  Testé sur Mac OSX, Win 7 avec Cygwin et Win10 avec le Bash installé par Git.

    cd data
    ip=$(ping -c 1 esp8266.local | gawk -F'[()]' '/PING/{print $2}'); echo "http://$ip/"
    for FILE in `ls -A1`; do curl -F "file=@$FILE" http://$ip/upload; done
    curl -F "file=@img1.jpg"        http://$ip/upload \
         -F "file=@img2.jpg"        http://$ip/upload \
         -F "file=@index.html"      http://$ip/upload \
         -F "file=@logo.png"        http://$ip/upload \
         -F "file=@style.css"       http://$ip/upload \
         -F "file=@websocket.js"    http://$ip/upload


juin 2016-2018, ouilogique.com

*/

#include <ArduinoJson.h>
#include "ws_functions.h"

char mDNSName[ 26 ];
char macAddress[ 18 ];
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
    if( payload[0] == '[' )
    {
      wifiSettingsFileWrite( num, type, payload, length );
    }
    if( payload[0] == '-' )
    {
      payload[0] = ' ';
      wifiSettingsActiveIndexFileWrite( num, type, payload, length );
    }
    if( payload[0] == '*' )
    {
      wifiSettingsFileRead( num );
    }
    if( payload[0] == '%' )
    {
      Serial.println( "RESTART !!!\n\n\n" );
      delay( 10 );
      digitalWrite( RESET_PIN, LOW );
    }
    if( payload[0] == 'W' )
    {
      IPAddress espIP = WiFi.localIP();
      static char jsonMsg[ 100 ] = "-";
      sprintf( jsonMsg, "{\"IP\":\"%d.%d.%d.%d\",\"mDNSName\":\"%s\"}", espIP[0], espIP[1], espIP[2], espIP[3], mDNSName );
      Serial.println( jsonMsg );
      webSocket.sendTXT( num, jsonMsg );
    }
    if( payload[0] == '{' )
    {
      // String JSON pour tests
      // {"GPIO":{"GPIO2":1,"GPIO16":0}, "SERIAL":"COUCOU c'est moi"}
      DynamicJsonBuffer jsonBuffer( length );
      JsonObject& root = jsonBuffer.parseObject( payload );
      if( ! root.success() )
      {
        Serial.println( "JSON parsing failed!" );
        return;
      }
      Serial.println( "JSON parsing OK!" );
      root.printTo( Serial );

      // À simplifier : lire https://bblanchon.github.io/ArduinoJson/api/jsonobject/containskey/
      for( JsonObject::iterator it=root.begin(); it!=root.end(); ++it )
      {
          const char* key   = it->key;
          JsonVariant value = it->value;
          Serial.print( "key = " );
          Serial.println( key );

          if( value.is<JsonObject>() ) {
            if( strcmp( key, "GPIO"   ) == 0 ) receivedGPIO( value );
          }
          if( strcmp( key, "SERIAL" ) == 0 ) receivedSERIAL( value );
      }
    }
    break;
  }
}

void receivedGPIO( JsonObject& GPIOobject ) {
  for( JsonObject::iterator it=GPIOobject.begin(); it!=GPIOobject.end(); ++it )
  {
    const char* key = it->key;
    JsonVariant value = it->value;
    Serial.print( "GPIOobject key = " );
    int NB = String( key ).substring( 4 ).toInt();
    Serial.print( key );
    Serial.print( " " );
    Serial.print( NB );
    Serial.print( " " );
    int val = value.as<int>();
    Serial.println( val );
    digitalWrite( NB, val );
  }
}

void receivedSERIAL( JsonVariant& SERIALobject ) {
  Serial.print( "cmd SERIAL recu" );
  Serial.println( "\n############# receivedSERIAL" );
  // SERIALobject.printTo( Serial );
  const char* SerialChar = SERIALobject;
  Serial.println( SerialChar );
  Serial.println( "\n############# \n" );
}

void wifiSettingsFileWrite( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
{
  String path = String( "/wifisettings.json" );
  Serial.println( "wifiSettingsFileWrite: " + path );
  Serial.printf( "PAYLOAD %s\n###\n", payload );

  // if( SPIFFS.exists( path ) )
  //   return webServer.send(500, "text/plain", "FILE EXISTS");
  File cFile;
  cFile = SPIFFS.open( path, "w" );
  if( cFile )
  {
    Serial.println( "Writing " + path );
    char payloadChar[ length+1 ];
    sprintf( payloadChar, "%s", payload );
    cFile.print( payloadChar );
    Serial.println( "Closing " + path );
    cFile.close();
  }
  else
  {
    Serial.println( "Error writing " + path );
    return webServer.send( 500, "text/plain", "CREATE FAILED" );
  }

  webServer.send( 200, "text/plain", "" );
  path = String();
}

void wifiSettingsActiveIndexFileWrite( uint8_t num, WStype_t type, uint8_t * payload, size_t length )
{
  String path = String( "/wifisettingsactiveindex.ini" );
  Serial.println( "wifiSettingsActiveIndexFileWrite: " + path );
  Serial.printf( "PAYLOAD %s\n###\n", payload );

  // if( SPIFFS.exists( path ) )
  //   return webServer.send(500, "text/plain", "FILE EXISTS");
  File cFile;
  cFile = SPIFFS.open( path, "w" );
  if( cFile )
  {
    Serial.println( "Writing " + path );
    char payloadChar[ length+1 ];
    sprintf( payloadChar, "%s", payload );
    cFile.print( payloadChar );
    Serial.println( "Closing " + path );
    cFile.close();
  }
  else
  {
    Serial.println( "Error writing " + path );
    return webServer.send( 500, "text/plain", "CREATE FAILED" );
  }

  webServer.send( 200, "text/plain", "" );
  path = String();
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

void initGPIO()
{
  // Initialisation des LED
  pinMode( LED_RED,  OUTPUT );
  pinMode( LED_BLUE, OUTPUT );
  pinMode( RESET_PIN,  OUTPUT );
  digitalWrite( RESET_PIN, HIGH );
  digitalWrite( LED_BLUE, LED_CLEAR );
  digitalWrite( LED_RED,  LED_CLEAR );
  onFaitUnePause( 4000 );
}

void initSerial()
{
  Serial.begin( 115200 );
  Serial.print( "\n\n\n\n\n\n\n\nDEMO WEBSOCKET\n==============\n\n" );
}

void printCurrentTime()
{
  jsonMsgTime = getNTPTime();
  Serial.printf( "\tHeure et date (main) : %s\n\n", jsonMsgTime );
}

void finSetup()
{
  Serial.print( "# FIN DE L'INITIALISATION\n" );
  Serial.printf( "\tNom         : %s.local\n", mDNSName );
  Serial.print( "\tAdresse Mac : " );
  Serial.println( macAddress );
  Serial.print( "\tAdresse IP  : " );
  Serial.println( WiFi.localIP() );
  Serial.print( "\tURL         : http://" );
  Serial.print( WiFi.localIP() );
  Serial.print( "/\n\n##############\n\n" );
}

void setup()
{
  // Initialisation du port série
  initSerial();

  // Affichage de quelques caractéristiques de l’ESP8266
  printESPInfo();

  // Initialisation des GPIO
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
}
