/*
 * PowerSensor.h
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
