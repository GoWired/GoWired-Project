/*
 * InOut.cpp
 */

#include "InOut.h"

// Constructor
InOut::InOut()  {
  
  _NewState = 0;
  _State = 0;
  DebugValue = 0;
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
    // Touch Field + External button + Relay (2Relay)
    case 1:
      _SensorPin = Pin1;
      _SensorPin2 = Pin2;
      _RelayPin = Pin3;
      pinMode(_SensorPin2, INPUT_PULLUP);
      pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Default case
    default:
      break;
  }

  ReadReference();
}

// Measure touch field reference value
void InOut::ReadReference()  {

  _TouchReference = ADCTouch.read(_SensorPin, 500);         // ADCTouch.read(pin, number of samples)
}

// Return _NewState
uint8_t InOut::ReadNewState() {

  return _NewState;
}

uint8_t InOut::ReadState()  {

  return _State;
}

// Set _NewState
void InOut::SetState(uint8_t NewState)  {

  _NewState = NewState;
  DebugValue = 3;
}

// Read analog & digital inputs
void InOut::ReadInput(uint8_t Threshold, uint16_t LongpressDuration, uint8_t DebounceValue) {

  bool Reading;
  bool Shortpress = false;
  uint32_t StartTime = millis();

  do  {
    Reading = _ReadAnalog(Threshold);
    
    if(SensorType == 1 && !Reading) {
      Reading = _ReadDigital(DebounceValue);
    }

    if(!Shortpress && Reading) {
      _NewState = !_State;
      Shortpress = true;
    }

    if(millis() - StartTime > LongpressDuration) {
      _NewState = 2;
      break;
    }

    if(millis() < StartTime)  {
      StartTime = millis();
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
    DebugValue = 1;
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
        DebugValue = 2;
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
