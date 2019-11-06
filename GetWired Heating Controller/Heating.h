/*
 * 
 * EEPROM storing order: thermostat (float), remote thermometer id (int)
 * 
 */

#ifndef Heating_h
#define Heating_h

#include "Arduino.h"
#include <EEPROM.h>

class Heating
{
  public:
    Heating();
    
    bool RelayState;                               // Current relay state

    //void SetRelay(bool NewState);
    uint16_t SetSectionValues(uint8_t T_ID, uint8_t DefaultSetPoint, uint16_t EEPROM_ADDRESS, bool RelayOn, bool RelayOff);
    void SetTemperature(float Temperature);
    void ReadTemperature(float Temperature);
    bool TemperatureCompare(float TemperatureSet, float Hysteresis);
    float GetSetTemp();
    uint8_t GetTID();

  private:
    uint16_t _EEPROM_ADDRESS;                      // EEPROM address of first value stored in eeprom by the section
    uint8_t _T_ID;                                 // Remote temperature sensor ID
    float _TemperatureSet;                         // Temperature set by controller for daytime
    float _TemperatureSensor;                      // Temperature send by remote nodes
    bool _RelayOn;
    bool _RelayOff;

};




#endif