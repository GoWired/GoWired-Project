/*
 * This is a code written for easy management of digital input/output sensors such as door/window/pir security sensors,
 * buttons and relays. It menages easy sensor initialization, holds sensor states, changes relays states and provides a quite smart 
 * function for reading inputs (including special button functionality which change its state after one second of holding a button).
 * 
 * 
 * SensorType:
 * 0 - INPUT_PULLUP sensor
 * 1 - INPUT sensor 
 * 2 - Relay output
 * 3 - Button input
 * 4 - Button input + Relay output
 *
 */

#ifndef UniversalInput_h
#define UniversalInput_h

#include "Arduino.h"


class UniversalInput
{
  public:
    UniversalInput();       

    uint8_t SensorType;
    uint8_t NewState;
    uint8_t OldState;

    void SetValues(bool RelayOFF, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0);
    void CheckInput();
    void SetRelay();
  
  private:
    uint8_t _RelayPin;
    uint8_t _SensorPin;
    bool _LowStateDetection;
    bool _HighStateDetection;
    bool _Condition;
    bool _RelayOFF;
   
};

#endif
