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
void PowerSensor::SetValues(uint8_t SensorPin, uint8_t mVperAmp, uint8_t ReceiverVoltage, uint8_t MaxCurrent, uint8_t PMTime, float MCUVoltage) {

  _SensorPin = SensorPin;
  _mVperAmp = mVperAmp;
  _ReceiverVoltage = ReceiverVoltage;
  _MaxCurrent = MaxCurrent;
  _PowerMaasuringTime = PMTime;
  _MCUVoltage = MCUVoltage;
  
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
  while((millis() - StartTime) < _PowerMaasuringTime)  {
    ReadValue = analogRead(_SensorPin);
    if(ReadValue > MaxValue)  {
      MaxValue = ReadValue;
    }
    if(ReadValue < MinValue)  {
      MinValue = ReadValue;
    }
  }

  Result = ((MaxValue - MinValue) * _MCUVoltage) / 1024.0;
    
  float VRMS = (Result / 2) * 0.707;
  float AmpsRMS = (VRMS * 1000) / _mVperAmp;

  return AmpsRMS;
}

/*  *******************************************************************************************
 *                                      Power Calculation
 *  *******************************************************************************************/
float PowerSensor::CalculatePower(float Current)  {

  float Power = Current * _ReceiverVoltage;
  
  return Power;
}

/*  *******************************************************************************************
 *                                  Electrical Status Check
 *  *******************************************************************************************/
bool PowerSensor::ElectricalStatus(float Current) {

  if(Current > _MaxCurrent)  {
    return true;
  }
  else  {
    return false;
  }
}