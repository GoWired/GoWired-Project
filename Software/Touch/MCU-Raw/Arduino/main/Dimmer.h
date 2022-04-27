/*
 * Dimmer.h
 */

#ifndef Dimmer_h
#define Dimmer_h

#include "Arduino.h"

class Dimmer
{
  public:
    Dimmer();

    void SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2=0, uint8_t Pin3=0, uint8_t Pin4=0);
    void UpdateLEDs(uint8_t DimmingLevel, uint8_t R, uint8_t G=0, uint8_t B=0, uint8_t W=0);
    
  private:
    uint8_t _NumberOfChannels;
    uint8_t _Channels[4];

};

#endif