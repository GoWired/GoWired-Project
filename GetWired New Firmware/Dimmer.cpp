/*
 *
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
void Dimmer::SetValues(uint8_t NumberOfChannels, uint8_t DimmingStep, uint8_t DimmingInterval, uint8_t Pin1, uint8_t Pin2=0, uint8_t Pin3=0, uint8_t Pin4=0)	{
	
	_NumberOfChannels = NumberOfChannels;
	_DimmingStep = DimmingStep;
	_DimmingInterval = DimmingInterval;
	
	_DimmerTime = 0;
	DimmerStatus = false;
	
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
void Dimmer::ChangeLevel(uint8_t NewLevel)  {

  uint8_t Delta = (NewLevel - _DimmingLevel) < 0 ? -_DimmingStep : _DimmingStep;

  while(_DimmingLevel != NewLevel) {
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
    Delta = (_Values[i] - _NewValues[i]) > 0 ? -_DimmingStep : _DimmingStep;
    while(_Values[i] != _NewValues[i]) {
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
void Dimmer::ChangeStatus(bool NewStatus) {

  uint8_t NewLevel;

  DimmerStatus = NewStatus;
  
  if(!NewStatus) {
    uint8_t TempLevel = _DimmingLevel;
    NewLevel = 0;

    ChangeLevel(NewLevel);

    _DimmingLevel = TempLevel;
  }
  else  {
    NewLevel = _DimmingLevel;
    _DimmingLevel = 0;

    ChangeLevel(NewLevel);
  }
}

/*  *******************************************************************************************
 *                                      Update Target Values
 *  *******************************************************************************************/
void Dimmer::NewColorValues(const char *input) {
  
  if (strlen(input) == 6) {
    //Serial.println("new rgb value");
    _NewValues[0] = fromhex(&input[0]);
    _NewValues[1] = fromhex(&input[2]);
    _NewValues[2] = fromhex(&input[4]);
    _NewValues[3] = 0;
  } else if (strlen(input) == 9) {
    //Serial.println("new rgbw value");
    _NewValues[0] = fromhex(&input[1]); // ignore # as first sign
    _NewValues[1] = fromhex(&input[3]);
    _NewValues[2] = fromhex(&input[5]);
    _NewValues[3] = fromhex(&input[7]);
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