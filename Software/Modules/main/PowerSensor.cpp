/*
 * PowerSensor.cpp
 */
 
#include "PowerSensor.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
PowerSensor::PowerSensor()  {
	
	OldValue = 0;
}

/*  *******************************************************************************************
 *                                      Set Values
 *  *******************************************************************************************/
void PowerSensor::SetValues(uint8_t SensorPin, uint8_t mVperAmp, uint8_t ReceiverVoltage, uint8_t MaxCurrent, uint8_t PMTime, float Vcc) {

  _SensorPin = SensorPin;
  _mVperAmp = mVperAmp;
  _ReceiverVoltage = ReceiverVoltage;
  _MaxCurrent = MaxCurrent;
  _PowerMeasuringTime = PMTime;
  
  pinMode(_SensorPin, INPUT);

  uint16_t Temp = 0;
  
  for(uint8_t i=1; i<=10; i++)	{
	  uint16_t ZeroVoltage = analogRead(_SensorPin);
	  Temp += ZeroVoltage;
	}

  _ZeroOffset = Temp / 10;
}

/*  *******************************************************************************************
 *                                   Current Measurement
 *  *******************************************************************************************/
float PowerSensor::MeasureAC(float Vcc)  {

  uint16_t ReadValue;
  uint16_t MaxValue = 0;
  uint16_t MinValue = 1024;

  uint32_t StartTime = millis();
  while((millis() - StartTime) < _PowerMeasuringTime)  {
    ReadValue = analogRead(_SensorPin);
    if(ReadValue > MaxValue)  {
      MaxValue = ReadValue;
    }
    if(ReadValue < MinValue)  {
      MinValue = ReadValue;
    }
    if(millis() < StartTime)  {
      StartTime = millis();
    }
  }

  uint16_t MaxMinValue = (MaxValue - MinValue);
  if(MaxMinValue <= 15) {
    MaxMinValue = 0;
  }
    
  float Result = (MaxMinValue * Vcc * 0.3535) / (_mVperAmp * 1024.0);

  return Result;
}

/*  *******************************************************************************************
 *                                   Current Measurement
 *  *******************************************************************************************/
float PowerSensor::MeasureDC(float Vcc)  {

  int ReadValue;
  uint32_t AverageSum = 0;
  uint16_t N = 0;

  uint32_t StartTime = millis();
  while((millis() - StartTime) < _PowerMeasuringTime)  {
    ReadValue = analogRead(_SensorPin);
    ReadValue = ReadValue - _ZeroOffset;
    ReadValue = abs(ReadValue);
    AverageSum += ReadValue;
    N++;
    if(millis() < StartTime)  {
      StartTime = millis();
    }
  }

  float Result = (float)AverageSum / N;
  Result = (Result * Vcc) / (_mVperAmp * 1024.0);

  return Result;
}

/*  *******************************************************************************************
 *                                      Power Calculation
 *  *******************************************************************************************/
float PowerSensor::CalculatePower(float Current, float cosfi)  {

  float Power = Current * _ReceiverVoltage * cosfi;
  OldValue = Current;
  
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
