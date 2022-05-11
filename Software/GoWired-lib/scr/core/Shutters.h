/**
 * @file Shutters.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief class controls shutter devices of any kind; opening, closing, percentage control
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef Shutters_h
#define Shutters_h

#include "Arduino.h"
#include <EEPROM.h>

class Shutters
{
  public:
    Shutters(uint16_t AddressDownTime, uint16_t AddressUpTime, uint16_t AddressPosition);

    uint8_t Position;
    uint8_t State;              // 0 - moving upward, 1- moving downward, 2 - stopped
    uint8_t NewState;           // 0 - move up, 1- move down, 2 - stop
    bool Calibrated;

    void Calibration(uint8_t UpTime=0, uint8_t DownTime=0);     // Finds up and down time by measuring current, save values to eeprom for use after power failure
    void SetOutputs(bool RelayOff, uint8_t UpPin, uint8_t DownPin);
    uint32_t ReadMessage(uint8_t Order);
    uint32_t ReadButtons(uint8_t Button);
    uint32_t ReadNewPosition(int NewPosition);
    uint8_t Movement();
    void CalculatePosition(bool Direction, uint32_t MeasuredTime);

  private:
    bool _RelayOff;
    bool _RelayOn;
    uint8_t _UpPin;
    uint8_t _DownPin;
    uint8_t _UpTime;
    uint8_t _DownTime;
        
};

#endif
