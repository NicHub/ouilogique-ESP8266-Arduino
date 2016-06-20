/*
 * aTunes.h
 *
 * http://www.instructables.com/id/How-to-easily-play-music-with-buzzer-on-arduino-Th/
 * Source http://tny.cz/e525c1b2
 *
 * NB: ALL NOTES DEFINED WITH STANDARD ENGLISH NAMES, EXCEPT FROM "A"
 * THAT IS CALLED WITH THE ITALIAN NAME "LA" BECAUSE A0,A1...ARE THE ANALOG PINS ON ARDUINO.
 * (Ab IS CALLED Ab AND NOT LAb)
 *
 */

#include "Arduino.h"


#define	aTunesC0 16.35
#define	aTunesDb0	17.32
#define	aTunesD0	18.35
#define	aTunesEb0	19.45
#define	aTunesE0	20.60
#define	aTunesF0	21.83
#define	aTunesGb0	23.12
#define	aTunesG0	24.50
#define	aTunesAb0	25.96
#define	aTunesLA0	27.50
#define	aTunesBb0	29.14
#define	aTunesB0	30.87
#define	aTunesC1	32.70
#define	aTunesDb1	34.65
#define	aTunesD1	36.71
#define	aTunesEb1	38.89
#define	aTunesE1	41.20
#define	aTunesF1	43.65
#define	aTunesGb1	46.25
#define	aTunesG1	49.00
#define	aTunesAb1	51.91
#define	aTunesLA1	55.00
#define	aTunesBb1	58.27
#define	aTunesB1	61.74
#define	aTunesC2	65.41
#define	aTunesDb2	69.30
#define	aTunesD2	73.42
#define	aTunesEb2	77.78
#define	aTunesE2	82.41
#define	aTunesF2	87.31
#define	aTunesGb2	92.50
#define	aTunesG2	98.00
#define	aTunesAb2	103.83
#define	aTunesLA2	110.00
#define	aTunesBb2	116.54
#define	aTunesB2	123.47
#define	aTunesC3	130.81
#define	aTunesDb3	138.59
#define	aTunesD3	146.83
#define	aTunesEb3	155.56
#define	aTunesE3	164.81
#define	aTunesF3	174.61
#define	aTunesGb3	185.00
#define	aTunesG3	196.00
#define	aTunesAb3	207.65
#define	aTunesLA3	220.00
#define	aTunesBb3	233.08
#define	aTunesB3	246.94
#define	aTunesC4	261.63
#define	aTunesDb4	277.18
#define	aTunesD4	293.66
#define	aTunesEb4	311.13
#define	aTunesE4	329.63
#define	aTunesF4	349.23
#define	aTunesGb4	369.99
#define	aTunesG4	392.00
#define	aTunesAb4	415.30
#define	aTunesLA4	440.00
#define	aTunesBb4	466.16
#define	aTunesB4	493.88
#define	aTunesC5	523.25
#define	aTunesDb5	554.37
#define	aTunesD5	587.33
#define	aTunesEb5	622.25
#define	aTunesE5	659.26
#define	aTunesF5	698.46
#define	aTunesGb5	739.99
#define	aTunesG5	783.99
#define	aTunesAb5	830.61
#define	aTunesLA5	880.00
#define	aTunesBb5	932.33
#define	aTunesB5	987.77
#define	aTunesC6	1046.50
#define	aTunesDb6	1108.73
#define	aTunesD6	1174.66
#define	aTunesEb6	1244.51
#define	aTunesE6	1318.51
#define	aTunesF6	1396.91
#define	aTunesGb6	1479.98
#define	aTunesG6	1567.98
#define	aTunesAb6	1661.22
#define	aTunesLA6	1760.00
#define	aTunesBb6	1864.66
#define	aTunesB6	1975.53
#define	aTunesC7	2093.00
#define	aTunesDb7	2217.46
#define	aTunesD7	2349.32
#define	aTunesEb7	2489.02
#define	aTunesE7	2637.02
#define	aTunesF7	2793.83
#define	aTunesGb7	2959.96
#define	aTunesG7	3135.96
#define	aTunesAb7	3322.44
#define	aTunesLA7	3520.01
#define	aTunesBb7	3729.31
#define	aTunesB7	3951.07
#define	aTunesC8	4186.01
#define	aTunesDb8	4434.92
#define	aTunesD8	4698.64
#define	aTunesEb8	4978.03

// DURATION OF THE NOTES
#define aTunesBPM 105              // you can change this value changing all the others
#define aTunesH 2*aTunesQ          // half 2/4
#define aTunesQ 60000/aTunesBPM    // quarter 1/4
#define aTunesE aTunesQ/2          // eighth 1/8
#define aTunesS aTunesQ/4          // sixteenth 1/16
#define aTunesW 4*aTunesQ          // whole 4/4
#define aTunesSeparator 4          // delay duration should always be 1 ms more than the note in order to separate them.


/*
 * MarioBros
 */
void
MarioBros( int buzzerPin )
{
	// tone( pin, note, duration )

	tone( buzzerPin, aTunesE7,  aTunesS   );        delay( aTunesSeparator+aTunesS   );
	tone( buzzerPin, aTunesE7,  aTunesS   );        delay( aTunesSeparator+aTunesE   );
	tone( buzzerPin, aTunesE7,  aTunesS   );        delay( aTunesSeparator+aTunesE   );

	tone( buzzerPin, aTunesC7,  aTunesS   );        delay( aTunesSeparator+aTunesS   );
	tone( buzzerPin, aTunesE7,  aTunesS   );        delay( aTunesSeparator+aTunesE   );
	tone( buzzerPin, aTunesG7,  aTunesS   );        delay( aTunesSeparator+aTunesQ+aTunesS );

	tone( buzzerPin, aTunesG6,  aTunesS   );        delay( aTunesSeparator+aTunesQ+aTunesS );
}
