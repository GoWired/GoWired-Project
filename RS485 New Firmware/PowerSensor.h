/*
 *  
 */

#ifndef PowerSensor_h
#define PowerSensor_h

#include "Arduino.h"

#ifndef MAX_CURRENT
#define MAX_CURRENT 3
#endif

#ifndef POWER_MEASURING_TIME
#define POWER_MEASURING_TIME 100   // This value needs to be carefully tested!
#endif

#ifndef MVPERAMP
#define MVPERAMP 185               // ACS7125A: 185 mV/A; ACS71220A: 100 mV/A
#endif

class PowerSensor
{
  public:
  PowerSensor();      

  void SetValues(int SensorPin);
  float MeasureAC();
  float CalculatePower(float Current, int ACVoltage);
  bool ElectricalStatus(float Current);

  private:
  uint8_t _SensorPin;
  uint8_t _mVperAmp;            // 100 mV/A for 20A module, 185 mV/A for 5A module
  
};


#endif
