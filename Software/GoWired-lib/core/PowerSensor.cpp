/**
 * @file PowerSensor.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
 
#include "PowerSensor.h"

/**
 * @brief Construct a new Power Sensor:: Power Sensor object
 * 
 */
PowerSensor::PowerSensor()  {
	
	OldValue = 0;
}

/**
 * @brief Pass parameters to new PowerSensor object
 * 
 * @param SensorPin uC pin to which current sensor is attached
 * @param mVperAmp voltage [mV] per one Amp for attached sensor
 * @param ReceiverVoltage voltage [V] of connected receiver
 * @param MaxCurrent maximum current allowed, defined current limit [A]
 * @param PMTime power measuring time [ms]
 * @param Vcc current uC voltage
 */
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

/**
 * @brief Measures AC
 * 
 * @param Vcc current uC voltage
 * @return float measured current value [A]
 */
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

/**
 * @brief Measures DC
 * 
 * @param Vcc current uC voltage
 * @return float measured current value [A]
 */
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

/**
 * @brief Calculates electrical power
 * 
 * @param Current current, as measured by MeasureAC() or MeasureDC() functions [A]
 * @param cosfi power factor value [0-1]
 * @return float calculated power value [W]
 */
float PowerSensor::CalculatePower(float Current, float cosfi)  {

  float Power = Current * _ReceiverVoltage * cosfi;
  OldValue = Current;
  
  return Power;
}

/**
 * @brief Checks if measured current is within a defined limit
 * 
 * @param Current current, as measured by MeasureAC() or MeasureDC() functions [A]
 * @return true if Current is higher than defined maximum
 * @return false if Current is below defined maximum
 */
bool PowerSensor::ElectricalStatus(float Current) {

  if(Current > _MaxCurrent)  {
    return true;
  }
  else  {
    return false;
  }
}
