/*
 * 
 */

#include "RShutterControl.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
RShutterControl::RShutterControl(int UpPin, int DownPin)  {

  _DownPin = DownPin;
  _UpPin = UpPin;
  int DownTime;
  int UpTime;
  
  pinMode(_UpPin, OUTPUT);
  pinMode(_DownPin, OUTPUT);

  EEPROM.get(EEA_RS_TIME_DOWN, DownTime);
  EEPROM.get(EEA_RS_TIME_UP, UpTime);
  
  if(UpTime > 30000 || DownTime > 30000)  {
    //Calibration();          // Calibration function not tested yet!
  }
  else  {
    _UpTime = UpTime;
    _DownTime = DownTime;
    EEPROM.get(EEA_RS_POSITION, Position);
  }
}

/*  *******************************************************************************************
 *                                        Calibration
 *  *******************************************************************************************/
void RShutterControl::Calibration()  {

  #ifdef MY_DEBUG
    Serial.println("Start Calibration");
  #endif

  // Move the roller shutter from unknown position upwards, stop when there is no current measured
  digitalWrite(_DownPin, RELAY_OFF);  
  digitalWrite(_UpPin, RELAY_ON);

  delay(100);
  while(PS.MeasureAC() > PS_OFFSET)  {     // Tu moze byc problem z okresleniem wartosci pradu
  }
  digitalWrite(_UpPin, RELAY_OFF);

  // Initialize variables
  int DownTimeCumulated = 0;
  int UpTimeCumulated = 0;
  unsigned long TIME_1 = 0;
  unsigned long TIME_2 = 0;
  unsigned long TIME_3 = 0;

  // Calibration: 1. Move down, 2. Move up, 3. Save measured values
  for(int i=0; i<CALIBRATION_SAMPLES; i++) {

    TIME_1 = millis();
    digitalWrite(_DownPin, RELAY_ON);
    delay(100);
    while(PS.MeasureAC() > PS_OFFSET) {
      TIME_2 = millis();
    }
    digitalWrite(_DownPin, RELAY_OFF);

    TIME_3 = TIME_2 - TIME_1;
    DownTimeCumulated += (int)(TIME_3 / 1000);

    delay(1000);

    TIME_1 = millis();
    digitalWrite(_UpPin, RELAY_ON);
    delay(100);
    while(PS.MeasureAC() > PS_OFFSET) {
      TIME_2 = millis();
    }
    digitalWrite(_UpPin, RELAY_OFF);

    TIME_3 = TIME_2 - TIME_1;
    UpTimeCumulated += (int)(TIME_3 / 1000);
  }

  Position = 0;

  _DownTime = (int)(DownTimeCumulated / CALIBRATION_SAMPLES);
  _UpTime = (int)(UpTimeCumulated / CALIBRATION_SAMPLES);

  EEPROM.put(EEA_RS_TIME_DOWN, _DownTime);
  EEPROM.put(EEA_RS_TIME_UP, _UpTime);
  EEPROM.put(EEA_RS_POSITION, Position);
}

int RShutterControl::Move(int Direction)  {
  
  int pin; int pin2; int Time;

  if(Direction == 1) {
    pin = _DownPin; pin2 = _UpPin; Time = _DownTime;
  }
  else if(Direction == 0)  {
    pin = _UpPin; pin2 = _DownPin; Time = _UpTime;
  }
  
  digitalWrite(pin2, RELAY_OFF);
  digitalWrite(pin, RELAY_ON);

  return Time;
}

void RShutterControl::Stop()  {

  digitalWrite(_DownPin, RELAY_OFF);
  digitalWrite(_UpPin, RELAY_OFF);
}
