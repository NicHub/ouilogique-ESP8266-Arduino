/*

AFFICHE QUELQUES CARACTÉRISTIQUES DE L’ESP8266 DANS LA CONSOLE SÉRIE
====================================================================

# Sources
https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#user-content-esp-specific-apis
http://links2004.github.io/Arduino/d4/dd2/class_esp_class.html
http://www.esp8266.com/viewtopic.php?f=32&t=4696

juillet 2016, ouilogique.com

*/

struct rst_info
{
  uint32 reason;
  uint32 exccause;
  uint32 epc1;
  uint32 epc2;
  uint32 epc3;
  uint32 excvaddr;
  uint32 depc;
};

void setup()
{
  Serial.begin( 115200 );

  Serial.printf( "\n\n\nESP8266 INFORMATION\n===================\n" );

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

  rst_info *xyz;
  Serial.printf( "ESP.getResetInfoPtr()\n" );
  xyz = ESP.getResetInfoPtr();
  Serial.println( ( *xyz ).reason );
  Serial.println( ( *xyz ).exccause );
  Serial.println( ( *xyz ).epc1 );
  Serial.println( ( *xyz ).epc2 );
  Serial.println( ( *xyz ).epc3 );
  Serial.println( ( *xyz ).excvaddr );
  Serial.println( ( *xyz ).depc );
}

void loop()
{
  delay( 10 );
}


