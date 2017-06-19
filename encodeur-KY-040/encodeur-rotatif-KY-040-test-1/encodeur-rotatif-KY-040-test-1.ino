/*

LECTURE D’UN ENCODEUR ROTATIF KY-040
====================================

# DESCRIPTION DU PROGRAMME
Lecture d’un encodeur rotatif KY-040

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

juin 2017, ouilogique.com

*/

#include "encoder.h"


void setup()
{
  Serial.begin( 115200 );
  initEncodeur();
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
}


void loop()
{
  // Lecture et affichage des valeurs de l’encodeur.
  if( encodeurTourne )
  {
    displayValues( lectureEncodeur( false ) );
    encodeurTourne = false;
  }

  // Si le bouton est pressé, on remet le compteur à 0.
  if( boutonPresse )
  {
    displayValues( lectureEncodeur( true ) );
    digitalWrite( LED_BUILTIN, LOW );
    delay( 20 );
    digitalWrite( LED_BUILTIN, HIGH );
    boutonPresse = false;
  }

  yield();
}


void displayValues( encType encodeurVal )
{
  if( encodeurVal.valid )
  {
    // Pour visionnement dans le traceur série de l’IDE Arduino (CMD-SHIFT-L)
    Serial.println( encodeurVal.encVal );
  }
}
