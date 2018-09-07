/*
 PubSubClient -mqtt


 piny bez ryzyka
 D1, D2, D5, D6, D7
 
 flashowanie
 TX(D10),RX(D8)

 status boot
 D8, D3, D4
*/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <pcf8574_esp.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "Defy.h"
#include "CWifi.h"
#include "Config.h"
#include "CWebSerwer.h"



////////////pcf
PCF857x pcf8574(0b00111000, &Wire);
////////////////

CWifi wifi;
PubSubClient *mqtt;

CConfig conf;

CWebSerwer web;

char tmpTopic[MAX_TOPIC_LENGHT];
char tmpMsg[MAX_MSG_LENGHT];

////////////// sprawdzic ntp
////////// https://github.com/arduino-libraries/NTPClient

unsigned long czasLokalnyMillis=0;
unsigned long czasLokalny=0;

unsigned long sLEDmillis=0;

byte stanSekcji=0;
bool czekaNaPublikacjeStanuMQTT=false;
bool czekaNaPublikacjeStanuWS=false;
bool czekaNaPublikacjeStanuHW=false;

bool czekaNaPublikacjeLBL=false;
bool czekaNaPublikacjePROG=false;
bool czekaNaPublikacjeKONF=false;
bool czekaNaPublikacjeSTAT=false;

uint8_t publicID=0;
unsigned long publicMillis=0;

/////////////// czujnik wilgoci ///////////////////////
int stanCzujnikaWilgoci;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
////////////////////// czujnik wilgoci koniec //////////

void(* resetFunc) (void) = 0; //declare reset function @ address 0



void callback(char* topic, byte* payload, unsigned int length) 
{
  char* p = (char*)malloc(length+1);
  memcpy(p,payload,length);
  p[length]='\0';
  if(strstr(topic,"watchdog"))
  {
    DPRINT("Watchdog msg=");
    DPRINT(p);
    DPRINT(" teraz=");
   
    if(isNumber(p))
      wifi.setWDmillis(strtoul (p, NULL, 0));
    DPRINTLN(wifi.getWDmillis());
    

  }
 if(topic[strlen(topic)-1]=='/')
    {topic[strlen(topic)-1] = '\0';}
 DPRINT("Debug: callback topic=");
 DPRINT(topic);
 DPRINT(" msg=");
 DPRINTLN(p);
 parsujRozkaz(topic,p);
  free(p);
}

////////////// obsluga websocket

void wse(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  web.webSocketEvent(num,type,payload,length);
  if(type==WStype_TEXT)
  {
    char* p = (char*)malloc(length+1);
    memcpy(p,payload,length);
    p[length]='\0';
    DPRINT("webSocket TEXT: ");DPRINTLN(p);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(p);
    if (!root.success()) 
    {
      Serial.println("parseObject() failed");
      Serial.println(p);
      free(p);
      DPRINTLN("return");
      return;
    }

    char* topic="";
    char msg[200]="";
    
    for (auto kv : root) {
       topic=(char*)kv.key;
        DPRINT(topic);DPRINTLN("root[topic]=");//Serial.println(String(root[topic]));
     /*   kv.value.prettyPrintTo(Serial); Serial.println(" # ");kv.value.printTo(Serial);*/
       if(root[topic].is<const char*>())
       {
          DPRINTLN("msg char*");
          strcpy(msg,(char*)kv.value.as<char*>());
       }else 
       { 
        if(kv.value.is<JsonObject>())
        {
            DPRINTLN(" kv obj ");
            root[topic].printTo(msg);
            DPRINT("msg JSON: ");DPRINTLN(msg);
        }else if(kv.value.is<JsonArray>())
         {
           DPRINTLN(" kv array ");
         }else  if(kv.value.is<unsigned int>())
         {
          DPRINTLN(" kv uint ");
          itoa ((uint8_t)kv.value.as<unsigned int>(), msg, 10);
          }else {   DPRINTLN(" kv undef... ");   }
       }
       DPRINT(topic);DPRINT("=");DPRINTLN(msg);
       parsujRozkaz(topic,msg);
     }
    free(p);
  }
}

/////////////////////////SETUP///////////////////////////
void setup()
{
 
  Serial.begin(115200);
   
  DPRINTLN("");
  DPRINTLN("Setup Serial");
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,ON);
 
  //Setup PCF8574
  Wire.pins(PIN_SDA, PIN_SCL);//SDA - D1, SCL - D2
  Wire.begin();
  
  pinMode(PIN_WILGOC, INPUT_PULLUP); //czujnik wilgoci

  pcf8574.begin( 0x00 ); //8 pin output
 // pcf8574.resetInterruptPin();
  wylaczWszystko();
  //

//////////////// wifi i mqtt init ///////////
  wifi.begin();
  mqtt=wifi.getMQTTClient();
  mqtt->setCallback(callback);
///////////// koniec wifi i mqtt init /////////

conf.begin();
conf.setTryb(TRYB_AUTO);
//delay(1000);
DPRINTLN("Programy");
/*Program pp;
conf.setProg(pp,1, 1, 1970, 7, 0,0,8*60,1,1); 
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 8,10,8*60,1,2);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 16,20,8*60,1,3);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 24,30,8*60,1,4);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 32,40,8*60,1,6);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 40,50,5*60,1,5);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 0,0,8*60,1,1); 
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 8,10,8*60,1,2);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 16,20,8*60,1,3);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 24,30,8*60,1,4);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 32,40,8*60,1,6);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 40,50,5*60,1,5);
conf.addProg(pp);
conf.publishAllProg();

conf.saveConfig();
*/
conf.publishAllProg();

//////////////// odczyt WiFi
String wifiJson=conf.loadJsonStr(PLIK_WIFI);
DPRINT("Konfig wifi:");DPRINTLN(wifiJson);
wifi.zmianaAP(wifiJson);
String mqttJson=conf.loadJsonStr(PLIK_MQTT);
DPRINT("Konfig mqtt:");DPRINTLN(mqttJson);
wifi.setupMqtt(mqttJson);

web.begin();
WebSocketsServer * webSocket=web.getWebSocket();
webSocket->onEvent(wse);
}

void wylaczWszystko()
{
  zmienStanSekcjiAll(0);
     czekaNaPublikacjeStanuMQTT=true;
   czekaNaPublikacjeStanuWS=true;
   czekaNaPublikacjeStanuHW=true;
}
void zmienStanSekcjiAll(uint8_t stan)
{
  if(stanSekcji==stan) return;
   stanSekcji=stan;
   czekaNaPublikacjeStanuMQTT=true;
   czekaNaPublikacjeStanuWS=true;
   czekaNaPublikacjeStanuHW=true;
}
void zmienStanSekcji(uint8_t sekcjanr,uint8_t stan)
{
  DPRINT("zmienStanSekcji nr=");DPRINT(sekcjanr);DPRINT(", stan=");DPRINT(stan);DPRINT(", stanSekcji=");DPRINTLN(stanSekcji);
  uint8_t x=bitRead(stanSekcji,sekcjanr);
  if(x==stan)return;
  if(stan==1)
  {
    DPRINT("ON");
    bitSet(stanSekcji,sekcjanr);
  }else
  {
    DPRINT("OFF");
    bitClear(stanSekcji,sekcjanr);
  }
   DPRINT("zmienStanSekcji koniec nr=");DPRINT(sekcjanr);DPRINT(", stan=");DPRINT(stan);DPRINT(", stanSekcji=");DPRINTLN(stanSekcji);
  //web.zmienStanSekcji(stanSekcji);
  czekaNaPublikacjeStanuMQTT=true;
  czekaNaPublikacjeStanuWS=true;
  czekaNaPublikacjeStanuHW=true;
}
void publikujStanSekcjiMQTT()
{
   if(wifi.getConStat()!=CONN_STAT_WIFIMQTT_OK)return;
   
   byte b = stanSekcji;//pcf8574.read8();
   DPRINT("publikujStanSekcjiMQTT ");DPRINT(b);DPRINT(", ");DPRINTLN(stanSekcji);
   for(int i=SEKCJA_MIN;i<=SEKCJA_MAX;i++)
   {
      //if(b&(1<<i))
      if(bitRead(b,i))
      {
          sprintf(tmpTopic,"%s/SEKCJA/%d/",wifi.getOutTopic(),i);
          strcpy(tmpMsg,"1");
      }else
      {
          sprintf(tmpTopic,"%s/SEKCJA/%d/",wifi.getOutTopic(),i);
          strcpy(tmpMsg,"0");
      }
      wifi.RSpisz(tmpTopic,tmpMsg);
   }
   DPRINT("######### ZMIANA STANU WYJSC ############ ");DPRINTLN(stanSekcji,BIN);
}

 void parsujRozkaz(char *topic, char *msg)
 {
  DPRINT("parsujRozkaz topic=");DPRINT(topic);DPRINT(", msg=");DPRINTLN(msg);
   char *ind=NULL;
   ///////////////// SEKCJA  //////////////////////////
   ind=strstr(topic,"SEKCJA/");
   if(ind!=NULL)
   {
    DPRINTLN(ind);
     ind+=strlen("SEKCJA/"); //bo jeszcze nr sekcji
    DPRINTLN(ind);
     if(isIntChars(ind))
     {
        if(isIntChars(msg))
        {
          if(msg[0]=='0')
          {
            zmienStanSekcji(atoi(ind),0);
          }else
          {
            zmienStanSekcji(atoi(ind),1);
          }
        }else
        {
          DPRINT("ERR dla topic SEKCJAx msg ");DPRINT(msg);DPRINT(" nie int, linia:");DPRINTLN(__LINE__);
        }
     }else
     {
         DPRINT("ERR topic ");DPRINT(topic);DPRINT(" nie int, linia:");DPRINTLN(__LINE__);
     }
     return;
    }
    //////////////////////////////////////
    ind=strstr(topic,"TRYB");
    if(ind!=NULL)
    {
       if(msg[0]=='a')
       {
          conf.setTryb(TRYB_AUTO);  
       }
       if(msg[0]=='m')
       {
            conf.setTryb(TRYB_MANUAL);
       }
        return;
    }
    ind=strstr(topic,"CZAS");
    if(ind!=NULL)
    {
      czasLokalny=atoi(msg);
      return;
    }
    ind=strstr(topic,"DEL_PROG");
    if(ind!=NULL)
    {
      uint16_t delID=atoi(msg);
      conf.delProg(delID);
      conf.saveProgs();  
      czekaNaPublikacjePROG=true;
    }
    ind=strstr(topic,"GET");
    if(ind!=NULL)
    {
       ind=strstr(msg,"SLBL");
       if(ind!=NULL)
       {
          czekaNaPublikacjeLBL=true;
       }
       ind=strstr(msg,"PROG");
       if(ind!=NULL)
       {
        czekaNaPublikacjePROG=true;
       }
       ind=strstr(msg,"KONF");
       if(ind!=NULL)
       {
        czekaNaPublikacjeKONF=true;
       }
       ind=strstr(msg,"STAT");
       if(ind!=NULL)
       {
        czekaNaPublikacjeSTAT=true;
       }
      
       return;
    }
    //////////////////////// komendy ktore maja jsona jako msg /////////////////////////
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(msg);
    DPRINT("msg=");DPRINTLN(msg);
    if (!json.success()) {
       DPRINTLN("Blad parsowania json !!!!");
      return;
    }
    DPRINTLN("Parsowanie zagniezdzonego jsona");
    String jsS;
    json.printTo(jsS);
    DPRINTLN(jsS);
    ind=strstr(topic,"NTP");
    if(ind!=NULL)
    {
      String host=json["host"];
      unsigned long offset=json["offset"];
      wifi.setNTP(host,offset);
      // zapisz do pliku
      conf.saveConfigStr(PLIK_NTP,jsS.c_str());
    }
    
    ind=strstr(topic,"Wifi");
    if(ind!=NULL)
    {
     String tryb=json["tryb"];
      String ssid=json["ssid"];
       String pass=json["pass"];
      if(tryb=="STA")
      {
        wifi.zmianaAP(ssid,pass);
        conf.saveConfigStr(PLIK_WIFI,jsS.c_str());
        // zapisz do pliku
      }else
      {/// utworzyc AP
        
      }
      czekaNaPublikacjeKONF=true;
    }
    ind=strstr(topic,"Mqtt");
    if(ind!=NULL)
    {     
     String host=json["host"];
      uint16_t port=json["port"];
       String user="";
       String pwd="";
      if(json.containsKey("user"))
      {
       user=String((const char*)json["user"]);
      }
      if(json.containsKey("pwd"))
      {
        pwd=String((const char*)json["pwd"]);
      }
      wifi.setupMqtt(host,port,user,pwd);
      // zapisz do pliku
      conf.saveConfigStr(PLIK_MQTT,jsS.c_str());
      czekaNaPublikacjeKONF=true;
    }
    ind=strstr(topic,"LBL");
    if(ind!=NULL)
    {
      uint8_t id=json["id"];
     
      String lbl=json["lbl"];
      conf.setSekcjaLbl(id,lbl);
      String str="{\"LBL\":[";
      for(int i=0;i<8;i++)
      {
        str+="{\"id\":"+String(i)+",\"lbl\":\""+conf.getSekcjaLbl(i)+"\"}";
        if(i<7)str+=",";
      }
      str+="]}";
      conf.saveConfigStr(PLIK_LBL,str.c_str());
      czekaNaPublikacjeLBL=true;
    }
    
    ind=strstr(topic,"PROG");
    if(ind!=NULL)
    {
       if(json.containsKey("id"))
      {
        uint16_t i=json["id"];
        Program a;
        conf.getProg(a,i);
        if(json.containsKey("ms")&&json.containsKey("dzienTyg"))
        {
          /////////// tu przeba przeliczyc
          //a.dataOdKiedy=json["dt"];
          a.dzienTyg=json["dzienTyg"];
          a.godzinaStartu=json["ms"];
        }
        if(json.containsKey("tStr"))a.tStr=String(json["tStr"].as<char*>());
        if(json.containsKey("okresS"))a.czas_trwania_s=json["okresS"];
        if(json.containsKey("sekcja"))a.sekcja=json["sekcja"];
        if(json.containsKey("coIle"))a.co_ile_dni=json["coIle"];
        if(json.containsKey("aktywny"))a.aktywny=json["aktywny"];
        ///////////////
        //set progr tab
        conf.changeProg(a,i);
        conf.saveProgs();
        czekaNaPublikacjePROG=true;
      }else
      {
        DPRINTLN("Nowy program");
        if(json.containsKey("dzienTyg")&&json.containsKey("ms")&&json.containsKey("okresS")&&json.containsKey("sekcja")&&json.containsKey("coIle")&&json.containsKey("aktywny"))
        {
          Program a;
          conf.setProg(a,json["dzienTyg"],json["tStr"],json["ms"],json["okresS"],json["coIle"],json["sekcja"],json["aktywny"]);
          conf.addProg(a);
          conf.saveProgs();
          czekaNaPublikacjePROG=true;
        }else
          {DPRINTLN("Za mało parametrów by dodać program.");}
      }
    }
 }

unsigned long d=0;

String millisTimeStr;
void loop()
{

  /////////////// czujnik wilgoci //////////////////
   int reading = digitalRead(PIN_WILGOC);
    if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stanCzujnikaWilgoci) {
      stanCzujnikaWilgoci = reading;
    }
  }
  lastButtonState = reading;
/////////////// czujnik wilgoci koniec //////////////////   
  String infoStr;
   if(millis()-czasLokalnyMillis>1000)
  {
    czasLokalnyMillis=millis();
    czasLokalny++;
    millisTimeStr=String(wifi.TimeToString(millis()/1000));
    //// przygotowanie ogólnego statusu
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
     /*
    *  1 styczen 2010 to
    *  Epoch timestamp: 1262304000
    *  Timestamp in milliseconds: 1262304000000
    */
    if(wifi.getEpochTime()<1262304000)// czyli brak polaczenia z NTP
    {
       root["CZAS"]=czasLokalny; 
    }else
    {
      root["CZAS"]= wifi.getEpochTime();
    }
    root["SEKCJE"]=stanSekcji;
    root["TRYB"]=String(conf.getTryb());
    root["GEO"]="Duchnice";
    root["TEMP"]=20.1f;
    root["CISN"]=1023.34f;
    root["DESZCZ"]=stanCzujnikaWilgoci;
    root["SYSTIME"]=millisTimeStr;
    root.printTo(infoStr); 
    char tmpTopic[MAX_TOPIC_LENGHT];
    sprintf(tmpTopic,"%s/INFO/",wifi.getOutTopic());
    
    wifi.RSpisz(String(tmpTopic),infoStr,true);
  ////////////
  }
 delay(5);
  wifi.loop();
 delay(5);

    web.loop(czasLokalny, infoStr);
  
 delay(5);

 
 
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     sLEDmillis=millis();
    // DPRINT( "[");DPRINT(wifi.getTimeString());DPRINT("] ");DPRINTLN(wifi.getEpochTime());
     if(conf.getTryb()==TRYB_AUTO) // test czy programator każe wlączyć
     {
      uint8_t sekcjaProg=conf.wlaczoneSekcje(wifi.getEpochTime());
        Serial.println(sekcjaProg,BIN);
      zmienStanSekcjiAll(sekcjaProg);
     }
    if(stanCzujnikaWilgoci==LOW);
    {
      //DPRINTLN("WYLACZANIE Z POWODU DESZCZU !!!!");
   //   zmienStanSekcjiAll(0);
    }
   }
   /////////////////// obsluga hardware //////////////////////
    if(czekaNaPublikacjeStanuHW)
    {
      
        pcf8574.write8(~stanSekcji);
        czekaNaPublikacjeStanuHW=false;
        delay(5);
    }
    /////////// publikowanie ///////////////
    if(czekaNaPublikacjeStanuMQTT)
    {
        publikujStanSekcjiMQTT();  // na podstawie pcf8574
        czekaNaPublikacjeStanuMQTT=false;   
        delay(10);
    }
    if(czekaNaPublikacjeStanuWS)
    {
        web.publikujStanSekcji(stanSekcji);
        czekaNaPublikacjeStanuWS=false;     
        delay(10);
    }
    
   if(czekaNaPublikacjeLBL)
   {
      String str;
      for(int i=1;i<7;i++)
      {
        str="{\"id\":"+String(i)+",\"lbl\":\""+conf.getSekcjaLbl(i)+"\"}";
        char tmpTopic[MAX_TOPIC_LENGHT];
        sprintf(tmpTopic,"%s/LBL/",wifi.getOutTopic());
        wifi.RSpisz((const char*)tmpTopic,(char*)str.c_str());
        String js=String("{\"LBL\":")+str+"}";
        DPRINTLN(js);
         web.sendWebSocket(js.c_str());
      }
      delay(10);      
      czekaNaPublikacjeLBL=false;
   }
   if(czekaNaPublikacjePROG)
   {
    DPRINTLN("PublikacjaPROG");
     char tmpTopic[MAX_TOPIC_LENGHT];
     sprintf(tmpTopic,"%s/INIT_PROGS/",wifi.getOutTopic());
      DPRINTLN(tmpTopic);
      String jjs=String(conf.getProgIle());
      DPRINTLN(jjs);
     wifi.RSpisz(String(tmpTopic),jjs);
     jjs=String("{\"INIT_PROGS\":")+String(conf.getProgIle())+"}";
     DPRINTLN(jjs);
     web.sendWebSocket(jjs.c_str());
      delay(5);
     sprintf(tmpTopic,"%s/PROG/",wifi.getOutTopic());
    for(uint16_t i=0;i<conf.getProgIle();i++)
    {
      String str=conf.publishTabProgJsonStr(i);
      wifi.RSpisz((const char*)tmpTopic,(char*)str.c_str());
      String js=String("{\"PROG\":")+str+"}";
      DPRINTLN(js);
      web.sendWebSocket(js.c_str());
      delay(1);
    }
    czekaNaPublikacjePROG=false;
    delay(10);
   }
   if(czekaNaPublikacjeKONF)
   {
    //ntp
    String tStr=wifi.getNTPjsonStr();
    char tmpTopic[MAX_TOPIC_LENGHT];
    sprintf(tmpTopic,"%s/NTP/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,(char*)tStr.c_str());
    String js=String("{\"NTP\":")+tStr+"}";
    web.sendWebSocket((const char*)js.c_str());
    //Wifi
    tStr=wifi.getWifijsonStr(); 
    sprintf(tmpTopic,"%s/Wifi/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,(char*)tStr.c_str());
    js=String("{\"Wifi\":")+tStr+"}";
    web.sendWebSocket((const char*)js.c_str());
    //Mqtt
    tStr=wifi.getMQTTjsonStr(); 
    sprintf(tmpTopic,"%s/Mqtt/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,(char*)tStr.c_str());
    js=String("{\"Mqtt\":")+tStr+"}";
    web.sendWebSocket((const char*)js.c_str());

    czekaNaPublikacjeKONF=false;
    delay(10);
   }
   if(czekaNaPublikacjeSTAT)
   {//tryb
    //sekcja
    //geo,temp,czas,cisn,deszcz
    czekaNaPublikacjeSTAT=false;
    delay(5);
   }
   
  ///////////////////// status LED /////////////////////////
            switch(wifi.getConStat())
            {
              case CONN_STAT_NO: ///----------__------------__  <-- ten stan praktycznie nie występuje
                if(d<2800)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFI_CONNECTING: // ------_-_---------_-_----
                if(d>=0&&d<1000)digitalWrite(LED,ON);
                if(d>=1000&&d<1300)digitalWrite(LED,OFF);
                if(d>=1300&&d<1600)digitalWrite(LED,ON);
                if(d>=1600&&d<1900)digitalWrite(LED,OFF);
                if(d>=1900)digitalWrite(LED,ON);
                    break;
              case CONN_STAT_WIFI_OK: // ---___---___---___ <-- ten stan praktycznie nie występuje
                if(d>=0&&d<700)digitalWrite(LED,ON);
                if(d>=700&&d<1400)digitalWrite(LED,OFF);
                if(d>=1400&&d<2100)digitalWrite(LED,ON);
                if(d>=2100)digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFIMQTT_CONNECTING:// ---___---___---___
                  digitalWrite(LED, !digitalRead(LED));
                    break;
              case CONN_STAT_WIFIMQTT_OK: // --___________--_________
               if(d<300)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              }
}

/////////////////////////////////////////////////////////// pomocnicze funkcje /////////////////////////////


bool isFloatString(String tString) {
  String tBuf;
  bool decPt = false;
 
  if(tString.charAt(0) == '+' || tString.charAt(0) == '-') tBuf = &tString[1];
  else tBuf = tString; 

  for(int x=0;x<tBuf.length();x++)
  {
    if(tBuf.charAt(x) == '.') {
      if(decPt) return false;
      else decPt = true; 
    }   
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
  }
  return true;
}


bool isFloatChars(char * ctab) {
  
  boolean decPt = false;
 uint8_t startInd=0;
  if(ctab[0] == '+' || ctab[0] == '-') startInd=1;

  for(uint8_t x=startInd;x<strlen(ctab);x++)
  {
    if(ctab[x] == '.')// ||ctab[x] == ',') 
    {
      if(decPt) return false;
      else decPt = true; 
    }   
    else if(!isDigit(ctab[x])) return false;
  }
  return true;
}
  
  bool isIntChars(char * ctab) {
  
  bool decPt = false;
  uint8_t startInd=0;
  if(ctab[0] == '+' || ctab[0] == '-') startInd=1;

  for(uint8_t x=startInd;x<strlen(ctab);x++)
  {
   if(!isDigit(ctab[x])) return false;
  }
  return true;
}
bool isNumber(char * tmp)
{
   int j=0;
   while(j<strlen(tmp))
  {
    if(tmp[j] > '9' || tmp[j] < '0')
    {
      return false;
    }     
    j++;
  }
 return true; 
}
