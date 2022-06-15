/**
 * @file Shutters.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Shutters.h"

/**
 * @brief Construct a new Shutters:: Shutters object
 * 
 */
Shutters::Shutters(uint16_t AddressDownTime, uint16_t AddressUpTime, uint16_t AddressPosition)  {

  NewState = 2;
  State = 2;

  uint8_t DownTime;
  uint8_t UpTime;

  EEPROM.get(AddressDownTime, DownTime);
  EEPROM.get(AddressUpTime, UpTime);
  
  if(UpTime != 255 && DownTime != 255)  {
    Calibrated = true;
    _UpTime = UpTime;
    _DownTime = DownTime;
    EEPROM.get(AddressPosition, Position);
  }
  else  {
    Calibrated = false;
  }
}

/**
 * @brief Pass parameters to new Shutters object
 * 
 * @param UpPin uC output pin responsible for upward direction
 * @param DownPin uC output pin responsible for downward direction
 * @param RelayOff boolean value which puts connected relays to off state
 */
void Shutters::SetOutputs(bool RelayOff, uint8_t UpPin, uint8_t DownPin)  {

  _DownPin = DownPin;
  _UpPin = UpPin;
  _RelayOff = RelayOff;
  _RelayOn = !RelayOff;

  pinMode(_UpPin, OUTPUT);  digitalWrite(_UpPin, RelayOff);
  pinMode(_DownPin, OUTPUT);  digitalWrite(_DownPin, RelayOff);
}

/**
 * @brief Saves movement times obtained from calibration to EEPROM
 * 
 * @param UpTime duration of upward movement
 * @param DownTime duration of downward movement
 */
void Shutters::Calibration(uint8_t UpTime, uint8_t DownTime)  {

  _UpTime = UpTime;
  _DownTime = DownTime;
}

/**
 * @brief Reads messages to obtain orders from controller
 * 
 * @param Order message payload
 * @return uint32_t movement time for a given direction
 */
uint32_t Shutters::ReadMessage(uint8_t Order) {

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

/**
 * @brief Read orders from buttons
 * 
 * @param Button button number (0 or 1)
 * @return uint32_t movement time for a given direction
 */
uint32_t Shutters::ReadButtons(uint8_t Button)  {

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

/**
 * @brief Reads position message from controller
 * 
 * @param NewPosition new position sent by controller
 * @return uint32_t movement time needed to move the shutter to new position
 */
uint32_t Shutters::ReadNewPosition(int NewPosition) {

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

/**
 * @brief Realizes movement of the shutter
 * 
 * @return uint8_t current state of the shutter (moving up/moving down/is stopped)
 */
uint8_t Shutters::Movement()  {

  // Stop roller shutter
  if(NewState == 2) {
    digitalWrite(_UpPin, _RelayOff);
    digitalWrite(_DownPin, _RelayOff);
  }
  // Move upward
  else if(NewState == 0)  {
    digitalWrite(_DownPin, _RelayOff);
	delay(20);
    digitalWrite(_UpPin, _RelayOn);
  }
  // Move downward
  else if(NewState == 1) {
    digitalWrite(_UpPin, _RelayOff);
	delay(20);
    digitalWrite(_DownPin, _RelayOn);
  }

  State = NewState;
  return State;  
}

/**
 * @brief Calculates new position after movement
 * 
 * @param Direction direction of movement
 * @param MeasuredTime duration of movement
 */
void Shutters::CalculatePosition(bool Direction, uint32_t MeasuredTime)  {

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
}

