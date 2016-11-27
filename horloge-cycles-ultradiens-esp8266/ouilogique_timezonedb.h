/*

OUILOGIQUE_TIMEZONEDB.H
=======================

Pour ESP8266
Obtient le fuseau horaire d’une zone géographique sur timezonedb.com.

Il faut créer un compte sur le site pour obtenir une clé API.

*/

#ifndef OUILOGIQUE_TIMEZONEDB_H
#define OUILOGIQUE_TIMEZONEDB_H
#define debug false

#include <Arduino.h>
#include <ESP8266WiFi.h>

const char* server = "api.timezonedb.com";
const char* timezonedbGetURLFormat = "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s&fields=gmtOffset";
char getURL[ 115 ];
const unsigned long HTTP_TIMEOUT = 10000;
const size_t MAX_CONTENT_SIZE = 512;



struct UserData
{
  char status   [ 32 ];
  char message  [ 32 ];
  char gmtOffset[ 32 ];
};


// Open connection to the HTTP server
bool httpConnect( const char* hostName )
{
  Serial.print( "CONNEXION AU SERVEUR HTTP : " );
  Serial.print( hostName );

  long T1 = millis();
  bool httpStatus = client.connect( hostName, 80 );
  Serial.print  ( httpStatus ? " / OK" : " / ECHEC" );
  Serial.print  ( " / dT = " );
  Serial.print  ( millis() - T1 );
  Serial.println( " ms " );
  return httpStatus;
}

// Send the HTTP GET request to the server
bool sendRequest( const char* host, const char* getURL )
{
  Serial.print( "GET :                       " );

  client.print( "GET " );
  client.print( getURL );
  client.print( " HTTP/1.1\r\nHost: " );
  client.print( server );
  client.print( "\r\nConnection: close\r\n\r\n" );

  Serial.println( getURL );

  return true;
}

// Close the connection with the HTTP server
void httpDisconnect()
{
  // Serial.println( "Disconnect" );
  client.stop();
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders()
{
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout( HTTP_TIMEOUT );
  bool ok = client.find( endOfHeaders );

  Serial.print( "EN-TETES HTTP :             " );
  if( !ok )
    Serial.println( "PAS DE REPONSE OU REPONSE INVALIDE" );
  else
    Serial.println( "OK" );

  return ok;
}


// Read the body of the response from the HTTP server
void readReponseContent( char* content, size_t maxSize )
{
  long T1 = millis();
  Serial.print( "CLIENT.READ TIME :          " );
  size_t length = 0;
  while( client.available() )
    content[ length++ ] = client.read();
  content[ length ] = 0;
  long dT = millis() - T1;
  Serial.println( dT );
  Serial.print( "NB OCTETS RECUS             " );
  Serial.println( length );

  // Serial.println( "REPONSE DU SERVEUR :        " );
  // Serial.println( content );
  // Serial.println( "FIN REPONSE DU SERVEUR" );

  int contentSize = ( int )strlen( content );

  char * pch;

  int msgStart = 0;
  pch = strchr( content, '{' );
  msgStart = pch-content+1;
  // printf( "msgStart = %d\n", msgStart );

  int msgStop = 0;
  pch = strrchr( content, '}' );
  msgStop = pch-content+1;
  // printf( "msgStop = %d\n", msgStop );

  // // ###########
  // Serial.print( "\n---------------------->" );
  // for( int i=0; i<contentSize; i++ )
  // {
  //   Serial.print( content[ i ] );
  // }
  // Serial.println( "<----------------------" );

  // ###########
  for( int i=0; i<msgStop; i++ )
    { content[ i ] = content[ i + msgStart - 1 ] ; }
  for( int i=msgStop-3; i<=contentSize; i++ )
    { content[ i ] = '\0' ; }

  // // ###########
  // Serial.print( "\n---------------------->" );
  // for( int i=0; i<contentSize; i++ )
  // {
  //   Serial.print( content[ i ] );
  // }
  // Serial.println( "<----------------------" );

  // ###########
  // Serial.print( "\n======================>" );
  // Serial.print( content );
  // Serial.println( "<======================" );

  Serial.print( "REPONSE DU SERVEUR :        " );
  Serial.println( content );

  // Serial.print( "sizeof( content ) = " );
  // Serial.println( sizeof( content ) );
  // Serial.print( "maxSize = " );
  // Serial.println( maxSize );
  // Serial.print( "contentSize = " );
  // Serial.println( contentSize );
  // Serial.print( "( unsigned )strlen( content ) = " );
  // Serial.println( ( unsigned )strlen( content ) );

}


bool parseUserData( char* content, struct UserData* userData )
{
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  const size_t BUFFER_SIZE =
        JSON_OBJECT_SIZE( 3 );

  // Allocate a temporary memory pool on the stack
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  // If the memory pool is too big for the stack, use this instead:
  // DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject( content );

  if (!root.success())
  {
    Serial.println( "JSON parsing failed!" );
    return false;
  }

  strcpy( userData->status,    root[ "status"    ] );
  strcpy( userData->message,   root[ "message"   ] );
  strcpy( userData->gmtOffset, root[ "gmtOffset" ] );
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string

  return true;
}

// Print the data extracted from the JSON
void printUserData( const struct UserData* userData )
{
  Serial.print  ( "status = " );
  Serial.println( userData->status );
  Serial.print  ( "message = " );
  Serial.println( userData->message );
  Serial.print  ( "gmtOffset = " );
  Serial.println( userData->gmtOffset );
  long l_gmtOffset = atoi( userData->gmtOffset );
  Serial.print  ( "l_gmtOffset = " );
  Serial.println( l_gmtOffset );
}



void getTimezone( long *gmtOffset )
{
  if( ! httpConnect( server ) )
    return;

  sprintf( getURL, timezonedbGetURLFormat, timezonedbAPIkey, timezonedbLocation );
  if( sendRequest( server, getURL ) && skipResponseHeaders() )
  {
    char response[ MAX_CONTENT_SIZE ];
    readReponseContent( response, MAX_CONTENT_SIZE );

    UserData userData;
    if( parseUserData( response, &userData ) )
    {
      // printUserData( &userData );
      *gmtOffset = atol( userData.gmtOffset );
    }
  }
  httpDisconnect();
}

#endif //  OUILOGIQUE_NTP_H
