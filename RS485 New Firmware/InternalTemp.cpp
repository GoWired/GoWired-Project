/*
 * 
 */

#include "InternalTemp.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
InternalTemp::InternalTemp(int SensorPin)  {

  pinMode(SensorPin, INPUT);
  _SensorPin = SensorPin;

  _mVperC = MVPERC;
  _ZeroVoltage = ZEROVOLTAGE;
  
}

/*  *******************************************************************************************
 *                                 Temperature measurement
 *  *******************************************************************************************/
float InternalTemp::MeasureT()  {

  int ReadValue = analogRead(_SensorPin);
  float Result = (ReadValue * 5.0) / 1024.0;
  Result -= _ZeroVoltage;                        // V = 500 mV in 0C
  Temperature = Result / _mVperC;       // Temperature coefficient 10mV/C

  return Temperature;
}

/*  *******************************************************************************************
 *                                  Thermal Status Check
 *  *******************************************************************************************/
bool InternalTemp::ThermalStatus(float Temperature)  {

  if(Temperature > MAX_TEMPERATURE)  {
    return true;
  }
  else  {
    return false;
  }
}
