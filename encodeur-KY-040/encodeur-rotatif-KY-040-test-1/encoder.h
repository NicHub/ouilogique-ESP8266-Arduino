/*
*/

#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

typedef struct
{
  int8_t encVal;
  bool valid;
} encType;

encType lectureEncodeur( bool reset );
int8_t lectureBruteEncodeur();
void interruptionEncodeur();
void interruptionBouton();

// Modifier ici la sensibilité de l’encodeur pour qu’il ne réagisse
// pas trop vite. Cette valeur représente le nombre de millisecondes
// avant un changement sur la sortie.
static const long sensibiliteEncodeur = 50;

// Gamme de sortie de l’encodeur. À modifier en fonction de l’application.
static const int8_t minVal = 0;
static const int8_t maxVal = 10;

// Connexions de l’encodeur
#define ENC_A D1
#define ENC_B D2
#define ENC_C D3


#define ENC_A_Read   digitalRead( ENC_A )
#define ENC_B_Read   digitalRead( ENC_B )
#define ENC_C_Read ! digitalRead( ENC_C )

bool ENC_A_Val;
bool ENC_B_Val;
bool ENC_C_Val;

bool encodeurTourne = false;
bool boutonPresse   = false;


void initEncodeur()
{
  pinMode( ENC_A, INPUT ); // Il y a une pull-up de 10 kΩ sur l’encodeur.
  pinMode( ENC_B, INPUT ); // Il y a une pull-up de 10 kΩ sur l’encodeur.
  pinMode( ENC_C, INPUT_PULLUP );

  attachInterrupt( digitalPinToInterrupt( ENC_A ), interruptionEncodeur, CHANGE );
  attachInterrupt( digitalPinToInterrupt( ENC_B ), interruptionEncodeur, CHANGE );
  attachInterrupt( digitalPinToInterrupt( ENC_C ), interruptionBouton,   FALLING ); // !! RISING et FALLING sont inversés en INPUT_PULLUP.
  encodeurTourne = true;
}


encType lectureEncodeur( bool reset )
{
  static int8_t compteur = 0;
  int8_t encodeurVal;
  encType encodeurValOut;
  encodeurValOut.encVal = 0;
  encodeurValOut.valid = true;

  if( reset )
  {
    compteur = 0;
    encodeurValOut.encVal = 0;
    encodeurValOut.valid = true;
    return encodeurValOut;
  }

  encodeurVal = lectureBruteEncodeur();
  if( encodeurVal == 0 )
  {
    encodeurValOut.encVal = compteur;
    encodeurValOut.valid = false;
    return encodeurValOut;
  }

  // Mise à jour du compteur et coercion dans la gamme minVal..maxVal
  if( compteur < minVal || ( compteur == minVal && encodeurVal < 0 ) )
    compteur = minVal;
  else if( compteur > maxVal || ( compteur == maxVal && encodeurVal > 0 ) )
    compteur = maxVal;
  else
    compteur = compteur + encodeurVal;

  encodeurValOut.encVal = compteur;
  encodeurValOut.valid = true;

  return encodeurValOut;
}


int8_t lectureBruteEncodeur()
{
  // Lecture des signaux de l’encodeur et comparaison
  // avec les valeurs possibles dans le code de Gray.
  // Ceci permet de filtrer la plupart des rebonds de l’encodeur.
  // Les valeurs à 0 représentent les transitions impossibles
  // et qui doivent être supprimées (resultat = 0).
  // Les valeurs à 1 représentent les transitions dans le sens horaire.
  // Les valeurs à -1 représentent les transitions dans le sens anti-horaire.
  static const int8_t enc_states[ 16 ] PROGMEM =
    { 0,-1, 1, 0, 1, 0, 0,-1,-1, 0, 0, 1, 0, 1,-1, 0 };
  static uint8_t old_AB = 0;

  old_AB <<= 2;
  bitWrite( old_AB, 0, ENC_A_Val );
  bitWrite( old_AB, 1, ENC_B_Val );
  uint8_t resultat = pgm_read_byte( &enc_states[ ( old_AB & 0x0F ) ] );

  // On accumule “maxVal” résulats dans un sens ou dans l’autre
  // avant de bouger. Ceci permet de supprimer les derniers rebonds
  // de l’encodeur qui n’ont pas été filtrés ci-dessus.
  if( resultat != 0 )
  {
    static int8_t resultatCumul = 0;
    const int8_t maxVal = 2;
    resultatCumul += resultat;
    if( resultatCumul >= maxVal )
    {
      resultatCumul = maxVal;
      resultat = 1;
    }
    else if( resultatCumul <= -maxVal )
    {
      resultatCumul = -maxVal;
      resultat = -1;
    }
    else
      { resultat = 0; }
  }

  // On rejete quelques lectures pour limiter la sensibilité de l’encodeur
  // et éviter ainsi qu’il ne tourne trop vite.
  static long lastT = millis();
  if( millis() - lastT < sensibiliteEncodeur )
    { resultat = 0; }
  else
    { lastT = millis(); }

  // On retourne le résultat.
  return( resultat );
}


void interruptionEncodeur()
{
  ENC_A_Val = ENC_A_Read;
  ENC_B_Val = ENC_B_Read;
  ENC_C_Val = ENC_C_Read;
  encodeurTourne = true;
}


void interruptionBouton()
{
  boutonPresse = true;
}


#endif //  ENCODER_H
