/*
 * RShutterControl.cpp
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
    EEPROM.get(EEA_RS_POSITION, Position);
  }
  else  {
    Calibrated = false;
  }

}

/*  *******************************************************************************************
 *                                        Auto Calibration
 *  *******************************************************************************************/
void RShutterControl::Calibration(uint8_t UpTime, uint8_t DownTime)  {
  
  Position = 0;

  _UpTime = UpTime;
  _DownTime = DownTime;

  EEPROM.put(EEA_RS_TIME_DOWN, _DownTime);
  EEPROM.put(EEA_RS_TIME_UP, _UpTime);
  EEPROM.put(EEA_RS_POSITION, Position);

  Calibrated = true;
  
}

/*  *******************************************************************************************
 *                                        Movement
 *  *******************************************************************************************/
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