#ifndef Defy_h
#define Defy_h

#include <arduino.h>

#define OFF HIGH
#define ON LOW

/*
 piny bez ryzyka
 D1, D2, D5, D6, D7
 
 flashowanie
 TX(D10),RX(D8)

 status boot
 D8, D3, D4
 */
#define PIN_WILGOC D5
#define PIN_SDA D1
#define PIN_SCL D2
#define LED 2

#define SEKCJA_MIN 1
#define SEKCJA_MAX 7

#define PIN_ONEWIRE D7

#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

bool isFloatChars(char * ctab);
bool isFloatString(String tString);
bool isIntChars(char * ctab) ;
 
#endif
