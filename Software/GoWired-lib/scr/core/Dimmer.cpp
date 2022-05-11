/**
 * @file Dimmer.cpp
 * @author feanor-anglin (hello@gowired.dev)
 * @brief 
 * @version 0.1
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
 
#include "Dimmer.h"

/**
 * @brief Construct a new Dimmer:: Dimmer object
 * 
 */
Dimmer::Dimmer()	{

    NewDimmingLevel = 20;
    CurrentState = false;
    _DimmingLevel = 20;
	
}

/**
 * @brief passes parameters to a new Dimmer object
 * 
 * @param NumberOfChannels number of dimmer channels (1-one colour, 3-RGB or 4-RGBW)
 * @param DimmingStep single step to increase/decrease dimming value in a time step
 * @param DimmingInterval time to wait between consecutive dimming steps [ms]
 * @param Pin1 pin of first channel
 * @param Pin2 pin of second channel
 * @param Pin3 pin of third channel
 * @param Pin4 pin of fourth channel
 */
void Dimmer::SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3, uint8_t Pin4)	{
	
	_NumberOfChannels = NumberOfChannels;
	_DimmingStep = DimmingStep;
	_DimmingInterval = DimmingInterval;
	
	uint8_t Channels[4] = {Pin1, Pin2, Pin3, Pin4};

  for(int i=0; i<_NumberOfChannels; i++)  {
    _Channels[i] = Channels[i];
  }
}

/**
 * @brief Sipmle LEDs update without transition effect
 * 
 * @param DimmingLevel new dimming value
 * @param R new red value
 * @param G new green value
 * @param B new blue value
 * @param W new white value
 */
void Dimmer::UpdateLEDs(uint8_t DimmingLevel, uint8_t R, uint8_t G, uint8_t B, uint8_t W)  {

  uint8_t RGBValues[4] = {R, G, B, W};

  for(int i=0; i<_NumberOfChannels; i++)  {
    analogWrite(_Channels[i], (int)(DimmingLevel / 100.0 * RGBValues[i]));
  }
}

/**
 * @brief updates dimming level and colour values of dimmer object with transition effect
 * 
 */
void Dimmer::UpdateDimmer()	{
	
	bool AdjustDimming = true;
	bool AdjustColors = true;
	bool ColorStates[4] = {false, false, false, false};
	uint8_t DeltaDimming = 0;
	uint8_t DeltaColors = 0;
	uint32_t Time = millis() - _DimmingInterval;
	
  if(CurrentState) {
	do	{
      if(millis() > Time + _DimmingInterval)	{
		if(_DimmingLevel != NewDimmingLevel)	{
			AdjustDimming = true;
			DeltaDimming = (NewDimmingLevel - _DimmingLevel) < 0 ? -_DimmingStep : _DimmingStep;
			_DimmingLevel += DeltaDimming;
		}
		else	{
			AdjustDimming = false;
		}
	
		for(int i=0; i<_NumberOfChannels; i++) {
			if(_Values[i] != NewValues[i])	{
				ColorStates[i] = true;
				DeltaColors = (_Values[i] - NewValues[i]) > 0 ? -_DimmingStep : _DimmingStep;
				_Values[i] += DeltaColors;
			}
			else	{
				ColorStates[i] = false;
			}
		}
		
		for(int i=0; i<_NumberOfChannels; i++) {
			if(ColorStates[i] == true)	{
				AdjustColors = true;
				break;
			}
            else  {
                AdjustColors = false;
            }
		}
		
		for(int i=0; i<_NumberOfChannels; i++) {
			analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
		}

        Time = millis();
      }
      else if(millis() < Time) {
        Time = millis();
      }
	} while(AdjustColors || AdjustDimming);
  }
}

/**
 * @brief changes colour values without transition effect
 * 
 */
void Dimmer::ChangeValuesOffline()  {

  for(int i=0; i<_NumberOfChannels; i++) {
    _Values[i] = NewValues[i];
  }
}

/**
 * @brief changes dimming level with transition effect
 * 
 */
void Dimmer::ChangeLevel()  {

  uint8_t Delta = (NewDimmingLevel - _DimmingLevel) < 0 ? -_DimmingStep : _DimmingStep;
  uint32_t Time = millis();

  while(_DimmingLevel != NewDimmingLevel) {
		if(millis() > Time + _DimmingInterval)	{
			Time = millis();
			_DimmingLevel += Delta;
			for(int i=0; i<_NumberOfChannels; i++)	{
				analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
			}
		}
    else if(millis() < Time)  {
      Time = millis();
    }
  }
}

/**
 * @brief changes colour values with transition effect
 * 
 */
void Dimmer::ChangeColors() {

  uint8_t Delta;
  uint32_t Time = millis();

  for(int i=0; i<_NumberOfChannels; i++) {
    Delta = (_Values[i] - NewValues[i]) > 0 ? -_DimmingStep : _DimmingStep;
    while(_Values[i] != NewValues[i]) {
		  if(millis() > Time + _DimmingInterval)	{
        Time = millis();
			  _Values[i] += Delta;
        analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
		  }
      else if(millis() < Time)  {
        Time = millis();
      }
    }
  }
}

/**
 * @brief changes dimmer status according to the parameter with transition effect
 * 
 * @param NewState new state of a dimmer
 */
void Dimmer::ChangeState(bool NewState) {

  uint8_t TempDimmingLevel = NewDimmingLevel;

  if(NewState != CurrentState)  {
    CurrentState = NewState;
  
    if(!NewState) {
      uint8_t TempLevel = _DimmingLevel;
      NewDimmingLevel = 0;

      ChangeLevel();

      _DimmingLevel = TempLevel;
    }
    else  {
      //NewDimmingLevel = _DimmingLevel;
      _DimmingLevel = 0;

      ChangeLevel();
    }
  }
  NewDimmingLevel = TempDimmingLevel;
}

/**
 * @brief applies new colour values to be changed by other functions
 * 
 * @param input string of chars representing RGB/RGBW colour values 
 */
void Dimmer::NewColorValues(const char *input) {
  
  for(int i=0; i<_NumberOfChannels; i++) {
    if(strlen(input) == 6 || strlen(input) == 8)    {
        NewValues[i] = _StringHexToByte(&input[2*i]);
    }
    else if(strlen(input) == 9) {
        // Ignore # as first sign
        NewValues[i] = _StringHexToByte(&input[2*i+1]);
    }
  }
}

/**
 * @brief decodes 2 char signs to a byte value
 * 
 * @param str input char value
 * @return byte output value
 */
byte Dimmer::_StringHexToByte(const char *str)  {
  
  char c = str[0] - '0';
  
  if(c > 9)
    c -= 7;
  int result = c;

  c = str[1] - '0';
  if(c > 9)
    c -= 7;
  return (result << 4) | c;
}
