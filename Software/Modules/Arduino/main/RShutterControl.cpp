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
  
  //Position = 0;

  _UpTime = UpTime;
  _DownTime = DownTime;

  EEPROM.put(EEA_RS_TIME_DOWN, _DownTime);
  EEPROM.put(EEA_RS_TIME_UP, _UpTime);
  EEPROM.put(EEA_RS_POSITION, Position);

  //Calibrated = true;
}

// Read Message
uint32_t RShutterControl::ReadMessage(uint8_t Order) {

  uint32_t MovementTime;

  if(State == 2)  {
    NewState = Order;

    if(Order == 0) {
      MovementTime = _UpTime;
    }
    else if(Order == 1)  {
      MovementTime = _DownTime;
    }
    else if(Order == 2) {
      MovementTime = 0;
    }
  }
  else  {
    NewState = 2;
    MovementTime = 0;
  }

  return MovementTime;
}

// Read Buttons
uint32_t RShutterControl::ReadButtons(uint8_t Button)  {

  uint32_t MovementTime;

  // Roller shutter is stopped -> start moving
  if(State == 2)  {
    NewState = Button;
    if(Button == 0) {
      MovementTime = _UpTime;
    }
    else if(Button == 1)  {
      MovementTime = _DownTime;
    }
  }
  // Roller shutter is moving -> stop
  else  {
    NewState = 2;
    MovementTime = 0;
  }

  return MovementTime;
}

// Read New Position
uint32_t RShutterControl::ReadNewPosition(int NewPosition) {

  int MovementRange = NewPosition - Position;             // Downward => MR > 0; Upward MR < 0
  bool MovementDirection = MovementRange > 0 ? 1 : 0;         // MovementDirection: 1 -> Down; 0 -> Up
  uint32_t MovementTime;

  if(!MovementDirection)  {
    NewState = 0;
    MovementTime = _UpTime * abs(MovementRange);
  }
  else  {
    NewState = 1;
    MovementTime = _DownTime * abs(MovementRange);
  }

  return MovementTime;
}

// Movement control
uint8_t RShutterControl::Movement()  {

  // Stop roller shutter
  if(NewState == 2) {
    digitalWrite(_UpPin, _RelayOff);
    digitalWrite(_DownPin, _RelayOff);
  }
  // Move upward
  else if(NewState == 0)  {
    digitalWrite(_DownPin, _RelayOff);
    digitalWrite(_UpPin, _RelayOn);
  }
  // Move downward
  else if(NewState == 1) {
    digitalWrite(_UpPin, _RelayOff);
    digitalWrite(_DownPin, _RelayOn);
  }

  State = NewState;
  return State;  
}

// Calculate new position
void RShutterControl::CalculatePosition(bool Direction, uint32_t MeasuredTime)  {

  uint8_t DirectionTime;
  int NewPosition = Position;

  if(Direction) {
    DirectionTime = _DownTime;
  }
  else  {
    DirectionTime = _UpTime;
  }

  float PositionChange = (float) MeasuredTime / (float) DirectionTime;
  PositionChange = PositionChange / 10;

  NewPosition += Direction == 1 ? PositionChange : -PositionChange;
  NewPosition = NewPosition > 100 ? 100 : NewPosition;
  NewPosition = NewPosition < 0 ? 0 : NewPosition;

  Position = (uint8_t)NewPosition;

  EEPROM.put(EEA_RS_POSITION, Position);
}
