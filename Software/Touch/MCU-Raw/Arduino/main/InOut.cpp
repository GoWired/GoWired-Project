/*
 * InOut.cpp
 */

#include "InOut.h"

// Constructor
InOut::InOut()  {
  
  _NewState = 0;
  _State = 0;
}

// Set Values - initialization
void InOut::SetValues(bool RelayOFF, bool RelayON, uint8_t Type, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3) {

  _RelayOFF = RelayOFF;
  _RelayON = RelayON;

  SensorType = Type;
  switch(SensorType)  {
    // Touch Field (Dimmers)
    case 0:
      _SensorPin = Pin1;
      break;
    // Touch field + external button + output
    case 1:
      _SensorPin = Pin1;
      _SensorPin2 = Pin2;
      _RelayPin = Pin3;
      pinMode(_SensorPin2, INPUT_PULLUP);
      pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Touch field + output
    case 2:
      _SensorPin = Pin1;
      _RelayPin = Pin2;
      pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
    // Default case
    default:
      break;
  }

  // Measure reference value
  _TouchReference = ADCTouch.read(_SensorPin, 500);
}

// Return _NewState
uint8_t InOut::ReadNewState() {

  return _NewState;
}

// Return _State
uint8_t InOut::ReadState()  {

  return _State;
}

// Set _NewState
void InOut::SetState(uint8_t NewState)  {

  _NewState = NewState;
}

void InOut::ReadInput(uint16_t Threshold, uint8_t DebounceValue, bool Monostable) {

  bool Reading;
  bool Shortpress = false;

  do  {
    Reading = _ReadAnalog(Threshold);
    
    if(SensorType == 1 && !Reading) {
      Reading = _ReadDigital(DebounceValue);
    }

    if(Monostable)  {
      if(!Shortpress && Reading)  {
        _NewState = !_State;  Shortpress = true;
        SetRelay();
      }
      else if(Shortpress && !Reading) {
        _NewState = !_State;  Shortpress = false;
        SetRelay();
      }
    }
    else  {
      if(!Shortpress && Reading) {
        _NewState = !_State;
        Shortpress = true;
      }
    }
  } while(Reading);
}

// Read analog input
bool InOut::_ReadAnalog(uint8_t Threshold)  {

  int TouchValue;
  bool ButtonState = false;

  TouchValue = ADCTouch.read(_SensorPin, 64);
  TouchValue -= _TouchReference;
  TouchDiagnosisValue = TouchValue;

  if(TouchValue > Threshold)  {
    ButtonState = true;
  }

  return ButtonState;
}

// Read digital input
bool InOut::_ReadDigital(uint8_t DebounceValue) {

  bool DigitalReading;
  bool PreviousReading = false;
  bool InputState = false;
  uint32_t Timeout = millis();
  uint32_t StartTime = Timeout;

  do {
    DigitalReading = !digitalRead(_SensorPin2);

    if(DigitalReading && !PreviousReading)  {
      StartTime = millis();
    }

    if(millis() - StartTime > DebounceValue)  {
      if(DigitalReading) {
        InputState = true;
      }
    }
    
    if(millis() - Timeout > 255 || millis() < StartTime) {
      break;
    }

    PreviousReading = DigitalReading;
  } while(DigitalReading);

  return InputState;
}

// Set Relay
void InOut::SetRelay() {

  if(_NewState == 1)  {
    digitalWrite(_RelayPin, _RelayON);
  }
  else  {
    digitalWrite(_RelayPin, _RelayOFF);
  }
  
  _State = _NewState;
}
