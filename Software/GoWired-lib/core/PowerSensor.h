/**
 * @file PowerSensor.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief class reads the output of attached current sensor and calculates electrical power
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PowerSensor_h
#define PowerSensor_h

#include "Arduino.h"

class PowerSensor
{
  public:
  PowerSensor();      
  
  float OldValue;

  void SetValues(uint8_t SensorPin, uint8_t mVperAmp, uint8_t ReceiverVoltage, uint8_t MaxCurrent, uint8_t PMTime, float Vcc);
  float MeasureAC(float Vcc);
  float MeasureDC(float Vcc);
  float CalculatePower(float Current, float cosfi);
  bool ElectricalStatus(float Current);

  private:
  uint8_t _SensorPin;
  uint8_t _mVperAmp;             
  uint8_t _ReceiverVoltage;
  uint8_t _MaxCurrent;
  uint8_t _PowerMeasuringTime;
  uint16_t _ZeroOffset;
  
};

#endif
