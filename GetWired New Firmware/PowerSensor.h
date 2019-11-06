/*
 *  
 */

#ifndef PowerSensor_h
#define PowerSensor_h

#include "Arduino.h"

class PowerSensor
{
  public:
  PowerSensor();      

  void SetValues(uint8_t SensorPin, uint8_t mVperAmp, uint8_t ReceiverVoltage, uint8_t MaxCurrent, uint8_t PMTime, float MCUVoltage);
  float MeasureAC();
  float CalculatePower(float Current);
  bool ElectricalStatus(float Current);

  private:
  uint8_t _SensorPin;
  uint8_t _mVperAmp;             // 100 mV/A for 20A module, 185 mV/A for 5A module
  uint8_t _ReceiverVoltage;
  uint8_t _MaxCurrent;
  uint8_t _PowerMaasuringTime;
  float _MCUVoltage;
  
};


#endif