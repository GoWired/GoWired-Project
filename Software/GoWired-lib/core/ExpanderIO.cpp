/**
 * @file ExpanderIO.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ExpanderIO.h"

PCF8575 Expander;

/**
 * @brief Construct a new ExpanderIO:: ExpanderIO object
 * 
 */
ExpanderIO::ExpanderIO()  {
  
  NewState = 0;
  State = 0;
  _HighStateDetected = true;
}

/**
 * @brief Initializes PCF8575 expander, as required by PCF8575.h library
 * 
 * @param Address I2C address of the expander
 */
void ExpanderIO::ExpanderInit(uint8_t Address)  {
    
  Expander.begin(Address);
}

/**
 * @brief Pass parameters to new ExpanderIO object
 * 
 * @param RelayOFF bool value to turn the relay off
 * @param Invert if set to true inverts functioning of _ReadDigital() function
 * @param Type sets the type of object
 * @param Pin1 first uC pin attached to the object (input pin)
 * @param Pin2 second uC pin attached to the object (output pin)
 */
void ExpanderIO::SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2) {
	
  _RelayOFF = RelayOFF;
  _Invert = Invert;
  SensorType = Type;

  switch(SensorType)  {
    // Door/window/motion sensors
    case 0:
      _SensorPin = Pin1;
      Expander.pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // No-pullup sensors
    case 1:
      _SensorPin = Pin1;
      Expander.pinMode(_SensorPin, INPUT);
      break;
    // Relay output
    case 2:
      _RelayPin = Pin1;
      Expander.pinMode(_RelayPin, OUTPUT);
      Expander.digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Button input
    case 3:
      _SensorPin = Pin1;
      Expander.pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // Button + Relay
    case 4:
      _SensorPin = Pin1;
      _RelayPin = Pin2;
      Expander.pinMode(_SensorPin, INPUT_PULLUP);
      Expander.pinMode(_RelayPin, OUTPUT);
      Expander.digitalWrite(_RelayPin, _RelayOFF);
      break;
    default:
      break;
  }
}

/**
 * @brief checks for any state changes on input attached to the object
 * 
 */
void ExpanderIO::CheckInput() {

  bool Reading;
  bool Shortpress = false;
  bool CheckHighState = true;
  uint32_t StartTime = millis();

  do  {
    // Hardcoded debounce value 50 ms
    Reading = _ReadDigital(50);

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
      // Hardcoded LongpressDuration 1000 ms
      if(millis() - StartTime > 1000) {
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
 * @brief reads any kind of digital input
 * 
 * @param DebounceValue debounce time [ms]
 * @return true if input state is true
 * @return false if input state is false
 */
bool ExpanderIO::_ReadDigital(uint8_t DebounceValue) {

  bool DigitalReading;
  bool PreviousReading = false;
  bool InputState = false;
  uint32_t StartTime = millis();

  do {
    DigitalReading = (_Invert ? Expander.digitalRead(_SensorPin) : !Expander.digitalRead(_SensorPin));

    if(DigitalReading && !PreviousReading)  {
      StartTime = millis();
    }

    if(millis() - StartTime > DebounceValue)  {
      if(DigitalReading) {
        InputState = true;
		break;
      }
    }
    
    if(millis() - StartTime > 255 || millis() < StartTime) {
      break;
    }

    PreviousReading = DigitalReading;
  } while(DigitalReading);

  return InputState;
}

/**
 * @brief Changes state of the relay
 * 
 */
void ExpanderIO::SetRelay() {

  bool RelayValue = NewState == 1 ? !_RelayOFF : _RelayOFF;

  Expander.digitalWrite(_RelayPin, RelayValue);
  State = NewState;
}
/*
 * 
 * EOF
 * 
 */
