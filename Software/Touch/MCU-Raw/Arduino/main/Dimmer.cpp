/*
 * Dimmer.cpp
 */
 
#include "Dimmer.h"

// Constructor
Dimmer::Dimmer()	{
	
}

// Initializing function
void Dimmer::SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3, uint8_t Pin4)	{
	
	_NumberOfChannels = NumberOfChannels;

  uint8_t Channels[4] = {Pin1, Pin2, Pin3, Pin4};

  for(int i=0; i<_NumberOfChannels; i++)  {
    _Channels[i] = Channels[i];
  }
}

// Simple LEDs update without transition effect
void Dimmer::UpdateLEDs(uint8_t DimmingLevel, uint8_t R, uint8_t G, uint8_t B, uint8_t W)  {

  uint8_t RGBValues[4] = {R, G, B, W};

  for(int i=0; i<_NumberOfChannels; i++)  {
    analogWrite(_Channels[i], (int)(DimmingLevel / 100.0 * RGBValues[i]));
  }
}
