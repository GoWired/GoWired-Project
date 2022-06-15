/**
 * @file CommonIO.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "CommonIO.h"

/**
 * @brief Construct a new CommonIO:: CommonIO object
 * 
 */
CommonIO::CommonIO()  {
  
  NewState = 0;
  State = 0;
  DebugValue = 0;
  _HighStateDetected = true;
}

/**
 * @brief Pass parameters to new CommonIO object
 * 
 * @param RelayOFF bool value to turn the relay off
 * @param Invert if set to true inverts functioning of _ReadDigital() function
 * @param SensorType sets the type of object
 * @param Pin1 first uC pin attached to this object (primary input)
 * @param Pin2 second uC pin attached to this object (output/secondary input)
 * @param Pin3 third uC pin attached to this object (output)
 */
void CommonIO::SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3) {
	
  _RelayOFF = RelayOFF;
  _Invert = Invert;
  SensorType = Type;

  switch(SensorType)  {
    // Security sensors (door/window/motion)
    case 0:
      _SensorPin = Pin1;    pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // Generic sensors
    case 1:
      _SensorPin = Pin1;    pinMode(_SensorPin, INPUT);
      break;
    // Output
    case 2:
      _RelayPin = Pin1;     pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Digital button input
    case 3:
      _SensorPin = Pin1;    pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // Digital button + output
    case 4:
      _SensorPin = Pin1;    pinMode(_SensorPin, INPUT_PULLUP);
      _RelayPin = Pin2;     pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Touch button
    case 5:
      _SensorPin = Pin1;    ReadReference();
      break;
    // Touch button + digital button + output
    case 6:
      _SensorPin = Pin1;    ReadReference();
      _SensorPin2 = Pin2;   pinMode(_SensorPin2, INPUT_PULLUP);
      _RelayPin = Pin3;     pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Touch button + output
    case 7:
      _SensorPin = Pin1;    ReadReference();
      _RelayPin = Pin2;     pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    default:
      break;
  }
}

/**
 * @brief reads reference value for touch buttons
 * 
 */
void CommonIO::ReadReference()  {

  _TouchReference = ADCTouch.read(_SensorPin, 500);         // ADCTouch.read(pin, number of samples)
}

/**
 * @brief saves new state received by message
 * 
 * @param ReceivedState 
 */
void CommonIO::SetState(uint8_t ReceivedState)  {

  NewState = ReceivedState;
  DebugValue = 1;
}

/**
 * @brief checks for any state changes on input attached to the object (digital)
 * 
 * @param LongpressDuration duration of longpress [ms]
 * @param DebounceValue debounce time [ms]
 */
void CommonIO::CheckInput(uint16_t LongpressDuration, uint8_t DebounceValue) {

  bool Reading;
  bool Shortpress = false;
  bool CheckHighState = true;
  uint32_t StartTime = millis();

  do  {
    Reading = _ReadDigital(DebounceValue);

    if(SensorType == 0 || SensorType == 1)  {
      if(State != Reading)  {
        NewState = Reading;
        break;
      }
      else break;
    }
    else if(SensorType == 3 || SensorType == 4)  {
      if(CheckHighState)  {
        // Check if high state was detected
        if(!_HighStateDetected) {
          _HighStateDetected = !Reading;
          break;
        }
        CheckHighState = false;
      }
      if(!Shortpress && Reading)  {
        NewState = !State;
        Shortpress = true;
        _HighStateDetected = false;
      }
      if(millis() - StartTime > LongpressDuration) {
        NewState = 2;
        break;
      }
      if(millis() < StartTime)  {
        StartTime = millis();
      }
    }
  } while(Reading);
}

/**
 * @brief check for any state changes on inputs attached to the object (touch & digital)
 * 
 * @param Threshold value of threshold for touch buttons [-]
 * @param LongpressDuration duration of longpress [ms]
 * @param DebounceValue debounce time [ms]
 */
void CommonIO::CheckInput2(uint8_t Threshold, uint16_t LongpressDuration, uint8_t DebounceValue) {

  bool Reading;
  bool Shortpress = false;
  uint32_t StartTime = millis();

  do  {
    Reading = _ReadAnalog(Threshold);
    
    if(SensorType == 6 && !Reading) {
      Reading = _ReadDigital(DebounceValue);
    }

    if(!Shortpress && Reading) {
      NewState = !State;
      Shortpress = true;
    }

    if(millis() - StartTime > LongpressDuration) {
      NewState = 2;
      break;
    }

    if(millis() < StartTime)  {
      StartTime = millis();
    }
  } while(Reading);
}

void CommonIO::CheckInput3(uint8_t Threshold, uint8_t DebounceValue, bool Monostable) {

  bool Reading;
  bool Shortpress = false;

  do  {
    Reading = _ReadAnalog(Threshold);
    
    if(SensorType == 6 && !Reading) {
      Reading = _ReadDigital(DebounceValue);
    }

    if(Monostable)  {
      if(!Shortpress && Reading)  {
        NewState = !State;  Shortpress = true;
        SetRelay();
      }
      else if(Shortpress && !Reading) {
        NewState = !State;  Shortpress = false;
        SetRelay();
      }
    }
    else  {
      if(!Shortpress && Reading) {
        NewState = !State;
        Shortpress = true;
      }
    }
  } while(Reading);
}

/**
 * @brief reads any kind of digital input
 * 
 * @param DebounceValue debounce time [ms]
 * @return true if input state is true
 * @return false if input state is false
 */
bool CommonIO::_ReadDigital(uint8_t DebounceValue) {

  bool DigitalReading;
  bool PreviousReading = false;
  bool InputState = false;
  uint32_t StartTime = millis();

  do {
    DigitalReading = (_Invert ? digitalRead(_SensorPin) : !digitalRead(_SensorPin));

    if(DigitalReading && !PreviousReading)  {
      StartTime = millis();
    }

    if(millis() - StartTime > DebounceValue)  {
      if(DigitalReading) {
        InputState = true;
        DebugValue = 2;
        break;
      }
    }
    
    // Hardcoded timeout 255 ms
    if(millis() - StartTime > 255 || millis() < StartTime) {
      break;
    }

    PreviousReading = DigitalReading;
  } while(DigitalReading);

  return InputState;
}

/**
 * @brief reads input from touch buttons
 * 
 * @param Threshold value of threshold for touch buttons [-]
 * @return true if touch was sensed
 * @return false if touch was not sensed
 */
bool CommonIO::_ReadAnalog(uint8_t Threshold)  {

  int TouchValue;
  bool ButtonState = false;

  TouchValue = ADCTouch.read(_SensorPin, 64);
  TouchValue -= _TouchReference;
  TouchDiagnosisValue = TouchValue;

  if(TouchValue > Threshold)  {
    ButtonState = true;
    DebugValue = 3;
  }

  return ButtonState;
}

/**
 * @brief Changes state of the relay
 * 
 */
void CommonIO::SetRelay() {

  bool RelayValue = NewState == 1 ? !_RelayOFF : _RelayOFF;

  digitalWrite(_RelayPin, RelayValue);
  State = NewState;
}
/*
 * 
 * EOF
 * 
 */
