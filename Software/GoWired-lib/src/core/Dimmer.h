/**
 * @file Dimmer.h
 * @author feanor-anglin (hello@gowired.dev)
 * @brief class uses uC PWM outputs to control dimmable DC lighting
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
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
  void UpdateDimmer();
  void ChangeValuesOffline();
  void ChangeLevel();
  void ChangeColors();
  void ChangeState(bool NewState);
  void NewColorValues(const char *input);

  bool CurrentState;
  uint8_t NewDimmingLevel;
  uint8_t NewValues[4] = {255, 255, 255, 255};

  private:
  uint8_t _NumberOfChannels;
  uint8_t _DimmingStep;
  uint8_t _DimmingInterval;
  uint8_t _Channels[4];
  uint8_t _Values[4] = {0, 0, 0, 0};
  uint8_t _DimmingLevel;

  byte _StringHexToByte(const char *str);
  
};


#endif