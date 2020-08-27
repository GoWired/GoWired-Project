/*
 * Dimmer.cpp
 */
 
#include "Dimmer.h"

/*  *******************************************************************************************
 *                                      Constructor
 *  *******************************************************************************************/
Dimmer::Dimmer()	{
	
}

/*  *******************************************************************************************
 *                                      Set Values
 *  *******************************************************************************************/
void Dimmer::SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2, uint8_t Pin3, uint8_t Pin4)	{
	
	_NumberOfChannels = NumberOfChannels;
	_DimmingStep = DimmingStep;
	_DimmingInterval = DimmingInterval;
	
	_DimmerTime = 0;
	_DimmerState = false;
	
	if(_NumberOfChannels == 3)	{
		_Channels[0] = Pin1;
		_Channels[1] = Pin2;
		_Channels[2] = Pin3;
	}
	else if(_NumberOfChannels == 4)	{
		_Channels[0] = Pin1;
		_Channels[1] = Pin2;
		_Channels[2] = Pin3;
		_Channels[3] = Pin4;
	}
}

/*  *******************************************************************************************
 *                                      Change Level
 *  *******************************************************************************************/
void Dimmer::ChangeLevel()  {

  uint8_t Delta = (NewDimmingLevel - _DimmingLevel) < 0 ? -_DimmingStep : _DimmingStep;

  while(_DimmingLevel != NewDimmingLevel) {
		if(millis() > _DimmerTime + _DimmingInterval)	{
			_DimmerTime = millis();
			_DimmingLevel += Delta;
			for(int i=0; i<_NumberOfChannels; i++)	{
				analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
			}
		}
  }
}

/*  *******************************************************************************************
 *                                      Change Colors
 *  *******************************************************************************************/
void Dimmer::ChangeColors() {

  uint8_t Delta;

  for(int i=0; i<_NumberOfChannels; i++) {
    Delta = (_Values[i] - NewValues[i]) > 0 ? -_DimmingStep : _DimmingStep;
    while(_Values[i] != NewValues[i]) {
		  if(millis() > _DimmerTime + _DimmingInterval)	{
			  _Values[i] += Delta;
        analogWrite(_Channels[i], (int)(_DimmingLevel / 100.0 * _Values[i]));
			  _DimmerTime = millis();
		  }
    }
  }
}

/*  *******************************************************************************************
 *                                      Change Status
 *  *******************************************************************************************/
void Dimmer::ChangeState() {

  //uint8_t NewLevel;

  _DimmerState = NewState;
  
  if(!NewState) {
    uint8_t TempLevel = _DimmingLevel;
    NewDimmingLevel = 0;

    ChangeLevel();

    _DimmingLevel = TempLevel;
  }
  else  {
    NewDimmingLevel = _DimmingLevel;
    _DimmingLevel = 0;

    ChangeLevel();
  }
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
