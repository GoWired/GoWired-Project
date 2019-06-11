/*
 *  
 */

#ifndef Dimmer_h
#define Dimmer_h

#include "Arduino.h"

#ifndef NUMBER_OF_CHANNELS
#define NUMBER_OF_CHANNELS 3
#endif

#ifndef DIMMING_STEP
#define DIMMING_STEP 1   // This value needs to be carefully tested!
#endif

#ifndef DIMMING_INTERVAL
#define DIMMING_INTERVAL 20               // ACS7125A: 185 mV/A; ACS71220A: 100 mV/A
#endif

class Dimmer
{
  public:
  Dimmer();      

  void SetValues(int Pin1, int Pin2=0, int Pin3=0, int Pin4=0);
  void ChangeLevel(int NewLevel);
  void ChangeColors();
  void ChangeStatus(bool NewStatus);
  void NewColorValues(const char *input);

  bool DimmerStatus;

  private:
  uint8_t Channels[NUMBER_OF_CHANNELS];
  uint8_t Values[NUMBER_OF_CHANNELS];
  uint8_t NewValues[NUMBER_OF_CHANNELS];
  uint8_t DimmingLevel;
  
  uint32_t DimmerTime;

  byte fromhex(const char *str);
  
};


#endif
