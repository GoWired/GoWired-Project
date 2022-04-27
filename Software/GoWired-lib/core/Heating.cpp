/**
 * @file Heating.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Heating.h"

/**
 * @brief Construct a new Heating:: Heating object
 * 
 */
Heating::Heating()  {

}

/**
 * @brief Pass parameters to new Heating object
 * 
 * @param T_ID ID of a thermometer
 * @param DefaultSetPoint default set point value
 * @param EEPROM_ADDRESS EEPROM address for storing temperature set point
 * @param RelayOff bool value to turn the relay off
 * @return uint16_t EEPROM_ADDRESS for next Heating object
 */
uint16_t Heating::SetSectionValues(uint8_t T_ID, uint8_t DefaultSetPoint, uint16_t EEPROM_ADDRESS, bool RelayOff) {

  _RelayOn = !RelayOff;
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

/**
 * @brief Set new set point value
 * 
 * @param Temperature 
 */
void Heating::SetTemperature(float Temperature)  {

  SetPointDay = Temperature;
  EEPROM.put(_EEPROM_ADDRESS, SetPointDay);

}

/**
 * @brief Gets thermometer ID which is private
 * 
 * @return uint8_t thermometer ID
 */
uint8_t Heating::GetTID() {

  return _T_ID;
}

/**
 * @brief Saves new temperature value obtained from sensor
 * 
 * @param Temperature 
 */
void Heating::ReadTemperature(float Temperature) {
  
  _TemperatureSensor = Temperature;
}

/**
 * @brief Compares temperature obtained from sensor with set point
 * 
 * @param SetPoint set point temperature
 * @param Hysteresis hysteresis value
 * @return new relay state true or false
 */
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
