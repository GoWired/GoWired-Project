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
    int TouchDiagnosisValue;

    uint8_t ReadNewState();
    uint8_t ReadState();
    void SetValues(bool RelayOFF, bool RelayOn, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0, uint8_t Pin3=0);
    void ReadInput(uint16_t Threshold, uint8_t DebounceValue, bool Monostable);
    void SetRelay();
    void SetState(uint8_t NewState);

  private:
    uint16_t _TouchReference;
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