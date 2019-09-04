/*
 * PinShift - variable corresponding to arythmetical difference
 * between a sensor and its pin
 */

#include "UniversalInput.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
UniversalInput::UniversalInput()  {
  
  NewState = 0;
  OldState = 0;
}

/*  *******************************************************************************************
 *                                    Set Values
 *  *******************************************************************************************/
void UniversalInput::SetValues(bool RelayOFF, uint8_t Type, uint8_t Pin1, uint8_t Pin2=0) {
	
	_RelayOFF = RelayOFF;

  SensorType = Type;
  switch(SensorType)  {
    // Door/window sensor
    case 0:
      _SensorPin = Pin1;
      pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // Motion sensor
    case 1:
      _SensorPin = Pin1;
      pinMode(_SensorPin, INPUT);
      break;
    // Relay output
    case 2:
      _RelayPin = Pin1;
      pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    // Button input + Relay output
    case 3:
      _SensorPin = Pin1;
      pinMode(_SensorPin, INPUT_PULLUP);
      break;
    case 4:
      _SensorPin = Pin1;
      _RelayPin = Pin2;
      pinMode(_SensorPin, INPUT_PULLUP);
      pinMode(_RelayPin, OUTPUT);
      digitalWrite(_RelayPin, _RelayOFF);
      break;
    default:
      break;
  }
}

/*  *******************************************************************************************
 *                                      Input Check
 *  *******************************************************************************************/
void UniversalInput::CheckInput() {

  if(SensorType == 0 || SensorType == 1) {
    if(digitalRead(_SensorPin) == LOW)  {
      NewState = 0;
    }
    else if(digitalRead(_SensorPin) != LOW)  {
      NewState = 1;
    }
  }
  else if(SensorType == 3 || SensorType == 4)  {
    
    if(digitalRead(_SensorPin) != LOW)  {
      _HighStateDetection = true;
      _LowStateDetection = false;
      _Condition = false;
    }
    if(_HighStateDetection == true) {
      unsigned long StartTime = millis();
      while(digitalRead(_SensorPin) == LOW)  {
        _LowStateDetection = true;
        _HighStateDetection = false;
        if(millis() - StartTime > 1000) {
          _Condition = true;
          break; 
        }
      }
      if(_Condition == false && _LowStateDetection == true) {
        if(OldState == 0) {
          NewState = 1;
        }
        else if(OldState == 1)  {
          NewState = 0;
        }
      }
      else if(_Condition == true) {
        NewState = 2;
      }
    }
  }
}

/*  *******************************************************************************************
 *                                      Set Relay
 *  *******************************************************************************************/
void UniversalInput::SetRelay() {

  digitalWrite(_RelayPin, NewState);
  OldState = NewState;
}
/*
 * 
 * EOF
 * 
 */
