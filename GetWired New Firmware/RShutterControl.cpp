/*
 * RShutterControl.cpp
 */

#include "RShutterControl.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
RShutterControl::RShutterControl(uint8_t UpPin, uint8_t DownPin, bool RelayOn, bool RelayOff)  {

  _DownPin = DownPin;
  _UpPin = UpPin;
  _RelayOn = RelayOn;
  _RelayOff = RelayOff;
  
  pinMode(_UpPin, OUTPUT);  digitalWrite(_UpPin, RelayOff);
  pinMode(_DownPin, OUTPUT);  digitalWrite(_DownPin, RelayOff);

  NewState = 2;
  State = 2;

  uint8_t DownTime;
  uint8_t UpTime;

  EEPROM.get(EEA_RS_TIME_DOWN, DownTime);
  EEPROM.get(EEA_RS_TIME_UP, UpTime);
  
  if(UpTime != 255 && DownTime != 255)  {
    Calibrated = true;
    _UpTime = UpTime;
    _DownTime = DownTime;
    EEPROM.get(EEA_RS_POSITION, Position);
  }
  else  {
    Calibrated = false;
  }

}

/*  *******************************************************************************************
 *                                        Auto Calibration
 *  *******************************************************************************************/
void RShutterControl::Calibration(uint8_t UpTime, uint8_t DownTime)  {
  
  Position = 0;

  _UpTime = UpTime;
  _DownTime = DownTime;

  EEPROM.put(EEA_RS_TIME_DOWN, _DownTime);
  EEPROM.put(EEA_RS_TIME_UP, _UpTime);
  EEPROM.put(EEA_RS_POSITION, Position);

  Calibrated = true;  
}

// Read Message
uint8_t RShutterControl::ReadMessage(uint8_t Order) {

  if(State == 2)  {
    NewState = Order;

    if(Order == 0) {
      return _UpTime;
    }
    else if(Order == 1)  {
      return _DownTime;
    }
    else if(Order == 2) {
      return 0;
    }
  }
  else  {
    NewState = 2;
    return 0;
  }
}

// Read Buttons
uint8_t RShutterControl::ReadButtons(uint8_t Button)  {

  // Roller shutter is stopped -> start moving
  if(State == 2)  {
    NewState = Button;
    if(Button == 0) {
      return _UpTime;
    }
    else if(Button == 1)  {
      return _DownTime;
    }
  }
  // Roller shutter is moving -> stop
  else  {
    NewState = 2;
    return 0;
  }
}

// Read New Position
uint8_t RShutterControl::ReadNewPosition(uint8_t NewPosition) {

  uint8_t MovementRange = NewPosition - Position;             // Downward => MR > 0; Upward MR < 0
  bool MovementDirection = MovementRange > 0 ? 1 : 0;         // MovementDirection: 1 -> Down; 0 -> Up
  uint32_t MovementTime;

  if(!MovementDirection)  {
    NewState = 0;
    MovementTime = _UpTime * abs(MovementRange) * 10;
  }
  else  {
    NewState = 1;
    MovementTime = _DownTime * abs(MovementRange) * 10;
  }

  return MovementTime;
}

// Movement control
uint8_t RShutterControl::Movement()  {

  // Stop roller shutter
  if(NewState == 2) {
    digitalWrite(_UpPin, _RelayOff);
    digitalWrite(_DownPin, _RelayOff);
    State = NewState;
    return State;
  }
  // Move upward
  else if(NewState == 0)  {
    digitalWrite(_DownPin, _RelayOff);
    digitalWrite(_UpPin, _RelayOn);
    State = NewState;
    return State;
  }
  // Move downward
  else if(NewState == 1) {
    digitalWrite(_UpPin, _RelayOff);
    digitalWrite(_DownPin, _RelayOn);
    State = NewState;
    return State;
  }
}

// Calculate new position
void RShutterControl::CalculatePosition(bool Direction, unsigned long MeasuredTime)  {

  uint8_t DirectionTime;

  if(Direction) {
    DirectionTime = _DownTime * 1000;
  }
  else  {
    DirectionTime = _UpTime * 1000;
  }

  uint8_t PositionChange = (float) MeasuredTime / (float) DirectionTime;
  PositionChange = PositionChange * 100;

  Position += Direction == 1 ? PositionChange : -PositionChange;
  Position = Position > 100 ? 100 : Position;
  Position = Position < 0 ? 0 : Position;

  EEPROM.put(EEA_RS_POSITION, Position);
}

