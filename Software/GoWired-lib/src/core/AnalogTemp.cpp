/**
 * @file AnalogTemp.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief see AnalogTemp.h
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "AnalogTemp.h"

/**
 * @brief Construct a new Analog Temp:: Analog Temp object
 * 
 * @param SensorPin pin to which analog temperature sensor is attached
 * @param MaxTemperature maximum temperature allowed, defined temperature limit
 * @param mVperC voltage [mV] per a Celcius degree for attached sensor 
 * @param ZeroVoltage voltage for 0 Celcius degrees for attached sensor
 */
AnalogTemp::AnalogTemp(uint8_t SensorPin, uint8_t MaxTemperature, float mVperC, float ZeroVoltage)  {
	
	_SensorPin = SensorPin;
	_MaxTemperature = MaxTemperature;
	_mVperC = mVperC;
	_ZeroVoltage = ZeroVoltage;

	pinMode(SensorPin, INPUT);
}

/**
 * @brief Measures temperature 
 * 
 * @param Vcc current uC voltage
 * @return float measured temperature
 */
float AnalogTemp::MeasureT(float Vcc)  {

  int ReadValue = analogRead(_SensorPin);
  float Result = (ReadValue * Vcc) / 1024.0;
  Result -= _ZeroVoltage;                        // V = 500 mV in 0C
  Temperature = Result / _mVperC;       // Temperature coefficient 10mV/C

  return Temperature;
}

/**
 * @brief Returns thermal status (if temperature is higher than defined limit or not)
 * 
 * @param Temperature as measured by MeasureT function
 * @return true if Temperature is higher than defined maximum
 * @return false if Temperature is lower than defined maximum
 */
bool AnalogTemp::ThermalStatus(float Temperature)  {

  if(Temperature > _MaxTemperature)  {
    return true;
  }
  else  {
    return false;
  }
}
