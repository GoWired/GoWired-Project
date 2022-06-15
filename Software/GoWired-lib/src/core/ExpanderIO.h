/**
 * @file ExpanderIO.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief Class reads digital inputs and sets digital outputs (relays); works with PCF8575
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ExpanderIO_h
#define ExpanderIO_h

#include "Arduino.h"
#include <Wire.h>
#include <PCF8575.h>

class ExpanderIO
{
  public:
    ExpanderIO();       

    uint8_t SensorType;
    uint8_t NewState;
    uint8_t State;
    
    void ExpanderInit(uint8_t Address=0x20);
    void SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0);
    void CheckInput();
    void SetRelay();
    bool _ReadDigital(uint8_t DebounceValue);
  
  private:
    uint8_t _RelayPin;
    uint8_t _SensorPin;
    bool _HighStateDetected;
    bool _Invert;
    bool _RelayOFF;    
   
};

#endif