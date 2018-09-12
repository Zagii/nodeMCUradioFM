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

#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "Defy.h"
#include "CWifi.h"
#include "Config.h"
#include "CWebSerwer.h"
#include "radioFM.h"

CWifi wifi;
PubSubClient *mqtt;

CConfig conf;

CWebSerwer web;

char tmpTopic[MAX_TOPIC_LENGHT];
char tmpMsg[MAX_MSG_LENGHT];

////////////// sprawdzic ntp
////////// https://github.com/arduino-libraries/NTPClient

CradioFM radioFM;


unsigned long czasLokalnyMillis=0;
unsigned long czasLokalny=0;

unsigned long sLEDmillis=0;


bool czekaNaPublikacjeStanuMQTT=false;
bool czekaNaPublikacjeStanuWS=false;

uint8_t publicID=0;
unsigned long Millis=0;


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
 
  //Wire.pins(PIN_SDA, PIN_SCL);//SDA - D1, SCL - D2
  //Wire.begin();

  radioFM.begin();
  //

//////////////// wifi i mqtt init ///////////
  wifi.begin();
  mqtt=wifi.getMQTTClient();
  mqtt->setCallback(callback);
///////////// koniec wifi i mqtt init /////////

conf.begin();
conf.setTryb(TRYB_AUTO);

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

void parsujRozkaz(char *topic, char *msg)
{
  DPRINT("parsujRozkaz topic=");DPRINT(topic);DPRINT(", msg=");DPRINTLN(msg);
   char *ind=NULL;
   
    //////////////////////////////////////
    ind=strstr(topic,"CMD");
    if(ind!=NULL)
    {
      char c=msg[0];
       if(c=='+'||c=='-'||c=='>'||c=='<'||c=='.'||c==','||c=='f'||c=='i'||c=='s'||c=='b'||c=='u')
       {
         uint16_t value=0;
         uint8_t ic=1;
         char v=msg[ic];
         while(v!='\0'&&ic<6) //10110
         {
            value = (value * 10) + (v - '0');
            ic++;
            v=msg[ic];
         }
         radioFM.runSerialCommand(c,value);
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
    
 }

unsigned long d=0;

String millisTimeStr;
void loop()
{

  
   if(millis()-czasLokalnyMillis>1000)
  {
    czasLokalnyMillis=millis();
    czasLokalny++;
    millisTimeStr=String(wifi.TimeToString(millis()/1000));
  }
  if(radioFM.loop())
  {
     String infoStr="";
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
    root["STATUS"]=radioFM.getStatusStr();
    root.printTo(infoStr); 
    char tmpTopic[MAX_TOPIC_LENGHT];
    sprintf(tmpTopic,"%s/INFO/",wifi.getOutTopic());
    
    wifi.RSpisz(String(tmpTopic),infoStr,true);
    String js=String("{\"INFO\":")+infoStr+"}";
    DPRINTLN(js);
    web.sendWebSocket(js.c_str());
  ////////////
  }
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
    
   }
   /////////////////// obsluga hardware //////////////////////
   
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
