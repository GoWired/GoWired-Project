/*
 * 
 */


#ifndef RShutterControl_h
#define RShutterControl_h

#include "Arduino.h"
#include <EEPROM.h>
#include "Configuration.h"
#include "PowerSensor.h"
#include "UniversalInput.h"
//#include <core/MySensorsCore.h>

#ifndef PS_OFFSET
#define PS_OFFSET 0.2
#endif

#ifndef CALIBRATION_SAMPLES
#define CALIBRATION_SAMPLES 2
#endif

class RShutterControl
{
  public:
    RShutterControl(int UpPin, int DownPi);

    uint8_t Position;

    void Calibration();     // Find up and down time by measuring current, save values to eeprom for use even after power failure
    uint8_t Move(bool Direction);
    void Stop();

  private:
    uint8_t _UpPin;
    uint8_t _DownPin;
    uint8_t _UpTime;
    uint8_t _DownTime;

    PowerSensor PS;
        
};

#endif
