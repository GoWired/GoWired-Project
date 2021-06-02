/*
 * RShutterControl.h
 */


#ifndef RShutterControl_h
#define RShutterControl_h

#include "Arduino.h"
#include <EEPROM.h>
#include "Configuration.h"

class RShutterControl
{
  public:
    RShutterControl(uint8_t UpPin, uint8_t DownPin, bool RelayOn, bool RelayOff);

    int Position;
    bool Calibrated;

    void Calibration(uint8_t UpTime=0, uint8_t DownTime=0);     // Finds up and down time by measuring current, save values to eeprom for use even after power failure
    int Move(int Direction);
    void Stop();

  private:
    uint8_t _UpPin;
    uint8_t _DownPin;
    uint8_t _UpTime;
    uint8_t _DownTime;
    bool _RelayOn;
    bool _RelayOff;
        
};

#endif