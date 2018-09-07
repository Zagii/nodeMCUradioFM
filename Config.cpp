#include "Config.h"

String  CConfig::loadJsonStr(const char* nazwaPliku)
{
   DPRINT("CConfig::loadJonStr ");DPRINTLN(nazwaPliku);
  String s="";
  File configFile = SPIFFS.open(nazwaPliku, "r");
  if (!configFile) {
     DPRINT("Blad odczytu pliku ");DPRINTLN(nazwaPliku);
   return s;
  }

//  size_t size = configFile.size();
 // if (size > 2048) {
//    DPRINT("Za duży plik ");DPRINTLN(nazwaPliku);
//    return s;
//  }

  // Allocate a buffer to store contents of the file.
 // std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  //configFile.readBytes(buf.get(), size);
  //yield();
  
  s=configFile.readString();
  configFile.close();
  DPRINTLN(s.c_str());
  return s;
  
}

bool CConfig::loadConfigSekcjeLBL()
{
  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + 10*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 80;
   DynamicJsonBuffer jsonBuffer;//(bufferSize);
  // DynamicJsonBuffer* jB=*jsonBuffer;
  String s=loadJsonStr(PLIK_LBL);
  DPRINTLN(s);
   JsonObject& js= jsonBuffer.parse(s);//parseObject(s);
   js.prettyPrintTo(Serial);
    
   if(js.containsKey("LBL"))
   {
    JsonArray& ar = js["LBL"];
    for (auto& j : ar) {
       int id = j["id"];
       char l[20];
       strcpy(l, j["lbl"]);
       setSekcjaLbl(id,l);
    }
    return true;
   }else
   {
    return false;
    }

}

void CConfig::begin()
{
  DPRINTLN("Mounting FS...");
  if (!SPIFFS.begin()) {
   DPRINTLN("Failed to mount file system");
    return;
  }
/*  if (!saveConfig()) {
   DPRINTLN("Failed to save config");
  } else {
   DPRINTLN("Config saved");
  }*/

  
  if (!loadConfig()) {
    DPRINTLN("Failed to load config");
  } else {
    DPRINTLN("Config loaded");
  }  
  if(!loadConfigSekcjeLBL()) {
    DPRINTLN("Failed to load config SekcjeLBL");
  } else {
    DPRINTLN("Config SekcjeLBL loaded");
  }  
}


bool CConfig::loadProgs() 
{
  DynamicJsonBuffer jsonBuffer;//(bufferSize);
  String s=loadJsonStr(PLIK_PROG);
  DPRINTLN(s);
   JsonObject& js= jsonBuffer.parse(s);//parseObject(s);
   js.printTo(Serial);
   if(!js.success())return false;
    
   if(js.containsKey("PROG"))
   {
    JsonArray& ar = js["PROG"];
    Program a;
    for (auto& json : ar)
    {
       setProg(a,json["dzienTyg"],json["tStr"],json["ms"],json["okresS"],json["coIle"],json["sekcja"],json["aktywny"]);
       addProg(a);
    }
    return true;
   }else
   {
    return false;
    }

}

bool CConfig::loadConfig() {

return loadProgs();
  ////////////////////////////////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< usunac
  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "r");
  if (!configFile) {
     DPRINT("Blad odczytu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
   return false;
  }

  size_t size = configFile.size();
  if (size > 2048) {
    DPRINT("Za duży plik ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  yield();
  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + 10*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 80;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(buf.get());

  if (!json.success()) {
    DPRINT("Blad parsowania json ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }
  
  uint8_t n = json["n"]; // progIle
  for(uint8_t i=0;i<n;i++)
  {
    yield();
    JsonArray& prog = json["Programy"][i];
    Program pp;
    setProg(pp,json["dzienTyg"],json["tStr"],json["ms"],json["okresS"],json["coIle"],json["sekcja"],json["aktywny"]);
   // setProg(pp,prog[0],prog[1],prog[2], prog[3], prog[4],prog[5],prog[6]);
    addProg(pp);
  }
  DPRINT("progIle=");DPRINTLN(n);
  return true;
}

bool CConfig::saveConfigStr(const char *nazwaPliku,const char * str) {

  DPRINT(" saveConfigJSON: ");DPRINT(str);DPRINT(" / ");DPRINTLN(nazwaPliku);

  File configFile = SPIFFS.open(nazwaPliku, "w");
  if (!configFile) {
    DPRINT("Blad zapisu pliku ");DPRINTLN(nazwaPliku);
    return false;
  }
  configFile.println(str);
  configFile.close();
  DPRINTLN("save [OK]");
  return true;
}

bool CConfig::saveProgs() {

  DPRINT(" saveProgs");
 // const size_t bufferSize = JSON_ARRAY_SIZE(2) + progIle*JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(1);
  //DPRINTLN(bufferSize);
  /*DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
    root["n"] = progIle;
  JsonArray& Programy = root.createNestedArray("Programy");
  for(uint8_t i=0;i<progIle;i++)
  {
    yield();
    JsonArray& pr = Programy.createNestedArray();
    pr.add(prTab[i].dzienTyg);
    pr.add(prTab[i].dataOdKiedy);
    pr.add(prTab[i].godzinaStartu);
    pr.add(prTab[i].czas_trwania_s);
    pr.add(prTab[i].sekcja);
    pr.add(prTab[i].co_ile_dni);
    pr.add(prTab[i].aktywny);
  }*/
  String str="{\"PROG\":[";
  for(uint8_t i=0;i<progIle;i++)
  {
    yield();
    str+=publishProgJsonStr(prTab[i],i);
    if(i<progIle-1)str+=",";
   }
   str+="]}";

  saveConfigStr(PLIK_PROG,str.c_str());

/*
  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "w");
  if (!configFile) {
    DPRINT("Blad zapisu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }
  root.printTo(Serial);
  root.printTo(configFile);
  configFile.close();*/
  return true;
}



void CConfig::setSekcjaLbl(uint8_t id,String lbl)
{
  DPRINT("SEKCJA id=");DPRINT(id);DPRINT("lbl=");DPRINTLN(lbl);
  sekcjeLbl[id]=lbl;
}
String CConfig::getSekcjaLbl(uint8_t id)
{
  return sekcjeLbl[id];
}


//----------------------------------------------------
void CConfig::setProg(Program &a,uint8_t dzien, uint8_t mies, uint16_t rok,  uint8_t h, uint8_t m,  uint8_t s,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekcja, bool aktywny)
{
  DPRINT("setProg #->");
  tmElements_t t;
/*  t.Year = CalendarYrToTm(rok);
  t.Month = mies;  t.Day = dzien;
  t.Hour = 0;  t.Minute = 0;  t.Second = 0;
  a.dataOdKiedy=makeTime(t);
*/
  a.dzienTyg=dzien;
  a.tStr=String(h)+":"+String(m)+":"+String(s);
  t.Year = CalendarYrToTm(1970);
  t.Month = 1;  t.Day = 1;
  t.Hour = h;  t.Minute = m;  t.Second = s;
  a.godzinaStartu=makeTime(t);
  
  a.czas_trwania_s=czas_trwania_s; 
  a.sekcja=sekcja; 
  a.co_ile_dni=co_ile_dni;
  a.aktywny=aktywny;
  publishProg(a,progIle);
}

void CConfig::setProg(Program &a,uint8_t dzien,String timeStr,time_t godzina,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekcja,bool aktywny)
{ 
  DPRINT("setProg time_t #->");
  a.dzienTyg=dzien;
 // a.dataOdKiedy=data;
  a.godzinaStartu=godzina;
  a.tStr=timeStr;
  a.czas_trwania_s=czas_trwania_s; 
  a.sekcja=sekcja; 
  a.co_ile_dni=co_ile_dni;
  a.aktywny=aktywny;
  publishProg(a,progIle);
  
}
void CConfig::setProg(Program &a, Program &b)
{
  DPRINT("addProg ref #");
  publishProg(b,progIle);
//  tmElements_t t;
 // breakTime(b.dataOdKiedy, t); 
 // a.dataOdKiedy=makeTime(t);
  a.tStr=b.tStr;
  a.dzienTyg=b.dzienTyg;
 // a.dataOdKiedy=b.dataOdKiedy;
  a.godzinaStartu=b.godzinaStartu;
  a.co_ile_dni=b.co_ile_dni;
  a.czas_trwania_s=b.czas_trwania_s;
  a.sekcja=b.sekcja; 
  a.aktywny=b.aktywny;
}

void CConfig::getProg(Program &a, uint16_t progRefID)
{
  setProg(a,prTab[progRefID]);
}

void CConfig::changeProg(Program a, uint16_t progRefID)
{
  DPRINT("changeProg #");
  publishProg(a,progIle);
  if(progRefID >=progIle)return;
  setProg(prTab[progRefID],a);
}
void CConfig::addProg(Program p)
{
  DPRINT("addProg #");
  publishProg(p,progIle);
  if(progIle+1>=MAX_PROGR)return;

  setProg(prTab[progIle],p);
  progIle++;
}



void CConfig::publishProg(Program &p,uint16_t i)
{
  DPRINT("ID="); DPRINT(i); DPRINT("; ");
  DPRINT(" dzienTyg=");DPRINT(p.dzienTyg);
  DPRINT(" tStr"); DPRINT(p.tStr.c_str());
  DPRINT("; ms="); DPRINT(hour(p.godzinaStartu)); DPRINT(":");DPRINT(minute(p.godzinaStartu)); DPRINT(":");DPRINT(second(p.godzinaStartu)); 
  DPRINT(" czas_trwania_s="); DPRINT(p.czas_trwania_s); DPRINT("; ");DPRINT(" co_ile_dni="); DPRINT(p.co_ile_dni); DPRINT("; ");
  DPRINT(" sekcja="); DPRINT(p.sekcja); DPRINT(" aktywny=");DPRINTLN(p.aktywny);
  
}
String CConfig::publishProgJsonStr(Program &p,uint16_t i)
{
  DPRINT("publishProgJsonStr: ");
  //{"PROG":{id:x, dzienTyg=1, tStr="00:33:22", ms="miliis", "okresS":s, "sekcja": n, "coIle":z, "aktywny":b }}
  String w="{\"id\":"+String(i)+",\"dzienTyg\":"+p.dzienTyg+",\"tStr\":\""+p.tStr+"\",\"ms\":"+String(p.godzinaStartu)+",\"okresS\":"+String(p.czas_trwania_s)+",\"coIle\":"+String(p.co_ile_dni)+",\"sekcja\":"+String(p.sekcja)+",\"aktywny\":"+String(p.aktywny?1:0)+"}";
  DPRINTLN(w.c_str());
  return w;
}
String CConfig::publishTabProgJsonStr(uint16_t i)
{
  return publishProgJsonStr(prTab[i],i);
}
void CConfig::printCzas(time_t t)
{
  DPRINT("czas* time_t= ");DPRINT(t); 
  DPRINT("; data="); DPRINT(day(t)); DPRINT("-");DPRINT(month(t)); DPRINT("-");DPRINT(year(t)); 
  DPRINT("; godz="); DPRINT(hour(t)); DPRINT(":");DPRINT(minute(t)); DPRINT(":");DPRINT(second(t)); 
}

void CConfig::delProg(uint16_t id)
{
  DPRINT("delProg #"); DPRINT(id);DPRINT(" / ");DPRINTLN(progIle);
  if(id>progIle|| progIle==0)return;
  publishProg(prTab[id],id);
  if(id==progIle){progIle--;return;};
  for(uint16_t i=id;i<progIle;i++)
  {
    yield();
    setProg(prTab[i],prTab[i+1]);
  }
  progIle--;
}

void CConfig::publishAllProg()
{
  for(uint16_t i=0;i<progIle;i++)
  {
    yield();
   publishProg(prTab[i],i);
  }
  
}

bool CConfig::checkRangeProg(Program &p,time_t sysczas_t)
{

uint8_t sysDzienTyg=weekday(sysczas_t);
if((sysDzienTyg-p.dzienTyg)%p.co_ile_dni==0) // test co ile Dni
{
   time_t aktualnaGodzina=sysczas_t % SEK_W_DNIU;
   p.godzinaStartu=p.godzinaStartu % SEK_W_DNIU;
  
  if((aktualnaGodzina >=p.godzinaStartu) && (p.godzinaStartu+p.czas_trwania_s>=aktualnaGodzina))
  {
 //   DPRINTLN(" w zakresie ]");
    return true;
  }
  else
  { 
  //  DPRINTLN(" poza zakresem ]");
    return false;
  }
   
  /*if(hour(p.godzinaStartu)<hour(sysczas_t) && hour(sysczas_t)<hour(godzKonca))
  {
    if(minute(p.godzinaStartu)<minute(sysczas_t) && minute(sysczas_t)<minute(godzKonca))
    {
      if(second(p.godzinaStartu)<second(sysczas_t) && second(sysczas_t)<second(godzKonca))
    }   
  }*/
}

return false;
  
 // DPRINT("checkRangeProg start, sysczas_s ");  printCzas(sysczas_t);DPRINTLN("");
 // publishProg(p);
  if(!p.aktywny) return false;
  
 /* if(p.dataOdKiedy>sysczas_t)
  {
    DPRINTLN(" Program z przyszlosci");
    return false;
  }*/
 /* time_t  czasSysOdKiedy = sysczas_t - p.dataOdKiedy;
  uint8_t deltaDni=czasSysOdKiedy/SEK_W_DNIU;
  if(deltaDni%p.co_ile_dni!=0)
  {
    // DPRINT(" Program nie z tego dnia. delta=");DPRINTLN(deltaDni);
    return false;
  }
   time_t aktualnaGodzina=sysczas_t % SEK_W_DNIU;

// DPRINT(" poczatkowaGodzina ");printCzas(p.godzinaStartu);DPRINT(" / ");
// DPRINT(" aktualnaGodzina ");printCzas(aktualnaGodzina);DPRINT(" / ");
// DPRINT(" koncowaGodzina ");printCzas(p.godzinaStartu+p.czas_trwania_s);DPRINT(" ");
 //DPRINT(" okno [ ");
 
  if((aktualnaGodzina >=p.godzinaStartu) && (p.godzinaStartu+p.czas_trwania_s>=aktualnaGodzina))
  {
 //   DPRINTLN(" w zakresie ]");
    return true;
  }
  else
  { 
  //  DPRINTLN(" poza zakresem ]");
    return false;
  }*/
}
uint8_t  CConfig::wlaczoneSekcje(time_t sysczas_s)
{
  uint8_t stan=0;
  for(uint16_t i=0;i<progIle;i++)
  {
    yield();
   // DPRINT("test programu:");DPRINTLN(i);
      if(checkRangeProg(prTab[i], sysczas_s))
      {
        bitSet(stan,prTab[i].sekcja);
        //stan |=1<<prTab[i].sekcja;
      }
  //  DPRINTLN(" koniec.");
  }
  return stan;
}

