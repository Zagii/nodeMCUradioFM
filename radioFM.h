#ifndef CradioFM_H
#define CradioFM_H


#include <Wire.h>

#include <radio.h>
#include <SI4703.h>
#include <RDSParser.h>

RADIO_FREQ preset[] = {
  8840,//   Radio Pogoda (Warszawa)   Rondo 1
  8900 ,//  Radio Maryja Radio Watyka�skie   Pa�ac Kultury i Nauki
  8940 ,//  Radio Hobby   Legionowo, Komin PEC
  8980 ,//  Radio WAWA  Rondo 1
  9060 ,//  RMF FM  Rondo 1
  9100 ,//  RMF FM  RTCN Raszyn
  9200 ,//  Polskie Radio 24  Hotel Marriott
  9240 ,//  Polskie Radio Jedynka   Hotel Marriott
  9280 ,//  POP Radio   Pruszk�w, Komin EC Pruszk�w II
  9330 ,//  Eska ROCK   Rondo 1
  9400 ,//  Meloradio   Pa�ac Kultury i Nauki
  9450 ,//  Radio Bogoria   Grodzisk Mazowiecki, Komin Gedeon Richter
  9470 ,//  Radio FaMa (Wo�omin)  Wo�omin, Komin ZEC
  9510  ,// Katolickie Radio Zbrosza Du�a Radio Maryja  Zbrosza Du�a
  9540 ,//  Radio Maryja Radio Watyka�skie   Skierniewice, RON Bartniki
  9580,//   RMF Maxxx   Pa�ac Kultury i Nauki
  9650 ,//  Radio Plus (Warszawa)   Pa�ac Kultury i Nauki
  9710 ,//  Radio Kampus  Rondo 1
  9770 ,//  Radio Tok FM  Pa�ac Kultury i Nauki
  9810 ,//  Radio Victoria  Mszczon�w, Wie�a Ci�nie�
  9830 ,//  RMF Classic   Pa�ac Kultury i Nauki
  9880 ,//  Polskie Radio Tr�jka  RTCN Raszyn
  9910 ,//  Polskie Radio Tr�jka  Hotel Marriott
  9950 ,//  Radio Pogoda (Warszawa)   Rondo 1 (0-12 godz)
  9950 ,//  Radio Armii Krajowej Jutrzenka  ul. Dereniowa (12-24 godz)
10010 ,// Radio Z�ote Przeboje (Warszawa)   Hotel Marriott
10100 ,// Polskie Radio RDC   Pa�ac Kultury i Nauki
10150 ,// Chillizet   Pa�ac Kultury i Nauki
10170 ,// Katolickie Radio Podlasie   Siedlce, RTCN �osice
10200 ,// Radio Muzo.FM   Pa�ac Kultury i Nauki
10240 ,// Polskie Radio Jedynka   RTCN Raszyn
10270 ,// Radio Niepokalan�w  Skierniewice, RON Bartniki
10300 ,// Radio Kolor   Pa�ac Kultury i Nauki
10340 ,// Polskie Radio RDC ?????????   Siedlce, RTCN �osice
10370 ,// Rock Radio (Warszawa)   Pa�ac Kultury i Nauki
10410 ,// Radio FaMa (�yrard�w)   �yrard�w, Komin PEC �yrard�w
10440 ,// Radio Vox FM  Pa�ac Kultury i Nauki
10490 ,// Polskie Radio Dw�jka  Hotel Marriott
10560 ,// Radio Eska (Warszawa)   Rondo 1
10620 ,// Radio Warszawa Radio Watyka�skie   Pa�ac Kultury i Nauki
10640 ,// Radio Vox FM  Gr�jec, ul. Mogielnicka
10680 ,// Antyradio   Pa�ac Kultury i Nauki
10750 // Radio ZET   Pa�ac Kultury i Nauki
};
/// State definition for this radio implementation.
enum RADIO_STATE {
  STATE_PARSECOMMAND, ///< waiting for a new command character.
  
  STATE_PARSEINT,     ///< waiting for digits for the parameter.
  STATE_EXEC          ///< executing the command.
};
// callback functions for RDS
RDSParser rds;/// get a RDS parser
void DisplayServiceName(char *name);
void receiveTextRDS(char *name);
void receiveTimeRDS(uint8_t h,uint8_t m);
void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4);
bool byloSN=false;
bool byloText=false;
bool byloTime=false;
String rdsLastName="";
String rdsLastText="";
String rdsLastTime="";

class CradioFM
{
    int    i_sidx=20;        ///< Start at Station with index=5
    SI4703   radio;    ///< Create an instance of a SI4703 chip radio.

    RADIO_STATE state; ///< The state variable is used for parsing input characters.
    static unsigned long nextFreqTime = 0;
    static unsigned long nextRadioInfoTime = 0;
    RADIO_FREQ f = 0;
    static RADIO_FREQ lastf = 0;
       // some internal static values for parsing the input
    static char command;
    static int16_t value;
     // int newPos;
    String statusStr="";
    bool testRDS();
    bool czekaStatus=false;

    public:
    CradioFM(){};
    void DisplayFrequency(RADIO_FREQ f);
    void runSerialCommand(char cmd, int16_t value);
    void begin();
    bool loop(); //zwraca true jesli zmienil sie status
    void checkSerial();
    void setStatusStr();
    void clearStatusFlag(){czekaStatus=false;}
    String getStatusStr(){return statusStr;}
};
#endif