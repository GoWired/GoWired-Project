/*
 * InOut.h
 */

#ifndef InOut_h
#define InOut_h

#include "Arduino.h"
#include "ADCTouch.h"

class InOut
{
  public:
    InOut();

    uint8_t SensorType;
    uint8_t DebugValue;
    int TouchDiagnosisValue;

    void SetValues(bool RelayOFF, bool RelayOn, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0, uint8_t Pin3=0);
    void ReadReference();
    void SetState(uint8_t NewState);
    void SetRelay();
    uint8_t ReadNewState();
    uint8_t ReadState();
    void ReadInput(uint8_t Threshold, uint16_t LongpressDuration, uint8_t DebounceValue);

  private:
    int _TouchReference;
    uint8_t _RelayPin;
    uint8_t _SensorPin;
    uint8_t _SensorPin2;
    bool _RelayOFF;
    bool _RelayON;
    uint8_t _NewState;
    uint8_t _State;

    bool _ReadAnalog(uint8_t Threshold);
    bool _ReadDigital(uint8_t DebounceValue);
};



#endif