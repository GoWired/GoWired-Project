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

  void SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3, uint8_t Pin4=0);
  void ChangeLevel();
  void ChangeColors();
  void ChangeState();
  void NewColorValues(const char *input);

  bool NewState = 0;
  uint8_t NewDimmingLevel = 0;
  uint8_t NewValues[4] = {255, 255, 255, 255};

  private:
  bool _DimmerState;
  uint8_t _NumberOfChannels;
  uint8_t _DimmingStep;
  uint8_t _DimmingInterval;
  uint8_t _Channels[4];
  uint8_t _Values[4] = {255, 255, 255, 255};
  uint8_t _DimmingLevel;
  
  //uint32_t _DimmerTime;

  byte fromhex(const char *str);
  
};


#endif
