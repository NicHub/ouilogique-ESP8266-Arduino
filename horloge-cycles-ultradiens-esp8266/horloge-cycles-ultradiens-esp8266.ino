/*

HORLOGE À CYCLES ULTRADIENS
===========================

http://ouilogique.com/horloge_cycles_ultradiens/

# DESCRIPTION DU PROGRAMME
Voir
https://github.com/NicHub/ouilogique-Arduino/blob/master/horloge-cycles-ultradiens-arduino/horloge-cycles-ultradiens-arduino.ino


# CONNEXIONS ESP8266 12E (Amica ou Lolin)
  GND          ⇒   GND
  VCC          ⇒   +3.3V
  I²C SDA      ⇒   pin D2  (GPIO  4) + pullup 4.7 kΩ
  I²C SCL      ⇒   pin D1  (GPIO  5) + pullup 4.7 kΩ
  Buzzer +     ⇒   pin D6  (GPIO 12)
  Buzzer -     ⇒   GND
  Bouton 1 +   ⇒   pin D5  (GPIO 14)
  Bouton 1 -   ⇒   GND
  Bouton 2 +   ⇒   pin D3  (GPIO  0)
  Bouton 2 -   ⇒   GND

# HORLOGE DS1307 I²C (RTC = Real Time Clock)
  ## RÉFÉRENCE AliExpress
  http://www.aliexpress.com/item/5pcs-lot-Tiny-RTC-I2C-AT24C32-DS1307-Real-Time-Clock-Module-Board-For-Arduino-With-A/32327865928.html

  ## ADRESSES I²C
  0x50 (EEPROM AT24C32)
  0x68 (DS1307)

  ## LIBRAIRIE Adafruit
  https://github.com/adafruit/RTClib.git

  ## CONNEXIONS
  GND   ⇒   GND
  VCC   ⇒   +5V
  SDA   ⇒   pin D2  (GPIO  4) + pullup 4.7 kΩ
  SCL   ⇒   pin D1  (GPIO  5) + pullup 4.7 kΩ

# ÉCRAN OLED 128×64 I²C (compatible SSD1306)
  ## RÉFÉRENCE AliExpress
  http://www.aliexpress.com/item/1Pcs-Yellow-blue-double-color-128X64-OLED-LCD-LED-Display-Module-For-Arduino-0-96/32305641669.html

  ## ADRESSE I²C
  0x3C

  ## LIBRAIRIE Adafruit
  https://github.com/adafruit/Adafruit_SSD1306.git

  ## CONNEXIONS
  GND   ⇒   GND
  VDD   ⇒   +5V
  SDA   ⇒   pin D2  (GPIO  4) + pullup 4.7 kΩ
  SCK   ⇒   pin D1  (GPIO  5) + pullup 4.7 kΩ

# MICROCONTRÔLEUR
  ESP8266 12E (Amica ou LoLin)

# FUSEAU HORAIRE
  Le fuseau horaire est déterminé en fonction de la position géographique sur timezonedb.com
  Voir ouilogique_timezonedb.h

# NOTES
  La fonction Serial.print perturbe la RTC. Il faut mettre les Serial.print à la
  fin des procédures.

  Le fichier `WifiSettings.h` doit être créé manuellement à la racine du projet
  et contenir les instructions suivantes :
  const char* ssid               = "***";
  const char* password           = "***";
  const char* timezonedbAPIkey   = "***";
  const char* timezonedbLocation = "Europe/Zurich";


juin 2016, ouilogique.com

*/
#include "WifiSettings.h"

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>


WiFiClient client;
#include "ouilogique_timezonedb.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 16 // OLED_RESET=16 pour ESP8266 // OLED_RESET=4 pour Arduino Nano
Adafruit_SSD1306 display( OLED_RESET );
#if( SSD1306_LCDHEIGHT != 64 )
#error( "Height incorrect, please fix Adafruit_SSD1306.h!" );
#endif

#include "aTunes.h"
#define carillonPin 12 // pin D6
#define dXCarillon 5
#define dYCarillon 25
bool carillonGet = false;

#include "ouilogique_ntp.h"

const int bBtn1  = 14;
const int bBtn2  =  0;
bool btn1Get = false;
bool btn2Get = false;

#include <RTClib.h>
RTC_DS1307 RTC = RTC_DS1307();

const int SSD1306_i2c = 0x3C;

#define LEDrouge  16 // D0 ⇒ GPIO 16
#define LEDbleue   2 // D4 ⇒ GPIO 2
#define LEDallumee 0
#define LEDeteinte 1

const unsigned long periodeRafraichissement = F_CPU * 1; // Un rafraîchissement par seconde
bool cestlheure = false;

// Modifier ici l’heure d’attention maximum.
// Par exemple, si 7 h 15 est une heure d’attention maximum :
// heureAttentionMax = 7 h 15
// heureAttentionMax = MOD( 7*3600 + 15*60, 5400 )
// heureAttentionMax = 4500 s
// (5400 est le nombre de secondes dans 1 h 30)
const long heureAttentionMax = 4500;
const byte displayWidth = 128;
static const unsigned char cosinus_cmap[ displayWidth ] PROGMEM =
{
  63, 63, 63, 63, 63, 62, 62, 62,
  62, 62, 62, 61, 61, 61, 61, 60,
  60, 60, 59, 59, 58, 58, 57, 57,
  56, 56, 55, 55, 54, 54, 53, 53,
  52, 52, 51, 51, 50, 50, 49, 49,
  48, 48, 47, 47, 46, 46, 45, 45,
  45, 44, 44, 44, 44, 43, 43, 43,
  43, 43, 43, 42, 42, 42, 42, 42,
  42, 42, 42, 42, 42, 43, 43, 43,
  43, 43, 43, 44, 44, 44, 44, 45,
  45, 45, 46, 46, 47, 47, 48, 48,
  49, 49, 50, 50, 51, 51, 52, 52,
  53, 53, 54, 54, 55, 55, 56, 56,
  57, 57, 58, 58, 59, 59, 60, 60,
  60, 61, 61, 61, 61, 62, 62, 62,
  62, 62, 62, 63, 63, 63, 63, 63
};

void prepareCourbeCycle( int16_t frac16eJourPx )
{
  unsigned char py;
  // Partie de la courbe avec remplissage
  for( int16_t px=0; px<=frac16eJourPx; px++ )
  {
    py = pgm_read_byte( &cosinus_cmap[ px ] );
    display.drawLine( px, display.height()-1, px, py, WHITE );
  }
  // Partie de la courbe sans remplissage
  for( int16_t px=frac16eJourPx+1; px<displayWidth; px++ )
  {
    py = pgm_read_byte( &cosinus_cmap[ px ] );
    display.drawPixel( px, py, WHITE );
  }
}

void clignote()
{
  for( int i=1; i<10; i++ )
  {
    digitalWrite( LEDbleue, LEDallumee );
    digitalWrite( LEDrouge, LEDeteinte );
    delay( 60 );
    digitalWrite( LEDbleue, LEDeteinte );
    digitalWrite( LEDrouge, LEDallumee );
    delay( 60 );
  }
  digitalWrite( LEDbleue, LEDeteinte );
  digitalWrite( LEDrouge, LEDeteinte );
}

void initGPIO()
{
  pinMode( bBtn1, INPUT_PULLUP );
  pinMode( bBtn2, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( bBtn1 ), btn1Press, FALLING );
  attachInterrupt( digitalPinToInterrupt( bBtn2 ), btn2Press, FALLING );
  pinMode( LEDbleue, OUTPUT );
  pinMode( LEDrouge, OUTPUT );
  clignote();
}

void initEcran()
{
  display.begin( SSD1306_SWITCHCAPVCC, 0x3C );
  display.clearDisplay();
  display.setTextColor( INVERSE );
  display.setTextSize( 2 );
  display.setCursor( 25, 0 );
  display.print( F( "HORLOGE" ) );
  display.setCursor( 20, 19 );
  display.print( F( "A CYCLES" ) );
  display.setCursor( 5, 40 );
  display.print( F( "ULTRADIENS" ) );
  display.display();
}

void initWifi()
{
  display.clearDisplay();
  display.setTextSize( 2 );
  display.setCursor( 5, 0 );
  display.print( "CONNEXION" );
  display.setCursor( 5, 19 );
  display.print( "AU RESEAU" );
  display.setCursor( 5, 40 );
  display.print( ssid );
  display.display();
  display.setCursor( 0, 50 );
  WiFi.begin( ssid, password );
  while( WiFi.status() != WL_CONNECTED )
  {
    delay( 500 );
    display.print( "." );
    display.display();
  }
  display.clearDisplay();
  display.setCursor( 5, 0 );
  display.print( "ADRESSE IP" );
  display.setCursor( 0, 19 );
  display.print( WiFi.localIP() );
  display.display();
  delay( 2000 );
}

void initHorloge( long gmtOffset )
{
  Serial.print( "gmtOffset :                 " );
  Serial.println( gmtOffset );

  // Initialisation de l’horloge
  RTC.begin();

  // Demande l’heure à un serveur NTP et règle l’heure interne de l’ESP
  // (pas celle du RTC) en conséquence.
  udpInit( gmtOffset );
  int *dateHeureInt;
  dateHeureInt = getESP8266intarrayTime();

  // Prépare l’affichage du statut NTP
  display.clearDisplay();
  display.setTextColor( INVERSE );
  display.setTextSize( 2 );

  // Réglage du RTC
  if( dateHeureInt[ 0 ] > 2015 )
  {
    RTC.adjust( DateTime(
      dateHeureInt[ 0 ],
      dateHeureInt[ 1 ],
      dateHeureInt[ 2 ],
      dateHeureInt[ 3 ],
      dateHeureInt[ 4 ],
      dateHeureInt[ 5 ] ) );
    display.setCursor( 18, 0 );
    display.print( F( "NTP  OK" ) );
  }
  else
  {
    display.setCursor( 5, 0 );
    display.print( F( "ECHEC  NTP" ) );
  }
  display.setCursor( 6, 19 );
  display.print( F( "GMT OFFSET" ) );
  display.setCursor( 42, 40 );
  display.print( gmtOffset );
  display.display();
  delay( 2000 );

  // Préparation de l’affichage de l’heure actuelle
  DateTime now = RTC.now();
  char nowChar[ 37 ];

  // Affiche l’heure à l’écran
  sprintf(
    nowChar,
    "%1d-%02d-%02d",
    now.year(), now.month(),  now.day() );
  display.clearDisplay();
  display.setCursor( 5, 19 );
  display.print( nowChar );
  display.display();
  delay( 1000 );

  sprintf(
    nowChar,
    "%02d:%02d:%02d",
    now.hour(), now.minute(), now.second() );
  display.setCursor( 20, 40 );
  display.print( nowChar );
  display.display();

  // Affiche l’heure sur le port série
  sprintf(
    nowChar,
    "Heure actuelle : %1d-%02d-%02d %02d:%02d:%02d",
    now.year(), now.month(),  now.day(),
    now.hour(), now.minute(), now.second() );
  Serial.println( nowChar );

  delay( 2000 );
}

void horloge()
{
  // ****
  // Calculs du pourcentage du cycle d’attention (cycleAtt)
  // et du temps équivalent en 1/16e de jour exprimé en pixels (frac16eJourPx)
  // Temps d’exécution sur l’ATmega328P ≅ 1.3 ms
  // Temps d’exécution sur l’ESP8266 ( 80 MHz) ≅ 1.1 ms
  // Temps d’exécution sur l’ESP8266 (160 MHz) ≅ 1.1 ms
  // **

  // lecture de l’heure actuelle
  DateTime now = RTC.now();

  // Calcul du temps équivalent en 1/16e de jour
  // NB : - Il y a 16 cycles d’1 h 30 dans 24 h
  //      - 1 h 30 = 5400 s
  long frac16eJour = ( now.secondstime() - heureAttentionMax ) % 5400;

  // Calcul du pourcentage du cycle d’attention
  // 2 * π * 16 / 86400 = 0.0011635528
  double cycleAtt = 100.0 * 0.5 * ( 1.0 + cos( ( double )( frac16eJour ) * 0.0011635528 ) );

  // Conversion de “frac16eJour” en pixels.
  // Le cosinus est affiché avec un déphasage d’une demi-période, donc
  // les valeurs de la 1ère moitié du cycle correspondent à la partie droite du cosinus et
  // les valeurs de la 2e moitié du cycle correspondent à la partie gauche du cosinus.
  // Le code ci-dessous permute les deux moitiés pour qu’elles s’affichent du bon côté.
  int16_t frac16eJourPx;
  if( frac16eJour < 5400/2 )
    { frac16eJourPx = map( frac16eJour,
                              0,              5400/2-1,
                              displayWidth/2, displayWidth-1 ); }
  else
    { frac16eJourPx = map( frac16eJour,
                              5400/2,         5400-1,
                              0,              displayWidth/2-1 ); }


  // ****
  //  Affichage des résultats
  // Temps d’exécution sur l’ATmega328P ≅ 69 ms
  // Temps d’exécution sur l’ESP8266 ( 80 MHz) ≅ 115 ms
  // Temps d’exécution sur l’ESP8266 (160 MHz) ≅ 111 ms
  // **

  // Effacement de l’écran
  display.clearDisplay();

  // Préparation de l’affichage de la date
  display.setTextSize( 1 );
  if( now.day() < 10 )
    { display.setCursor( 6, 0 ); }
  else
    { display.setCursor( 0, 0 ); }
  display.print( now.day() );
  display.setCursor( 15, 0 );
  switch( now.month() )
  {
    case  1: display.print( F( "JAN"  ) ); break;
    case  2: display.print( F( "FEV"  ) ); break;
    case  3: display.print( F( "MARS" ) ); break;
    case  4: display.print( F( "AVR"  ) ); break;
    case  5: display.print( F( "MAI"  ) ); break;
    case  6: display.print( F( "JUIN" ) ); break;
    case  7: display.print( F( "JUIL" ) ); break;
    case  8: display.print( F( "AOUT" ) ); break;
    case  9: display.print( F( "SEPT" ) ); break;
    case 10: display.print( F( "OCT"  ) ); break;
    case 11: display.print( F( "NOV"  ) ); break;
    case 12: display.print( F( "DEC"  ) );
  }
  display.setCursor( 15, 9 );
  display.print( now.year() );

  // Préparation de l’affichage de l’heure
  char texteAffichage[ 5 ];
  display.setTextSize( 2 );
  display.setCursor( 54, 0 );
  sprintf( texteAffichage, "%2d", now.hour() );
  display.print( texteAffichage );

  // Préparation de l’affichage du séparateur
  display.setCursor( 76, 0 );
  display.print( F( ":" ) );

  // Préparation de l’affichage des minutes
  display.setCursor( 86, 0 );
  sprintf( texteAffichage, "%02d", now.minute() );
  display.print( texteAffichage );

  // Préparation de l’affichage des secondes
  display.setTextSize( 1 );
  display.setCursor( 112, 0 );
  sprintf( texteAffichage, "%02d", now.second() );
  display.print( texteAffichage );

  // Préparation de l’affichage de la courbe du cycle
  prepareCourbeCycle( frac16eJourPx );

  // Préparation de l’affichage du pourcentage du cycle
  display.setTextSize( 2 );
  #define hY 48
  #if false // Affichage avec 3 chiffres significatifs pour le déverminage
    if( cycleAtt < 0.005 )
      { display.setCursor( 53, hY ); display.print( cycleAtt, 0 ); }
    else if( cycleAtt < 9.995 )
      { display.setCursor( 34, hY ); display.print( cycleAtt, 2 ); }
    else if( cycleAtt < 99.95 )
      { display.setCursor( 34, hY ); display.print( cycleAtt, 1 ); }
    else
      { display.setCursor( 42, hY ); display.print( cycleAtt, 0 ); }
  #else // Affichage sans décimales pour l’utilisation normale
    int16_t tx;
    if( cycleAtt < 9.5 )       { tx = 52; }
    else if( cycleAtt < 99.5 ) { tx = 47; }
    else                       { tx = 41; }
    display.setCursor( tx, hY );
    display.print( cycleAtt, 0 );
    display.setCursor( display.getCursorX() + 3, hY );
  #endif
  display.print( F( "\x25" ) ); // signe %

  // Préparation de l’affichage de l’icône du carillon
  prepareIconeCarillon();

  // Met à jour l’affichage
  display.display();

  // Sonne lorsque le cycle est au maximum
  // Cette procédure est bloquante, donc l’horloge ne sera
  // pas mise à jour pendant la sonnerie !
  if( frac16eJour == 0  )
    { carillon(); }
}

void prepareIconeCarillonBase()
{
  display.drawLine(  0+dXCarillon,  5+dYCarillon,  0+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  1+dXCarillon,  5+dYCarillon,  1+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  2+dXCarillon,  5+dYCarillon,  2+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  3+dXCarillon,  5+dYCarillon,  3+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  4+dXCarillon,  5+dYCarillon,  4+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  5+dXCarillon,  5+dYCarillon,  5+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine(  6+dXCarillon,  4+dYCarillon,  6+dXCarillon, 12+dYCarillon, WHITE );
  display.drawLine(  7+dXCarillon,  3+dYCarillon,  7+dXCarillon, 13+dYCarillon, WHITE );
  display.drawLine(  8+dXCarillon,  2+dYCarillon,  8+dXCarillon, 14+dYCarillon, WHITE );
  display.drawLine(  9+dXCarillon,  1+dYCarillon,  9+dXCarillon, 15+dYCarillon, WHITE );
  display.drawLine( 10+dXCarillon,  0+dYCarillon, 10+dXCarillon, 16+dYCarillon, WHITE );
}

void prepareIconeCarillonON()
{
  display.drawLine( 13+dXCarillon,  5+dYCarillon, 13+dXCarillon,  6+dYCarillon, WHITE );
  display.drawLine( 13+dXCarillon, 10+dYCarillon, 13+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine( 14+dXCarillon,  6+dYCarillon, 14+dXCarillon, 10+dYCarillon, WHITE );

  display.drawLine( 15+dXCarillon,  3+dYCarillon, 15+dXCarillon,  4+dYCarillon, WHITE );
  display.drawLine( 15+dXCarillon, 12+dYCarillon, 15+dXCarillon, 13+dYCarillon, WHITE );
  display.drawLine( 16+dXCarillon,  4+dYCarillon, 16+dXCarillon,  5+dYCarillon, WHITE );
  display.drawLine( 16+dXCarillon, 11+dYCarillon, 16+dXCarillon, 12+dYCarillon, WHITE );
  display.drawLine( 17+dXCarillon,  5+dYCarillon, 17+dXCarillon, 11+dYCarillon, WHITE );

  display.drawLine( 17+dXCarillon,  1+dYCarillon, 17+dXCarillon,  2+dYCarillon, WHITE );
  display.drawLine( 17+dXCarillon, 14+dYCarillon, 17+dXCarillon, 15+dYCarillon, WHITE );
  display.drawLine( 18+dXCarillon,  2+dYCarillon, 18+dXCarillon,  3+dYCarillon, WHITE );
  display.drawLine( 18+dXCarillon, 13+dYCarillon, 18+dXCarillon, 14+dYCarillon, WHITE );
  display.drawLine( 19+dXCarillon,  3+dYCarillon, 19+dXCarillon,  5+dYCarillon, WHITE );
  display.drawLine( 19+dXCarillon, 11+dYCarillon, 19+dXCarillon, 14+dYCarillon, WHITE );
  display.drawLine( 20+dXCarillon,  5+dYCarillon, 20+dXCarillon, 12+dYCarillon, WHITE );
}

void prepareIconeCarillonOFF()
{
  display.drawLine( 14+dXCarillon,  5+dYCarillon, 20+dXCarillon, 11+dYCarillon, WHITE );
  display.drawLine( 14+dXCarillon, 11+dYCarillon, 20+dXCarillon,  5+dYCarillon, WHITE );
}

void prepareIconeCarillon()
{
  display.setTextSize( 1 );
  display.setCursor( 2, 25 );
  prepareIconeCarillonBase();
  prepareIconeCarillonOFF();
  if( carillonGet )
    { prepareIconeCarillonON(); }
  else
    { prepareIconeCarillonOFF(); }
}

void initCarillon()
{
  carillon();
}

void carillon()
{
  if( carillonGet )
    { MarioBros( carillonPin ); }

  // On met le carillon en INPUT_PULLUP pour éviter les bruits
  // parasites lorsqu’il n’est pas utilisé.
  // Il n’y a pas besoin de le mettre en OUTPUT avant l’utilisation,
  // car la procédure “tone” s’en charge.
  pinMode( carillonPin, INPUT_PULLUP );
}

void initTimer()
{
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt( timer0_ISR );
  timer0_write( ESP.getCycleCount() + periodeRafraichissement );
  interrupts();
}

void timer0_ISR( void )
{
  cestlheure = true;
  timer0_write( ESP.getCycleCount() + periodeRafraichissement );
}

void initSerial()
{
  Serial.begin( 115200 );
}

void setup()
{
  initSerial();
  initGPIO();
  initEcran();
  initWifi();
  long gmtOffset;
  getTimezone( &gmtOffset );
  initHorloge( gmtOffset );
  initCarillon();
  initTimer();
}

void loop()
{
  if( cestlheure )
  {
    horloge();
    char nowChar[ 37 ];
    DateTime now = RTC.now();
    sprintf(
      nowChar,
      "Heure actuelle : %1d-%02d-%02d %02d:%02d:%02d",
      now.year(), now.month(),  now.day(),
      now.hour(), now.minute(), now.second() );
    Serial.println( nowChar );
    cestlheure = false;
  }

  // Si le bouton 1 est appuyé le son est activé ou désactivé
  if( btn1Get )
  {
    carillonGet = ! carillonGet;
    digitalWrite( LEDrouge, LEDallumee );
    horloge();
    digitalWrite( LEDrouge, LEDeteinte );
    delay( 50 );
    btn1Get = false;
  }

  // Si le bouton 2 est appuyé, on joue la mélodie,
  // même si le son est désactivé
  if( btn2Get )
  {
    MarioBros( carillonPin );
    delay( 50 );
    btn2Get = false;
  }
}

void btn1Press()
{
  btn1Get = true;
}

void btn2Press()
{
  btn2Get = true;
}
