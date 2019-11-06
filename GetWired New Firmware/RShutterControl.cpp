/*
 * 
 */

#include "RShutterControl.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
RShutterControl::RShutterControl(uint8_t UpPin, uint8_t DownPin, bool RelayOn, bool RelayOff)  {

  _DownPin = DownPin;
  _UpPin = UpPin;
  _RelayOn = RelayOn;
  _RelayOff = RelayOff;
  
  pinMode(_UpPin, OUTPUT);  digitalWrite(_UpPin, RelayOff);
  pinMode(_DownPin, OUTPUT);  digitalWrite(_DownPin, RelayOff);

  uint8_t DownTime;
  uint8_t UpTime;

  EEPROM.get(EEA_RS_TIME_DOWN, DownTime);
  EEPROM.get(EEA_RS_TIME_UP, UpTime);

  if(UpTime != 255 && DownTime != 255)  {
    Calibrated = true;
    _UpTime = UpTime;
    _DownTime = DownTime;
  }
  else  {
    Calibrated = false;
  }

}

/*  *******************************************************************************************
 *                                        Auto Calibration
 *  *******************************************************************************************/
void RShutterControl::Calibration(uint8_t CalibrationSamples, float PSOffset, bool Calibrated, uint8_t UpTime=0, uint8_t DownTime=0)  {

  if(UpTime != 0 && DownTime != 0)  {
    _UpTime = UpTime;
    _DownTime = DownTime;
  }
  else if(CalibrationSamples != 0 && Calibrated != true)  {

    int DownTimeCumulated = 0;
    int UpTimeCumulated = 0;
    unsigned long TIME_1 = 0;
    unsigned long TIME_2 = 0;
    unsigned long TIME_3 = 0;
    
    digitalWrite(_DownPin, _RelayOff);
    digitalWrite(_UpPin, _RelayOn);

    delay(100);

    while(PS.MeasureAC() > PSOffset)  {
      delay(100);
    }
    digitalWrite(_UpPin, _RelayOff);

    for(int i=0; i<CalibrationSamples; i++) {
      TIME_1 = millis();
      digitalWrite(_DownPin, _RelayOn);
      delay(100);

      while(PS.MeasureAC() > PS_OFFSET) {
        TIME_2 = millis();
      }
      digitalWrite(_DownPin, _RelayOff);

      TIME_3 = TIME_2 - TIME_1;
      DownTimeCumulated += (int)(TIME_3 / 1000);

      delay(1000);

      TIME_1 = millis();
      digitalWrite(_UpPin, _RelayOn);
      delay(100);
      
      while(PS.MeasureAC() > PSOffset) {
        TIME_2 = millis();
      }
      digitalWrite(_UpPin, _RelayOff);

      TIME_3 = TIME_2 - TIME_1;
      UpTimeCumulated += (int)(TIME_3 / 1000);
    }

    Position = 0;

    _DownTime = (int)(DownTimeCumulated / CalibrationSamples);
    _UpTime = (int)(UpTimeCumulated / CalibrationSamples);

    EEPROM.put(EEA_RS_TIME_DOWN, _DownTime);
    EEPROM.put(EEA_RS_TIME_UP, _UpTime);
    EEPROM.put(EEA_RS_POSITION, Position);
  }
}

int RShutterControl::Move(int Direction)  {
  
  int pin; int pin2; int Time;

  if(Direction == 1) {
    pin = _DownPin; pin2 = _UpPin; Time = _DownTime;
  }
  else if(Direction == 0)  {
    pin = _UpPin; pin2 = _DownPin; Time = _UpTime;
  }
  
  digitalWrite(pin2, _RelayOff);
  digitalWrite(pin, _RelayOn);

  return Time;
}

void RShutterControl::Stop()  {

  digitalWrite(_DownPin, _RelayOff);
  digitalWrite(_UpPin, _RelayOff);
}