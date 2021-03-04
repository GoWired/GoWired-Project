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

  _RelayOn = RelayOn;
  _RelayOff = RelayOff;
  _T_ID = T_ID;
  _TemperatureSensor = -100;
  RelayState = 0;
  _EEPROM_ADDRESS = EEPROM_ADDRESS;
  
  EEPROM.get(EEPROM_ADDRESS, SetPointDay);

  SetPointDay = SetPointDay < 100 ? SetPointDay : DefaultSetPoint;
  
  EEPROM_ADDRESS += sizeof(float);

  return EEPROM_ADDRESS;
}

/*  *******************************************************************************************
 *                               Saving Temperature from Controller
 *  *******************************************************************************************/
void Heating::SetTemperature(float Temperature)  {

  SetPointDay = Temperature;
  EEPROM.put(_EEPROM_ADDRESS, SetPointDay);

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
bool Heating::TemperatureCompare(float SetPoint, float Hysteresis) {

  if(_TemperatureSensor != -100)  {
    if(_TemperatureSensor < SetPoint) {
      return _RelayOn;
    }
    else if(_TemperatureSensor >= SetPoint+Hysteresis)  {
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