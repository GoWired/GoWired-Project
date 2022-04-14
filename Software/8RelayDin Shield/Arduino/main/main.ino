/*
 * GoWired is an open source project for WIRED home automation. It aims at making wired
 * home automation easy and affordable for every home automation enthusiast. GoWired provides
 * hardware, software, enclosures and instructions necessary to build your own bus communicating
 * smart home installation.
 * 
 * GoWired is based on RS485 industrial communication standard. The software uses MySensors
 * communication protocol (http://www.mysensors.org).
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
 * This is source code for GoWired MCU working with 8RelayDin Shield.
 * 
 */

/*  *******************************************************************************************
 *                                      Includes
 *  *******************************************************************************************/
#include "Configuration.h"
#include "ExpanderIO.h"
#include <MySensors.h>
#include <avr/wdt.h>


/*  *******************************************************************************************
 *                                      Globals
 *  *******************************************************************************************/
// Additional presentation status required by Home Assistant
bool InitConfirm = false;
uint8_t NumberOfLongpresses = NUMBER_OF_OUTPUTS;

// Module Safety Indicators
bool THERMAL_ERROR = false;                        // Thermal error status

/*  *******************************************************************************************
 *                                      Constructors
 *  *******************************************************************************************/
// Expander Input constructor
ExpanderIO EIO[TOTAL_NUMBER_OF_OUTPUTS+INDEPENDENT_IO];
MyMessage msgSTATUS(0, V_STATUS);

/*  *******************************************************************************************
                                            Before
 *  *******************************************************************************************/
void before() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif
  
}

/*  *******************************************************************************************
 *                                          Setup
 *  *******************************************************************************************/
void setup() {

  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_4S);
  #endif
  
  // This function calls Expander.begin(0x20);
  EIO[0].ExpanderInit();

  for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+INDEPENDENT_IO; i++)  {
    EIO[i].SetValues(RELAY_OFF, false, 2, i);
    EIO[i+TOTAL_NUMBER_OF_OUTPUTS].SetValues(RELAY_OFF, INVERT_INPUT_LOGIC, INPUT_TYPE, i+TOTAL_NUMBER_OF_OUTPUTS);
  }

  uint8_t j = FIRST_OUTPUT_ID + INDEPENDENT_IO;
    
  for(int i=j; i<j+NUMBER_OF_OUTPUTS; i++)  {
    EIO[i].SetValues(RELAY_OFF, INVERT_BUTTON_LOGIC, 4, i+TOTAL_NUMBER_OF_OUTPUTS, i);
  }
}

/*  *******************************************************************************************
 *                                          Presentation
 *  *******************************************************************************************/
void presentation() {

  sendSketchInfo(MN, FV);

  uint8_t Current_ID = FIRST_OUTPUT_ID;

  for(int i=Current_ID; i<Current_ID+INDEPENDENT_IO; i++)  {
    present(i, S_BINARY, "8RD Relay");  wait(PRESENTATION_DELAY);
    present(i+TOTAL_NUMBER_OF_OUTPUTS, S_BINARY, "8RD Input"); wait(PRESENTATION_DELAY);
  }

  Current_ID += INDEPENDENT_IO;

  for(int i=Current_ID; i<Current_ID+NUMBER_OF_OUTPUTS; i++)  {
    present(i, S_BINARY, "8RD B+R");  wait(PRESENTATION_DELAY);
  }

  if(INPUT_TYPE == 3) {
    NumberOfLongpresses += INDEPENDENT_IO;
  }

  Current_ID = TOTAL_NUMBER_OF_OUTPUTS + INDEPENDENT_IO;

  for(int i=Current_ID; i<Current_ID+NumberOfLongpresses; i++)  {
    present(i, S_BINARY, "Longpress"); wait(PRESENTATION_DELAY);
  }    
}

/*  *******************************************************************************************
                                            Init Confirmation
 *  *******************************************************************************************/
void InitConfirmation() {

  uint8_t SensorsToConfirm = TOTAL_NUMBER_OF_OUTPUTS+INDEPENDENT_IO;

  for(int i=FIRST_OUTPUT_ID; i<FIRST_OUTPUT_ID+SensorsToConfirm; i++)  {
    send(msgSTATUS.setSensor(i).set(EIO[i].NewState));
    request(i, V_STATUS);
    wait(1000, C_SET, V_STATUS);
  }

  uint8_t FirstLongpressID = FIRST_OUTPUT_ID+SensorsToConfirm;

  for(int i=FirstLongpressID; i<FirstLongpressID+NumberOfLongpresses; i++)  {
    send(msgSTATUS.setSensor(i).set("0"));
    request(i, V_STATUS);
    wait(1000, C_SET, V_STATUS);
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

  for(int i=FirstSensor; i<FirstSensor+NumberOfSensors; i++)  {
    EIO[i].CheckInput();
    if(EIO[i].NewState != EIO[i].State)  {
      switch(EIO[i].SensorType)  {
        case 0:
          // Door/window/button
        case 1:
          // Motion sensor
          send(msgSTATUS.setSensor(i).set(EIO[i].NewState));
          EIO[i].State = EIO[i].NewState;
          break;
        case 2:
          // Relay output
          // Nothing to do here
          break;
        case 3:
          // Button input
          if(EIO[i].NewState != 2)  {
            send(msgSTATUS.setSensor(i).set(EIO[i].NewState));
            EIO[i].State = EIO[i].NewState;
          }
          #ifdef SPECIAL_BUTTON
            else if(EIO[i].NewState == 2)  {
              send(msgSTATUS.setSensor(i + TOTAL_NUMBER_OF_OUTPUTS).set(true)); 
              EIO[i].NewState = EIO[i].State;
            }
          #endif
          break;
        case 4:
          // Button input + Relay output
          if(EIO[i].NewState != 2)  {
            if(!THERMAL_ERROR)  {
              EIO[i].SetRelay();
              send(msgSTATUS.setSensor(i).set(EIO[i].NewState));
            }
          }
          #ifdef SPECIAL_BUTTON
            else if(EIO[i].NewState == 2)  {
              send(msgSTATUS.setSensor(i + TOTAL_NUMBER_OF_OUTPUTS).set(true));
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

  if(INDEPENDENT_IO > 0)  {
    IOUpdate(TOTAL_NUMBER_OF_OUTPUTS, INDEPENDENT_IO);
    if(INDEPENDENT_IO < TOTAL_NUMBER_OF_OUTPUTS)  {
      IOUpdate(INDEPENDENT_IO, NUMBER_OF_OUTPUTS);
    }
  }
  else  {
    IOUpdate(FIRST_OUTPUT_ID, NUMBER_OF_OUTPUTS);
  }

  wait(LOOP_TIME);

}
