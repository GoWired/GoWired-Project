/*
 * IODigital.cpp
 */

#include "IODigital.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
IODigital::IODigital()  {
  
  NewState = 0;
  OldState = 0;
  _HighStateDetected = true;
}

/*  *******************************************************************************************
 *                                    Set Values
 *  *******************************************************************************************/
void IODigital::SetValues(bool RelayOFF, bool Invert, uint8_t Type, uint8_t Pin1, uint8_t Pin2) {
	
  _RelayOFF = RelayOFF;
  _Invert = Invert;
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
    // Button input
    case 3:
      _SensorPin = Pin1;
      pinMode(_SensorPin, INPUT_PULLUP);
      break;
    // Button + Relay
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
void IODigital::CheckInput() {

  bool Reading;
  bool Shortpress = false;
  bool CheckHighState = true;
  uint32_t StartTime = millis();

  do  {
    // Hardcoded debounce value 50 ms
    Reading = _ReadDigital(50);

    if(SensorType == 0 || SensorType == 1)  {
      if(OldState != Reading)  {
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
        NewState = !OldState;
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

// Read digital input
bool IODigital::_ReadDigital(uint8_t DebounceValue) {

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
      }
    }
    
    if(millis() - StartTime > 255 || millis() < StartTime) {
      break;
    }

    PreviousReading = DigitalReading;
  } while(DigitalReading);

  return InputState;
}

/*  *******************************************************************************************
 *                                      Set Relay
 *  *******************************************************************************************/
void IODigital::SetRelay() {

  digitalWrite(_RelayPin, NewState);
  OldState = NewState;
}
/*
 * 
 * EOF
 * 
 */
