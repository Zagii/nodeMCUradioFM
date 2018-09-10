#include "radioFM.h"



/// Update the ServiceName text on the LCD display.
void DisplayServiceName(char *name)
{
  Serial.print("RDS:"); 
  rdsLastName=name;
   Serial.println(name);
} // DisplayServiceName()

void receiveTextRDS(char *name)
{
  Serial.print("RDS text:");
  rdsLastText=name;
  Serial.println(rdsLastText);
}
void receiveTimeRDS(uint8_t h,uint8_t m)
{
  Serial.print("RDS time:");
  rdsLastTime=h+":"+m;
  Serial.println(rdsLastTime);
}
void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - -

/// Update the Frequency on the LCD display.
void CradioFM::DisplayFrequency(RADIO_FREQ f)
{
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("FREQ:"); Serial.println(s);
} // DisplayFrequency()





/// Execute a command identified by a character and an optional number.
/// See the "?" command for available commands.
/// \param cmd The command character.
/// \param value An optional parameter for the command.
void CradioFM::runSerialCommand(char cmd, int16_t value)
{
  if (cmd == '?') {
    Serial.println();
    Serial.println("? Help");
    Serial.println("+ increase volume");
    Serial.println("- decrease volume");
    Serial.println("> next preset");
    Serial.println("< previous preset");
    Serial.println(". scan up   : scan up to next sender");
    Serial.println(", scan down ; scan down to next sender");
    Serial.println("fnnnnn: direct frequency input");
    Serial.println("i station status");
    Serial.println("s mono/stereo mode");
    Serial.println("b bass boost");
    Serial.println("u mute/unmute");
  }

  // ----- control the volume and audio output -----
  
  else if (cmd == '+') {
    // increase volume
    int v = radio.getVolume();
    if (v < 15) radio.setVolume(++v);
  }
  else if (cmd == '-') {
    // decrease volume
    int v = radio.getVolume();
    if (v > 0) radio.setVolume(--v);
  }

  else if (cmd == 'u') {
    // toggle mute mode
    radio.setMute(! radio.getMute());
  }
  
  // toggle stereo mode
  else if (cmd == 's') { radio.setMono(! radio.getMono()); }

  // toggle bass boost
  else if (cmd == 'b') { radio.setBassBoost(! radio.getBassBoost()); }

  // ----- control the frequency -----
  
  else if (cmd == '>') {
    // next preset
    if (i_sidx < (sizeof(preset) / sizeof(RADIO_FREQ))-1) {
      i_sidx++; radio.setFrequency(preset[i_sidx]);
    } // if
  }
  else if (cmd == '<') {
    // previous preset
    if (i_sidx > 0) {
      i_sidx--;
      radio.setFrequency(preset[i_sidx]);
    } // if

  }
  else if (cmd == 'f') { radio.setFrequency(value); }

  else if (cmd == '.') { radio.seekUp(false); }
  else if (cmd == ':') { radio.seekUp(true); }
  else if (cmd == ',') { radio.seekDown(false); }
  else if (cmd == ';') { radio.seekDown(true); }


  // not in help:
  else if (cmd == '!') {
    if (value == 0) radio.term();
    if (value == 1) radio.init();

  }
  else if (cmd == 'i') {
    char s[12];
    radio.formatFrequency(s, sizeof(s));
    Serial.print("Station:"); Serial.println(s);
    Serial.print("Radio:"); radio.debugRadioInfo();
    Serial.print("Audio:"); radio.debugAudioInfo();

  } // info

  else if (cmd == 'x') { 
    radio.debugStatus(); // print chip specific data.
  }
} // runSerialCommand()


/// Setup a FM only radio configuration with I/O for commands and debugging on the Serial port.
void CradioFM::begin() {
  // Initialize the Radio 
  radio.init();

  // Enable information to the Serial port
  radio.debugEnable();

  radio.setBandFrequency(RADIO_BAND_FM, preset[i_sidx]); // 5. preset.

  // delay(100);

  radio.setMono(true);
  radio.setMute(false);
  // radio.debugRegisters();
  radio.setVolume(8);

  Serial.write('>');
  
  state = STATE_PARSECOMMAND;
  
  // setup the information chain for RDS data.
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);
  rds.attachTextCallback(receiveTextRDS);
  rds.attachTimeCallback(receiveTimeRDS);
  
  runSerialCommand('?', 0);
} // Setup

void CradioFM::checkSerial()
{
  char c;
  if (Serial.available() > 0) {
    // read the next char from input.
    c = Serial.peek();

    if ((state == STATE_PARSECOMMAND) && (c < 0x20)) {
      // ignore unprintable chars
      Serial.read();

    }
    else if (state == STATE_PARSECOMMAND) {
      // read a command.
      command = Serial.read();
      state = STATE_PARSEINT;

    }
    else if (state == STATE_PARSEINT) {
      if ((c >= '0') && (c <= '9')) {
        // build up the value.
        c = Serial.read();
        value = (value * 10) + (c - '0');
      }
      else {
        // not a value -> execute
        runSerialCommand(command, value);
        command = ' ';
        state = STATE_PARSECOMMAND;
        value = 0;
      } // if
    } // if
  } // if

}

CradioFM::printStatus()
{
   unsigned long now = millis();
  // update the display from time to time
  if (now > nextFreqTime) {
    f = radio.getFrequency();
    if (f != lastf) {
      // print current tuned frequency
      DisplayFrequency(f);
      lastf = f;
    } // if
    nextFreqTime = now + 400;
  } // if  

}
void CRadio::setStatusStr()
{
 int v = radio.getVolume();
 RADIO_FREQ ff = radio.getFrequency();
    statusStr="{'freq':"+ff+", 'vol':"+v+",'rdsName':'"+ rdsLastName+
                "','rdsText':'"+rdsLastText+
                "','rdsTime':'"+rdsLastTime+"'}";
}
void CradioFM::testRDS()
{
  bool ret=false;
  radio.checkRDS(); // check for RDS data
  if(byloSN)
  {
    ret=true;
    byloSN=false;
  }
  if(byloText)
  {
    ret=true;
    byloText=false;
  }
  if(byloTime)
  {
    ret=true;
    byloTime=false;
  }
  if(ret)
  {
    setStatusStr();
  }
  return ret;
}
/// Constantly check for serial input commands and trigger command execution.
bool CradioFM::loop() 
{
  checkSerial(); //sprawdza komende z Seriala
  bool test=testRDS();
   
  printStatus();  
  return test;
} // loop

// End.
