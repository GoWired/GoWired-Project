/*
 * For use with MCP9700A analog temperature sensor
 * mVperC = 10
 * ZeroVoltage = 0.5
 * 
 */


#ifndef InternalTemp_h
#define InternalTemp_h

#include "Arduino.h"

class InternalTemp
{
  public:
    InternalTemp(uint8_t SensorPin, uint8_t MaxTemperature, float mVperC, float ZeroVoltage, float MCUVoltage);
  
    float Temperature;

    float MeasureT();
    bool ThermalStatus(float Temperature);

  private:
    uint8_t _SensorPin;
	  uint8_t _MaxTemperature;
    float _mVperC;
    float _ZeroVoltage;
	  float _MCUVoltage;
};


#endif