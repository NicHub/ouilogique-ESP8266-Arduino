/*

OUILOGIQUE_NTP.H
================

Pour ESP8266
Obtient l’heure d’un serveur NTP.

*/

#ifndef OUILOGIQUE_NTP_H
#define OUILOGIQUE_NTP_H

#include <Arduino.h>
// #include <ESP8266WiFi.h>

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

WiFiUDP Udp;
unsigned int localPort = 2390;  // local port to listen for UDP packets

unsigned long secsSince1900Offset;
int timeZone = 0;
int timeZoneGet()
{
  return timeZone;
}


char *getESP8266jsonTime()
{
  static time_t prevTime = 0;
  static char jsonMsg[ 32 ] = "-";

  if( timeStatus() != timeNotSet )
  {
    prevTime = now();
    sprintf( jsonMsg,
      "{\"TIME\":\"%02d:%02d:%02d %02d-%02d-%04d\"}",
      hour( prevTime ), minute( prevTime ), second( prevTime ),
       day( prevTime ),  month( prevTime ),   year( prevTime )
    );
    Serial.printf( "L'HEURE ET LA DATE SONT : %s\n", jsonMsg );
  }
  return jsonMsg;
}

int *getESP8266intarrayTime()
{
  static time_t prevTime = 0;
  static int intarrayTime[ 6 ] = { 0 };

  if( timeStatus() != timeNotSet )
  {
    prevTime = now();
    intarrayTime[ 0 ] =   year( prevTime );
    intarrayTime[ 1 ] =  month( prevTime );
    intarrayTime[ 2 ] =    day( prevTime );
    intarrayTime[ 3 ] =   hour( prevTime );
    intarrayTime[ 4 ] = minute( prevTime );
    intarrayTime[ 5 ] = second( prevTime );
  }
  return intarrayTime;
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[ NTP_PACKET_SIZE ]; //buffer to hold incoming & outgoing packets


// send an NTP request to the time server at the given address
void sendNTPpacket( IPAddress &address )
{
  // set all bytes in the buffer to 0
  memset( packetBuffer, 0, NTP_PACKET_SIZE );
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[  0 ] = 0b11100011;   // LI, Version, Mode
  packetBuffer[  1 ] = 0;            // Stratum, or type of clock
  packetBuffer[  2 ] = 6;            // Polling Interval
  packetBuffer[  3 ] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[ 12 ] = 49;
  packetBuffer[ 13 ] = 0x4E;
  packetBuffer[ 14 ] = 49;
  packetBuffer[ 15 ] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket( address, 123 ); // NTP requests are to port 123
  Udp.write( packetBuffer, NTP_PACKET_SIZE );
  Udp.endPacket();
}


time_t _getNtpTime()
{
  //get a random server from the pool
  WiFi.hostByName( ntpServerName, timeServerIP );
  Serial.print( "ntpServerName = " );
  Serial.println( ntpServerName );
  Serial.print( "timeServerIP = " );
  Serial.println( timeServerIP );

  // discard any previously received packets
  while( Udp.parsePacket() > 0 );

  Serial.println( "Transmit NTP Request" );
  sendNTPpacket( timeServerIP );
  uint32_t beginWait = millis();
  while( millis() - beginWait < 1500 )
  {
    int size = Udp.parsePacket();
    if( size >= NTP_PACKET_SIZE )
    {
      Serial.println( "Receive NTP Response" );
      // read packet into the buffer
      Udp.read( packetBuffer, NTP_PACKET_SIZE );
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900  = ( unsigned long )packetBuffer[ 40 ] << 24;
      secsSince1900 |= ( unsigned long )packetBuffer[ 41 ] << 16;
      secsSince1900 |= ( unsigned long )packetBuffer[ 42 ] << 8;
      secsSince1900 |= ( unsigned long )packetBuffer[ 43 ];
      secsSince1900 += secsSince1900Offset;
      return secsSince1900;
    }
  }
  Serial.println( "No NTP Response :-(" );
  return 0; // return 0 if unable to get the time
}


void udpInit( long gmtOffset )
{
  // https://tools.ietf.org/html/rfc868
  // the time  2,208,988,800 corresponds to 00:00  1 Jan 1970 GMT,
  secsSince1900Offset = gmtOffset - 2208988800UL;
  Udp.begin( localPort );
  setSyncProvider( _getNtpTime );
}

#endif //  OUILOGIQUE_NTP_H
