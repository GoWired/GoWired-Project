/*
 * Dimmer.cpp
 */
 
#include "Dimmer.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
Dimmer::Dimmer()	{

    NewDimmingLevel = 20;
    CurrentState = false;
    _DimmingLevel = 20;
	
}

/*  *******************************************************************************************
 *                                      Set Values
 *  *******************************************************************************************/
void Dimmer::SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3, uint8_t Pin4)	{
	
	_NumberOfChannels = NumberOfChannels;
	_DimmingStep = DimmingStep;
	_DimmingInterval = DimmingInterval;

  uint8_t Channels[4] = {Pin1, Pin2, Pin3, Pin4};

  for(int i=0; i<_NumberOfChannels; i++)  {
    _Channels[i] = Channels[i];
  }
}

// Update Dimmer: New, experimental function which adjust colors and dimming level at the same time
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

/*  *******************************************************************************************
 *                                      Change Level
 *  *******************************************************************************************/
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
    if(millis() < Time)  {
      Time = millis();
    }
  }
}

/*  *******************************************************************************************
 *                                      Change Colors
 *  *******************************************************************************************/
void Dimmer::ChangeColors() {

  uint8_t Delta;
  uint32_t Time = millis();

  for(int i=0; i<_NumberOfChannels; i++) {
    Delta = (_Values[i] - NewValues[i]) > 0 ? -_DimmingStep : _DimmingStep;
    while(_Values[i] != NewValues[i]) {
		  if(millis() > Time + _DimmingInterval)	{
			  _Values[i] += Delta;
        analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
			  Time = millis();
		  }
      if(millis() < Time)  {
        Time = millis();
      }
    }
  }
}

/*  *******************************************************************************************
 *                                      Change Status
 *  *******************************************************************************************/
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

/*  *******************************************************************************************
 *                                      Update Target Values
 *  *******************************************************************************************/
void Dimmer::NewColorValues(const char *input) {
  
  if (strlen(input) == 6) {
    //Serial.println("new rgb value");
    NewValues[0] = fromhex(&input[0]);
    NewValues[1] = fromhex(&input[2]);
    NewValues[2] = fromhex(&input[4]);
    NewValues[3] = 0;
  }
  else if (strlen(input) == 8) {
    //Serial.println("new rgbw value");
    NewValues[0] = fromhex(&input[0]);
    NewValues[1] = fromhex(&input[2]);
    NewValues[2] = fromhex(&input[4]);
    NewValues[3] = fromhex(&input[6]);
  }
  else if (strlen(input) == 9) {
    //Serial.println("new rgbw value");
    NewValues[0] = fromhex(&input[1]); // ignore # as first sign
    NewValues[1] = fromhex(&input[3]);
    NewValues[2] = fromhex(&input[5]);
    NewValues[3] = fromhex(&input[7]);
  }
}

/*  *******************************************************************************************
 *                                      Hex to Byte Conversion
 *  *******************************************************************************************/
byte Dimmer::fromhex(const char *str)  {
  
  char c = str[0] - '0';
  
  if(c > 9)
    c -= 7;
  int result = c;
  c = str[1] - '0';
  if(c > 9)
    c -= 7;
  return (result << 4) | c;
}
