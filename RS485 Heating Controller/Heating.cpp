/*
 * 
 */

#include "Heating.h"

/*  *******************************************************************************************
 *                                        Constructor
 *  *******************************************************************************************/
Heating::Heating()  {

}

/*  *******************************************************************************************
 *                                   Setting Section Values
 *  *******************************************************************************************/
uint16_t Heating::SetSectionValues(uint8_t T_ID, uint8_t DefaultSetPoint, uint16_t EEPROM_ADDRESS, bool RelayOn, bool RelayOff) {

  float TemperatureSet;

  _RelayOn = RelayOn;
  _RelayOff = RelayOff;
  _T_ID = T_ID;
  _TemperatureSensor = -100;

  _EEPROM_ADDRESS = EEPROM_ADDRESS;
  EEPROM.get(_EEPROM_ADDRESS, TemperatureSet);
  if(TemperatureSet < 100)  {
    _TemperatureSet = TemperatureSet;
  }
  else  {
    _TemperatureSet = DefaultSetPoint;
  }
  EEPROM_ADDRESS += sizeof(float);

  return EEPROM_ADDRESS;
}

/*  *******************************************************************************************
 *                                    Changing Relay State
 *  *******************************************************************************************/
/*void Heating::SetRelay(bool NewState) {
  // Debug info
  //Serial.print("Relay pin: ");  Serial.println(_RelayPin);
  //Serial.print("New State: ");  Serial.println(NewState);
  digitalWrite(_RelayPin, NewState);
  RelayState = NewState;
}

/*  *******************************************************************************************
 *                               Saving Temperature from Controller
 *  *******************************************************************************************/
void Heating::SetTemperature(float Temperature)  {

  _TemperatureSet = Temperature;
  EEPROM.put(_EEPROM_ADDRESS, _TemperatureSet);

}

/*  *******************************************************************************************
 *                                  Get SetPoint Temperature
 *  *******************************************************************************************/
float Heating::GetSetTemp() {
  
  return _TemperatureSet;
}

/*  *******************************************************************************************
 *                                  Get T_ID of this Section
 *  *******************************************************************************************/
uint8_t Heating::GetTID() {

  return _T_ID;
}

/*  *******************************************************************************************
 *                             Saving Temperature from Remote Sensor
 *  *******************************************************************************************/
void Heating::ReadTemperature(float Temperature) {
  
  _TemperatureSensor = Temperature;
}

/*  *******************************************************************************************
 *                                 Comparing Temperature Values
 *  *******************************************************************************************/
bool Heating::TemperatureCompare(float TemperatureSet, float Hysteresis) {

  if(_TemperatureSensor != -100)  {
    if(_TemperatureSensor < TemperatureSet) {
      return _RelayOn;
    }
    else if(_TemperatureSensor >= TemperatureSet+Hysteresis)  {
      return _RelayOff;
    }
  }
  else  {
    return _RelayOff;
  }
}
/*
 * 
 * EOF
 * 
 */