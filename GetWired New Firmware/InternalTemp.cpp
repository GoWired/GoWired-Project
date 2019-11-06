/*
 * 
 */

#include "InternalTemp.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
InternalTemp::InternalTemp(uint8_t SensorPin, uint8_t MaxTemperature, float mVperC, float ZeroVoltage, float MCUVoltage)  {
	
	_SensorPin = SensorPin;
	_MaxTemperature = MaxTemperature;
	_mVperC = mVperC;
	_ZeroVoltage = ZeroVoltage;
	_MCUVoltage = MCUVoltage;

	pinMode(SensorPin, INPUT);
  
}

/*  *******************************************************************************************
 *                                 Temperature measurement
 *  *******************************************************************************************/
float InternalTemp::MeasureT()  {

  int ReadValue = analogRead(_SensorPin);
  float Result = (ReadValue * _MCUVoltage) / 1024.0;
  Result -= _ZeroVoltage;                        // V = 500 mV in 0C
  Temperature = Result / _mVperC;       // Temperature coefficient 10mV/C

  return Temperature;
}

/*  *******************************************************************************************
 *                                  Thermal Status Check
 *  *******************************************************************************************/
bool InternalTemp::ThermalStatus(float Temperature)  {

  if(Temperature > _MaxTemperature)  {
    return true;
  }
  else  {
    return false;
  }
}