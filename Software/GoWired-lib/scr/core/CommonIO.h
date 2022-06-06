/**
 * @file CommonIO.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief Class reads digital & analog (touch buttons) inputs and sets digital outputs (relays)
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CommonIO_h
#define CommonIO_h

#include "Arduino.h"
#include "ADCTouch.h"

class CommonIO
{
  public:
    CommonIO();

    uint8_t SensorType;
    uint8_t NewState;
    uint8_t State;

    uint8_t DebugValue;
    int TouchDiagnosisValue;

    void SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0, uint8_t Pin3=0);
    void ReadReference();
    void SetState(uint8_t ReceivedState);
    void CheckInput(uint16_t LongpressDuration, uint8_t DebounceValue);
    void CheckInput2(uint8_t Threshold, uint16_t LongpressDuration, uint8_t DebounceValue);
    void CheckInput3(uint8_t Threshold, uint8_t DebounceValue, bool Monostable);
    void SetRelay();
  
  private:
    int _TouchReference;
    uint8_t _RelayPin;
    uint8_t _SensorPin;
    uint8_t _SensorPin2;
    bool _Invert;
    bool _HighStateDetected;
    bool _RelayOFF;

    bool _ReadAnalog(uint8_t Threshold);
    bool _ReadDigital(uint8_t DebounceValue);
   
};

#endif