/*
 * InternalTemp.h
 */


#ifndef InternalTemp_h
#define InternalTemp_h

#include "Arduino.h"

class InternalTemp
{
  public:
    InternalTemp(uint8_t SensorPin, uint8_t MaxTemperature, float mVperC, float ZeroVoltage);
  
    float Temperature;

    float MeasureT(float Vcc);
    bool ThermalStatus(float Temperature);

  private:
    uint8_t _SensorPin;
	  uint8_t _MaxTemperature;
    float _mVperC;
    float _ZeroVoltage;

};


#endif
