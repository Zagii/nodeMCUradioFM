#ifndef Config_h
#define Config_h

#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "FS.h"
#include "Defy.h"


#define PROGRAM_CONFIG_FILE "/programy.json"

#define MAX_PROGR 50

#define GODZ_ZERO 30000L
#define SEK_W_DNIU (3600*24)

#define TRYB_AUTO 'a'
#define TRYB_MANUAL 'm'

#define PLIK_NTP "NPT.json"
#define PLIK_MQTT "MQTT.json"
#define PLIK_LBL "LBL.json"
#define PLIK_TRYB "TRYB.json"
#define PLIK_WIFI "Wifi.json"
#define PLIK_PROG "PROG.json"

typedef struct 
{
  uint8_t dzienTyg;
  time_t dataOdKiedy; //dzien przedstawia date od kiedy podlewac dla godz 00:00:00, 
  time_t godzinaStartu; //a godzina o której godzinie wzgledem daty
  String tStr; //czas ms UTC z godzina startu wyrazony jako str, READONLY
  unsigned long czas_trwania_s;    //czas trwania
  uint8_t sekcja;
  uint8_t co_ile_dni;
  bool aktywny;
}Program;

class CConfig
{
  uint16_t progIle=0;
  Program prTab[MAX_PROGR];
  String sekcjeLbl[8];
  char tryb; 
  public:
    CConfig(){for(int i=0;i<8;i++){sekcjeLbl[i]="Sekcja "+String(i);}};
    void begin();
    bool loadConfig();
       bool loadProgs();
    bool loadConfigSekcjeLBL();
  
    String loadJsonStr(const char* nazwaPliku);
    bool saveProgs();
    bool saveConfigStr(const char *nazwaPliku,const char * str);

    void setProg(Program &a,uint8_t dzien, uint8_t mies, uint16_t rok,  uint8_t h, uint8_t m,  uint8_t s,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekwencja, bool aktywny);
     void setProg(Program &a,uint8_t dzien,String timeStr,time_t godzina,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekwencja, bool aktywny);
    void setProg(Program &a, Program &b);
    void addProg(Program p);
    void changeProg(Program a, uint16_t progRefID);
    void getProg(Program &a, uint16_t progRefID);
    uint16_t getProgIle(){return progIle;}
    void publishProg(Program &p,uint16_t i=-1);
    String publishProgJsonStr(Program &p,uint16_t i=-1);
    String publishTabProgJsonStr(uint16_t i);
    void delProg(uint16_t id);
    void publishAllProg();
    bool checkRangeProg(Program &p,time_t sysczas_s);
    void printCzas(time_t t);
    uint8_t wlaczoneSekcje(time_t sysczas_s);
    void setTryb(char t){tryb=t; DPRINT("nowy TRYB=");DPRINTLN(t);};
    char getTryb(){return tryb;};
    void setSekcjaLbl(uint8_t id,String lbl);
    String getSekcjaLbl(uint8_t id);
};


#endif
