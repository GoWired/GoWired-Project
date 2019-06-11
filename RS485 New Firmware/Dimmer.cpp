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
void Dimmer::SetValues(int Pin1, int Pin2, int Pin3, int Pin4)	{
	
	if(NUMBER_OF_CHANNELS ==1)	{
		Channels[0] = Pin1;
		Values[0] = 255;
	}
	else if(NUMBER_OF_CHANNELS == 3)	{
		Channels[0] = Pin1;
		Channels[1] = Pin2;
		Channels[2] = Pin3;
	}
	else if(NUMBER_OF_CHANNELS == 4)	{
		Channels[0] = Pin1;
		Channels[1] = Pin2;
		Channels[2] = Pin3;
		Channels[3] = Pin4;
	}
}

/*  *******************************************************************************************
 *                                      Change Level
 *  *******************************************************************************************/
void Dimmer::ChangeLevel(int NewLevel)  {

  int Delta = (NewLevel - DimmingLevel) < 0 ? -DIMMING_STEP : DIMMING_STEP;

  while (DimmingLevel != NewLevel) {
		if(millis() > DimmerTime + DIMMING_INTERVAL)	{
			DimmingLevel += Delta;
			for(int i=0; i<NUMBER_OF_CHANNELS; i++)	{
				analogWrite(Channels[i], (int)(DimmingLevel / 100. * Values[i]));
			}
		}
  }
}

/*  *******************************************************************************************
 *                                      Change Colors
 *  *******************************************************************************************/
void Dimmer::ChangeColors() {

  int Delta;

  for(int i=0; i<NUMBER_OF_CHANNELS; i++) {
    Delta = (Values[i] - NewValues[i]) > 0 ? -DIMMING_STEP : DIMMING_STEP;
    while(Values[i] != NewValues[i]) {
      if(millis() > DimmerTime + DIMMING_INTERVAL)	{
			  Values[i] += Delta;
			  analogWrite(Channels[i], (int)(DimmingLevel / 100.0 * Values[i]));
		  }
    }
  }
}

/*  *******************************************************************************************
 *                                      Change Status
 *  *******************************************************************************************/
void Dimmer::ChangeStatus(bool NewStatus) {

  int NewLevel;

  DimmerStatus = NewStatus;
  
  if(!NewStatus) {
    int TempLevel = DimmingLevel;
    NewLevel = 0;

    ChangeLevel(NewLevel);

    DimmingLevel = TempLevel;
  }
  else  {
    NewLevel = DimmingLevel;
    DimmingLevel = 0;

    ChangeLevel(NewLevel);
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
  } else if (strlen(input) == 9) {
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
