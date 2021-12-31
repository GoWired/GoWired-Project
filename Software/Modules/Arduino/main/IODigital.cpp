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
}

/*  *******************************************************************************************
 *                                    Set Values
 *  *******************************************************************************************/
void IODigital::SetValues(bool RelayOFF, uint8_t Type, uint8_t Pin1, uint8_t Pin2) {
	
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
  uint32_t StartTime = millis();

  do  {
    if(SensorType == 0 || SensorType == 3 || SensorType == 4) {
      // Hardcoded DebounceValue = 50
      Reading = ReadDigital(50, false);
    }
    else  {
      Reading = ReadDigital(50, true);
    }

    switch(SensorType)  {
      case 0:
      case 1:
        if(Reading == true) {
          NewState = 1;
        }
        else  {
          NewState = 0;
        }
        break;
      case 3:
      case 4:
        if(!Shortpress && Reading)  {
          NewState = !OldState;
          Shortpress = true;
        }

        // Hardcoded LongpressDuration = 1000
        if(millis() - StartTime > 1000) {
          NewState = 2;
          break;
        }

        if(millis() < StartTime)  {
          StartTime = millis();
        }
        break;
      default:
        break;
    }
  } while(Reading);
}

// Read digital input
bool IODigital::ReadDigital(uint8_t DebounceValue, bool Invert) {

  bool DigitalReading;
  bool PreviousReading = false;
  bool InputState = false;
  uint32_t Timeout = millis();
  uint32_t StartTime = Timeout;

  do {
    DigitalReading = (Invert ? digitalRead(_SensorPin) : !digitalRead(_SensorPin));

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
