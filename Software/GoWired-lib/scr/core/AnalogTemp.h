/**
 * @file AnalogTemp.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief This class measures temperature from analog temperature sensor and checks if it is below defined limit
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef AnalogTemp_h
#define AnalogTemp_h

#include "Arduino.h"

class AnalogTemp
{
  public:
    AnalogTemp(uint8_t SensorPin, uint8_t MaxTemperature, float mVperC, float ZeroVoltage);
  
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