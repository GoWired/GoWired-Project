/*
 * 
 */
 
#include "PowerSensor.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
PowerSensor::PowerSensor()  {

}

/*  *******************************************************************************************
 *                                      Set Values
 *  *******************************************************************************************/
void PowerSensor::SetValues(int SensorPin) {

  _mVperAmp = MVPERAMP;
  _SensorPin = SensorPin;
  pinMode(_SensorPin, INPUT);
}

/*  *******************************************************************************************
 *                                   Current Measurement
 *  *******************************************************************************************/
float PowerSensor::MeasureAC()  {
  
  float Result;

  int ReadValue;
  int MaxValue = 0;
  int MinValue = 1024;

  uint32_t StartTime = millis();
  while((millis() - StartTime) < POWER_MEASURING_TIME)  {
    ReadValue = analogRead(_SensorPin);
    if(ReadValue > MaxValue)  {
      MaxValue = ReadValue;
    }
    if(ReadValue < MinValue)  {
      MinValue = ReadValue;
    }
  }

  Result = ((MaxValue - MinValue) * 5.0) / 1024.0;
    
  float VRMS = (Result / 2) * 0.707;
  float AmpsRMS = (VRMS * 1000) / _mVperAmp;

  return AmpsRMS;
}

/*  *******************************************************************************************
 *                                      Power Calculation
 *  *******************************************************************************************/
float PowerSensor::CalculatePower(float Current, int ACVoltage)  {

  float Power = Current * ACVoltage;
  
  return Power;
}

/*  *******************************************************************************************
 *                                  Electrical Status Check
 *  *******************************************************************************************/
bool PowerSensor::ElectricalStatus(float Current) {

  if(Current > MAX_CURRENT)  {
    return true;
  }
  else  {
    return false;
  }
}
