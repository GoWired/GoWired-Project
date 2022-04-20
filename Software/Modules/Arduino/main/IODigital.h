/*
 * IODigital.h
 * 
 * SensorType:
 * 0 - INPUT_PULLUP sensor
 * 1 - INPUT sensor
 * 2 - Relay output
 * 3 - Button input
 * 4 - Button input + Relay output
 *
 */

#ifndef IODigital_h
#define IODigital_h

#include "Arduino.h"


class IODigital
{
  public:
    IODigital();       

    uint8_t SensorType;
    uint8_t NewState;
    uint8_t OldState;

    void SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0);
    void CheckInput();
    void SetRelay();
  
  private:
    uint8_t _RelayPin;
    uint8_t _SensorPin;
    bool _Invert;
    bool _HighStateDetected;
    bool _RelayOFF;

    bool _ReadDigital(uint8_t DebounceValue);
   
};

#endif
