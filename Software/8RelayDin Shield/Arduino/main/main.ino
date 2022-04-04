/*
 * GetWired is an open source project for WIRED home automation. It aims at making wired
 * home automation easy and affordable for every home automation enthusiast. GetWired provides:
 * - hardware (https://www.crowdsupply.com/domatic/getwired),
 * - software (https://github.com/feanor-anglin/GetWired-Project), 
 * - 3D printable enclosures (https://github.com/feanor-anglin/GetWired-Project/tree/master/Enclosures),
 * - instructions (both campaign page / campaign updates and our GitHub wiki).
 * 
 * GetWired is based on RS485 industrial communication standard. The software is an implementation
 * of MySensors communication protocol (http://www.mysensors.org). 
 *
 * Created by feanor-anglin
 * Copyright (C) 2018-2022 feanor-anglin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 * This software is designed for GoWired MCU working with GoWired 8RelayDin board with 8 relays
 * and 8 digital inputs.
 * 
 * To define some important variables, look at Configuration.h
 * 
 */

/*  *******************************************************************************************
 *                                      Includes
 *  *******************************************************************************************/
#include "Configuration.h"
#include "ExpanderIO.h"
#include <MySensors.h>


/*  *******************************************************************************************
 *                                      Globals
 *  *******************************************************************************************/
// Additional presentation status required by Home Assistant
bool InitConfirm = false;

// Module Safety Indicators
bool THERMAL_ERROR = false;                        // Thermal error status

/*  *******************************************************************************************
 *                                      Constructors
 *  *******************************************************************************************/
// Expander Input constructor
ExpanderIO EIO[TOTAL_NUMBER_OF_OUTPUTS+INDEPENDENT_IO];
MyMessage msgEIO(0, V_STATUS);

// Module Safety Indicators
MyMessage msgSI(0, V_STATUS);

/*  *******************************************************************************************
                                            Before
 *  *******************************************************************************************/
void before() {

  uint32_t InitDelay = MY_NODE_ID * INIT_DELAY;
  
  wait(InitDelay);
}

/*  *******************************************************************************************
 *                                          Setup
 *  *******************************************************************************************/
void setup() {
  
  // This function calls Expander.begin(0x20);
  EIO[0].ExpanderInit();

  for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+INDEPENDENT_IO; i++)  {
    EIO[i].SetValues(RELAY_OFF, 2, i);
    EIO[i+8].SetValues(RELAY_OFF, INPUT_TYPE, i+8);
  }

  uint8_t j = FIRST_OUTPUT_ID + INDEPENDENT_IO;
    
  for(int i=j; i<j+NUMBER_OF_OUTPUTS; i++)  {
    EIO[i].SetValues(RELAY_OFF, 4, i+8, i);
  }
}

/*  *******************************************************************************************
 *                                          Presentation
 *  *******************************************************************************************/
void presentation() {

  sendSketchInfo(SN, SV);

  uint8_t Current_ID = 0;

  for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+INDEPENDENT_IO; i++)  {
    present(i, S_BINARY, "8RelayDin Relay");  wait(PRESENTATION_DELAY);
    present(i+8, S_BINARY, "8RelayDin Button"); wait(PRESENTATION_DELAY);
  }

  Current_ID = FIRST_OUTPUT_ID+INDEPENDENT_IO;

  for(int i=Current_ID; i<Current_ID+NUMBER_OF_OUTPUTS; i++)  {
    present(i, S_BINARY, "8RelayDin B+R");  wait(PRESENTATION_DELAY);
  }

  Current_ID = INDEPENDENT_IO + 8;

  for(int i=Current_ID; i<Current_ID+NUMBER_OF_OUTPUTS; i++)  {
    present(i, S_BINARY, "Special Button"); wait(PRESENTATION_DELAY);
  }
}

/*  *******************************************************************************************
                                            Init Confirmation
 *  *******************************************************************************************/
void InitConfirmation() {

  for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+TOTAL_NUMBER_OF_OUTPUTS; i++)  {
    send(msgEIO.setSensor(i).set(EIO[i].NewState));
    request(i, V_STATUS);
    wait(200, C_SET, V_STATUS); //wait(2000, C_SET, V_STATUS);
  }
    
  InitConfirm = true;
  
}

/*  *******************************************************************************************
 *                                      MySensors Receive
 *  *******************************************************************************************/
void receive(const MyMessage &message)  {

  if(message.type == V_STATUS)  {
    for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+TOTAL_NUMBER_OF_OUTPUTS; i++)  {
      if(message.sensor == i)  {
        EIO[i].NewState = message.getBool();
        EIO[i].SetRelay();
      }
    }
  }
}

/*  *******************************************************************************************
                                        Universal Input
 *  *******************************************************************************************/
void IOUpdate(uint8_t FirstSensor, uint8_t NumberOfSensors) {

  uint8_t FirstSpecialButtonID = INDEPENDENT_IO + 8;

  for(int i=FirstSensor; i<FirstSensor+NumberOfSensors; i++)  {
    EIO[i].CheckInput();
    if(EIO[i].NewState != EIO[i].State)  {
      switch(EIO[i].SensorType)  {
        case 0:
          // Door/window/button
        case 1:
          // Motion sensor
          send(msgEIO.setSensor(i).set(EIO[i].NewState));
          EIO[i].State = EIO[i].NewState;
          break;
        case 2:
          // Relay output
          // Nothing to do here
          break;
        case 3:
          // Button input
          if(EIO[i].NewState != 2)  {
            if(!THERMAL_ERROR)  {
              send(msgEIO.setSensor(i).set(EIO[i].NewState));
              EIO[i].State = EIO[i].NewState;
            }
          }
          #ifdef SPECIAL_BUTTON
            else if(EIO[i].NewState == 2)  {
              send(msgEIO.setSensor(FirstSpecialButtonID + i).set(true));
              EIO[i].NewState = EIO[i].State;
            }
          #endif
          break;
        case 4:
          // Button input + Relay output
          if(EIO[i].NewState != 2)  {
            if(!THERMAL_ERROR)  {
              EIO[i].SetRelay();
              send(msgEIO.setSensor(i).set(EIO[i].NewState));
            }
          }
          #ifdef SPECIAL_BUTTON
            else if(EIO[i].NewState == 2)  {
              send(msgEIO.setSensor(FirstSpecialButtonID + i).set(true));
              EIO[i].NewState = EIO[i].State;
            }
          #endif
          break;
        default:
          // Nothing to do here
          break;
      }
    }
  }
}

void loop() {

  // Extended presentation as required by Home Assistant; runs only after startup
  if(!InitConfirm)  {
    InitConfirmation();
  }

  IOUpdate(FIRST_OUTPUT_ID, NUMBER_OF_OUTPUTS);

  if(INDEPENDENT_IO > 0)  {
    IOUpdate(8, INDEPENDENT_IO);
  }

/*
  #ifdef INTERNAL_TEMP
    // Safety check
    THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT());

    // Handling safety procedures
    if(THERMAL_ERROR == true && InformControllerTS == false) {
      // Board temperature to hot
      // Turn off relays/triacs
      HeatingStatus = false;
      for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
        Expander.digitalWrite(i, RELAY_OFF);
        send(msgH1.setSensor(i).set(RELAY_OFF));
      }
      send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = true;
    }
    else if(THERMAL_ERROR == false && InformControllerTS == true) {
      HeatingStatus = true;
      send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = false;
    }

    // Update internal temperature value to the controller
    if(millis() > LastUpdate + INTERVAL)  {
      ITUpdate();
      LastUpdate = millis();
    }
  #endif*/

  wait(LOOP_TIME);

}
