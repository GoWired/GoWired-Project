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
 * Copyright (C) 2018-2020 feanor-anglin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0.0 - feanor-anglin
 *
 * DESCRIPTION version 1.0.0
 * This is code for GetWired MCU Module which is designed to work with many different inputs 
 * (buttons, digital and analog sensors) and a certain kind of output (depending on what shield is 
 * used). For the list of available shields have a look here: 
 * https://github.com/feanor-anglin/GetWired-Project/wiki/GetWired-Hardware.
 * 
 * Hardware serial is used with baud rate of 57600 by default.
 * 
 * 
 */

/*  *******************************************************************************************
                                        Includes
 *  *******************************************************************************************/
#include "PowerSensor.h"
#include "InternalTemp.h"
#include "UniversalInput.h"
#include "Dimmer.h"
#include "RShutterControl.h"
#include "Configuration.h"
#include <MySensors.h>
#include <dht.h>
#include <Wire.h>
#include "SHTSensor.h"

/*  *******************************************************************************************
                                        Globals
 *  *******************************************************************************************/
// RShutter
int NewPosition;

// Dimmer
int CurrentLevel = 0;
bool DimmerStatus = false;

// Timer
unsigned long LastUpdate = 0;               // Time of last update of interval sensors
bool CheckNow = false;

// Module Safety Indicators
bool THERMAL_ERROR = false;                 // Thermal error status
bool InformControllerTS = false;            // Was controller informed about error?
bool OVERCURRENT_ERROR[4] = {false, false, false, false};             // Overcurrent error status
bool InformControllerES = false;            // Was controller informed about error?
int ET_ERROR = 3;                           // External thermometer status (0 - ok, 1 - checksum error, 2 - timeout error, 3 - default/initialization)

// Initialization
bool InitConfirm = false;

/*  *******************************************************************************************
                                        Constructors
 *  *******************************************************************************************/
//Universal input constructor
#if (UI_SENSORS_NUMBER > 0)
  UniversalInput UI[UI_SENSORS_NUMBER];
  MyMessage msgUI(0, V_LIGHT);
#endif

// Dimmer
#if defined(DIMMER) || defined(RGB) || defined(RGBW)
  Dimmer Dimmer;
  MyMessage msgDIM(DIMMER_ID, V_PERCENTAGE);
  MyMessage msgDIM2(DIMMER_ID, V_RGB);
  MyMessage msgDIM3(DIMMER_ID, V_RGBW);
#endif

// Power sensor constructor
#if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
  PowerSensor PS;
  MyMessage msgPS(PS_ID, V_WATT);
#elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
  PowerSensor PS[NUMBER_OF_RELAYS];
  MyMessage msgPS(0, V_WATT);
#endif

// RShutter Control Constructor
#ifdef ROLLER_SHUTTER
  RShutterControl RS(RELAY_1, RELAY_2, RELAY_ON, RELAY_OFF);
  MyMessage msgRS1(RS_ID, V_UP);
  MyMessage msgRS2(RS_ID, V_DOWN);
  MyMessage msgRS3(RS_ID, V_STOP);
  MyMessage msgRS4(RS_ID, V_PERCENTAGE);
#endif

// Internal thermometer constructor
#ifdef INTERNAL_TEMP
  InternalTemp IT(IT_PIN, MAX_TEMPERATURE, MVPERC, ZEROVOLTAGE);
  MyMessage msgIT(IT_ID, V_TEMP);
#endif

// External thermometer constructor
#ifdef EXTERNAL_TEMP
  #ifdef DHT22
    dht DHT;
  #endif
  #ifdef SHT30
    SHTSensor sht;
  #endif
  MyMessage msgETT(ETT_ID, V_TEMP);
  MyMessage msgETH(ETH_ID, V_HUM);
#endif

// Error Reporting
#ifdef ERROR_REPORTING
  MyMessage msgSI(0, V_STATUS);
#endif

#ifdef RS485_DEBUG
  MyMessage msgDEBUG(DEBUG_ID, V_TEXT);
  MyMessage msgDEBUG2(DEBUG_ID, V_WATT);
#endif

/*  *******************************************************************************************
                                            Before
 *  *******************************************************************************************/
void before() {

  uint32_t InitDelay = MY_NODE_ID * INIT_DELAY;
  
  wait(InitDelay);
}

/*  *******************************************************************************************
                                            Setup
 *  *******************************************************************************************/
void setup() {

  float Vcc = ReadVcc();  // mV

  // OUTPUT
  #ifdef DOUBLE_RELAY
    UI[RELAY_ID_1].SetValues(RELAY_OFF, 4, BUTTON_1, RELAY_1);
    UI[RELAY_ID_2].SetValues(RELAY_OFF, 4, BUTTON_2, RELAY_2);
  #endif

  #ifdef ROLLER_SHUTTER
    UI[RS_ID].SetValues(RELAY_OFF, 3, BUTTON_1);
    UI[RS_ID + 1].SetValues(RELAY_OFF, 3, BUTTON_2);
  #endif

  #ifdef FOUR_RELAY
    UI[RELAY_ID_1].SetValues(RELAY_OFF, 2, RELAY_1);
    UI[RELAY_ID_2].SetValues(RELAY_OFF, 2, RELAY_2);
    UI[RELAY_ID_3].SetValues(RELAY_OFF, 2, RELAY_3);
    UI[RELAY_ID_4].SetValues(RELAY_OFF, 2, RELAY_4);
  #endif

  #ifdef DIMMER
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4);
  #endif

  #ifdef RGB
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3);
  #endif

  #ifdef RGBW
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4);
  #endif

  // INPUT
  #ifdef INPUT_1
    #ifdef PULLUP_1
      UI[INPUT_ID_1].SetValues(RELAY_OFF, 0, PIN_1);
    #else
      UI[INPUT_ID_1].SetValues(RELAY_OFF, 1, PIN_1);
    #endif
  #endif

  #ifdef INPUT_2
    #ifdef PULLUP_2
      UI[INPUT_ID_2].SetValues(RELAY_OFF, 0, PIN_2);
    #else
      UI[INPUT_ID_2].SetValues(RELAY_OFF, 1, PIN_2);
    #endif
  #endif

  #ifdef INPUT_3
    #ifdef PULLUP_3
      UI[INPUT_ID_3].SetValues(RELAY_OFF, 0, PIN_3);
    #else
      UI[INPUT_ID_3].SetValues(RELAY_OFF, 1, PIN_3);
    #endif
  #endif

  #ifdef INPUT_4
    #ifdef PULLUP_4
      UI[INPUT_ID_4].SetValues(RELAY_OFF, 0, PIN_4);
    #else
      UI[INPUT_ID_4].SetValues(RELAY_OFF, 1, PIN_4);
    #endif
  #endif

  // POWER SENSOR
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    PS.SetValues(PS_PIN, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    PS[RELAY_ID_1].SetValues(PS_PIN_1, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_2].SetValues(PS_PIN_2, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_3].SetValues(PS_PIN_3, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_4].SetValues(PS_PIN_4, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
  #endif

  // EXTERNAL THERMOMETER
  #ifdef EXTERNAL_TEMP
    #ifdef DHT22
      pinMode(ET_PIN, INPUT);
    #endif
    #ifdef SHT30
      Wire.begin();
      sht.init();
      sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
    #endif
  #endif

}

/*  *******************************************************************************************
                                            Presentation
 *  *******************************************************************************************/
void presentation() {

  sendSketchInfo(SN, SV);

  // OUTPUT
  #ifdef DOUBLE_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1");
    present(RELAY_ID_2, S_BINARY, "Relay 2");
  #endif

  #ifdef ROLLER_SHUTTER
    present(RS_ID, S_COVER, "Roller Shutter");
  #endif

  #ifdef FOUR_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1"); Serial.flush();
    present(RELAY_ID_2, S_BINARY, "Relay 2"); Serial.flush();
    present(RELAY_ID_3, S_BINARY, "Relay 3"); Serial.flush();
    present(RELAY_ID_4, S_BINARY, "Relay 4"); Serial.flush();
  #endif

  #ifdef DIMMER
    present(DIMMER_ID, S_DIMMER, "Dimmer");
  #endif

  #ifdef RGB
    present(DIMMER_ID, S_RGB_LIGHT, "RGB");
  #endif

  #ifdef RGBW
    present(DIMMER_ID, S_RGBW_LIGHT, "RGBW");
  #endif

  // DIGITAL INPUT
  #ifdef INPUT_1
    present(INPUT_ID_1, S_BINARY, "Input 1");
  #endif

  #ifdef INPUT_2
    present(INPUT_ID_2, S_BINARY, "Input 2");
  #endif

  #ifdef INPUT_3
    present(INPUT_ID_3, S_BINARY, "Input 3");
  #endif

  #ifdef INPUT_4
    present(INPUT_ID_4, S_BINARY, "Input 4");
  #endif

  #ifdef SPECIAL_BUTTON
    present(SPECIAL_BUTTON_ID, S_BINARY, "Special Button");
  #endif

  // POWER SENSOR
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    present(PS_ID, S_POWER, "Power Sensor");
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    present(PS_ID_1, S_POWER, "Power Sensor 1");
    present(PS_ID_2, S_POWER, "Power Sensor 2");
    present(PS_ID_3, S_POWER, "Power Sensor 3");
    present(PS_ID_4, S_POWER, "Power Sensor 4");
  #endif

  // Internal Thermometer
  #ifdef INTERNAL_TEMP
    present(IT_ID, S_TEMP, "Internal Thermometer");
  #endif

  // External Thermometer
  #ifdef EXTERNAL_TEMP
    present(ETT_ID, S_TEMP, "External Termometer");
    present(ETH_ID, S_HUM, "External Termometer");
  #endif

  // I2C


  // Error Reporting
  #ifdef ERROR_REPORTING
    #ifdef POWER_SENSOR
      present(ES_ID, S_BINARY, "OVERCURRENT ERROR");
    #endif
    #ifdef INTERNAL_TEMP
      present(TS_ID, S_BINARY, "THERMAL ERROR");
    #endif
    #ifdef EXTERNAL_TEMP
      present(ETS_ID, S_BINARY, "ET STATUS");
    #endif
  #endif

  #ifdef RS485_DEBUG
    present(DEBUG_ID, S_INFO, "DEBUG INFO");
  #endif

}

/*  *******************************************************************************************
                                            Init Confirmation
 *  *******************************************************************************************/
void InitConfirmation() {

  // OUTPUT
  #ifdef DOUBLE_RELAY
    send(msgUI.setSensor(RELAY_ID_1).set(UI[RELAY_ID_1].NewState));
    request(RELAY_ID_1, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgUI.setSensor(RELAY_ID_2).set(UI[RELAY_ID_2].NewState));
    request(RELAY_ID_2, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    InitConfirm = true;
  #endif

  #ifdef ROLLER_SHUTTER
    send(msgRS1.set(0));
    request(RS_ID, V_UP);
    wait(2000, C_SET, V_UP);

    send(msgRS2.set(0));
    request(RS_ID, V_DOWN);
    wait(2000, C_SET, V_DOWN);

    send(msgRS3.set(0));
    request(RS_ID, V_STOP);
    wait(2000, C_SET, V_STOP);

    send(msgRS4.set(RS.Position));
    request(RS_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    InitConfirm = true;
  #endif

  #ifdef FOUR_RELAY
    send(msgUI.setSensor(RELAY_ID_1).set(UI[RELAY_ID_1].NewState));
    request(RELAY_ID_1, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgUI.setSensor(RELAY_ID_2).set(UI[RELAY_ID_2].NewState));
    request(RELAY_ID_2, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgUI.setSensor(RELAY_ID_3).set(UI[RELAY_ID_3].NewState));
    request(RELAY_ID_3, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgUI.setSensor(RELAY_ID_4).set(UI[RELAY_ID_4].NewState));
    request(RELAY_ID_4, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    InitConfirm = true;
  #endif

  #ifdef DIMMER
    send(msgUI.setSensor(DIMMER_ID).set(DimmerStatus);
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgDIM.set(CurrentLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    InitConfirm = true;
  #endif

  #ifdef RGB
    send(msgUI.setSensor(DIMMER_ID).set(DimmerStatus));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgDIM.set(CurrentLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    send(msgDIM2.set("000000"));
    request(DIMMER_ID, V_RGB);
    wait(2000, C_SET, V_RGB);

    InitConfirm = true;
  #endif

  #ifdef RGBW
    send(msgUI.setSensor(DIMMER_ID).set(DimmerStatus));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgDIM.set(CurrentLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    send(msgDIM3.set("00000000"));
    request(DIMMER_ID, V_RGBW);
    wait(2000, C_SET, V_RGBW);

    InitConfirm = true;
  #endif

  // DIGITAL INPUT
  #ifdef INPUT_1
    send(msgUI.setSensor(INPUT_ID_1).set(UI[INPUT_ID_1].NewState));
  #endif

  #ifdef INPUT_2
    send(msgUI.setSensor(INPUT_ID_2).set(UI[INPUT_ID_2].NewState));
  #endif

  #ifdef INPUT_3
    send(msgUI.setSensor(INPUT_ID_3).set(UI[INPUT_ID_3].NewState));
  #endif

  #ifdef INPUT_4
    send(msgUI.setSensor(INPUT_ID_4).set(UI[INPUT_ID_4].NewState));
  #endif

  #ifdef SPECIAL_BUTTON
    send(msgUI.setSensor(SPECIAL_BUTTON_ID).set(0));
  #endif

  // Built-in sensors
  #ifdef POWER_SENSOR
    #if !defined(FOUR_RELAY)
      send(msgPS.set("0"));
    #elif defined(FOUR_RELAY)
      for(int i=PS_ID_1; i<=PS_ID_4; i++)  {
        send(msgPS.setSensor(i).set("0"));
      }
    #endif
  #endif

  #ifdef INTERNAL_TEMP
    send(msgIT.set((int)IT.MeasureT(ReadVcc())));
  #endif

  // External sensors
  #ifdef EXTERNAL_TEMP
    ETUpdate();
  #endif

  // Error Reporting
  #ifdef ERROR_REPORTING
    #ifdef POWER_SENSOR
      send(msgSI.setSensor(ES_ID).set(0));
    #endif
    #ifdef INTERNAL_TEMP
      send(msgSI.setSensor(TS_ID).set(0));
    #endif
    #ifdef EXTERNAL_TEMP
      send(msgSI.setSensor(ETS_ID).set(0));
    #endif
  #endif

  #ifdef RS485_DEBUG
    send(msgDEBUG.setSensor(DEBUG_ID).set("DEBUG MESSAGE"));
  #endif

}


/*  *******************************************************************************************
                                        MySensors Receive
 *  *******************************************************************************************/
void receive(const MyMessage &message)  {
  
  if (message.type == V_STATUS) {
    #if defined(POWER_SENSOR) && defined(ERROR_REPORTING)
      if (message.sensor == ES_ID)  {
        for (int i = 0; i < 4; i++)  {
          OVERCURRENT_ERROR[i] = message.getBool();
        }
        InformControllerES = false;
      }
    #endif
    #if defined(INTERNAL_TEMP) && defined(ERROR_REPORTING)
      if (message.sensor == TS_ID)  {
        THERMAL_ERROR = message.getBool();
        if (THERMAL_ERROR == false)  {
          InformControllerTS = false;
        }
      }
    #endif
    #ifdef SPECIAL_BUTTON
      if (message.sensor == SPECIAL_BUTTON_ID)  {
        // Ignore this message
      }
    #endif
    #ifdef ROLLER_SHUTTER
      if (message.sensor == RS_ID)  {
        if (message.getBool() == true) {
          UI[RS_ID + 1].NewState = message.getBool();
        }
        else  {
          UI[RS_ID].NewState = message.getBool();
        }
      }
    #endif
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      if (message.sensor == DIMMER_ID) {
        Dimmer.ChangeStatus(message.getBool());
      }
    #endif
    #if defined(DOUBLE_RELAY)
      if (message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS)  {
        if (!OVERCURRENT_ERROR[0] && !THERMAL_ERROR) {
          UI[message.sensor].NewState = message.getBool();
          UI[message.sensor].SetRelay();
        }
      }
    #endif
    #ifdef FOUR_RELAY
      if (message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS) {
        for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++) {
          if (message.sensor == i) {
            if (!OVERCURRENT_ERROR[i] && !THERMAL_ERROR) {
              UI[message.sensor].NewState = message.getBool();
              UI[message.sensor].SetRelay();
            }
          }
        }
      }
    #endif
  }
  else if (message.type == V_PERCENTAGE) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        NewPosition = atoi(message.data);
        NewPosition = NewPosition > 100 ? 100 : NewPosition;
        NewPosition = NewPosition < 0 ? 0 : NewPosition;
      }
    #endif
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      if(message.sensor == DIMMER_ID) {
        int NewLevel = atoi(message.data);
        NewLevel = NewLevel > 100 ? 100 : NewLevel;
        NewLevel = NewLevel < 0 ? 0 : NewLevel;

        Dimmer.DimmerStatus = true;
        Dimmer.ChangeLevel(NewLevel);
      }
    #endif
  }
  else if (message.type == V_RGB || message.type == V_RGBW) {
    #if defined(RGB) || defined(RGBW)
      if(message.sensor == DIMMER_ID) {
        const char *rgbvalues = message.getString();

        Dimmer.DimmerStatus = true;
        Dimmer.NewColorValues(rgbvalues);
        Dimmer.ChangeColors();
      }
    #endif
  }
  else if(message.type == V_DOWN) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        UI[RS_ID + 1].NewState = 1;
      }
    #endif
  }
  else if(message.type == V_UP) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        UI[RS_ID].NewState = 1;
      }
    #endif
  }
  else if(message.type == V_STOP) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        UI[RS_ID].NewState = 0;
        UI[RS_ID + 1].NewState = 0;
      }
    #endif
  }
}

/*  *******************************************************************************************
                                      External Thermometer
 *  *******************************************************************************************/
void ETUpdate()  {

  #ifdef EXTERNAL_TEMP
    #ifdef DHT22
      int chk = DHT.read22(ET_PIN);
      switch (chk)  {
        case DHTLIB_OK:
          send(msgETT.setDestination(0).set(DHT.temperature, 1));
          send(msgETH.set(DHT.humidity, 1));
          #ifdef HEATING_SECTION_SENSOR
            send(msgETT.setDestination(MY_HEATING_CONTROLLER).set(DHT.temperature, 1));
          #endif
          #ifdef ERROR_REPORTING
            if (ET_ERROR != 0) {
              ET_ERROR = 0;
              send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
            }
          #endif
        break;
        case DHTLIB_ERROR_CHECKSUM:
          #ifdef ERROR_REPORTING
            ET_ERROR = 1;
            send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
        case DHTLIB_ERROR_TIMEOUT:
          #ifdef ERROR_REPORTING
            ET_ERROR = 2;
            send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
        default:
          #ifdef ERROR_REPORTING
            ET_ERROR = 3;
            send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
      }
    #elif defined(SHT30)
      if(sht.readSample())  {
        send(msgETT.setDestination(0).set(sht.getTemperature(), 1));
        send(msgETH.set(sht.getHumidity(), 1));
        #ifdef HEATING_SECTION_SENSOR
          send(msgETT.setDestination(MY_HEATING_CONTROLLER).set(sht.getTemperature(), 1));
        #endif
      }
      else  {
        #ifdef ERROR_REPORTING
          ET_ERROR = 1;
          send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
        #endif
      }
    #endif
  #endif
}

/*  *******************************************************************************************
                                        Universal Input
 *  *******************************************************************************************/
void UIUpdate() {

  int FirstSensor;
  int Iterations;

  #ifdef DOUBLE_RELAY
    FirstSensor = RELAY_ID_1;
    Iterations = UI_SENSORS_NUMBER;
  #elif !defined(DOUBLE_RELAY) && defined(INPUT_1)
    FirstSensor = INPUT_ID_1;
    Iterations = NUMBER_OF_INPUTS;
  #endif

  if (Iterations > 0)  {
    for (int i = FirstSensor; i < FirstSensor + Iterations; i++)  {
      UI[i].CheckInput();
      if (UI[i].NewState != UI[i].OldState)  {
        // Door/window/button/motion sensor
        if (UI[i].SensorType == 0 || UI[i].SensorType == 1)  {
          send(msgUI.setSensor(i).set(UI[i].NewState));
          UI[i].OldState = UI[i].NewState;
        }
        // Relay output
        //
        // Button input
        //
        // Button input + Relay output
        else if (UI[i].SensorType == 4)  {
          if (UI[i].NewState != 2)  {
            if (!OVERCURRENT_ERROR[0] && !THERMAL_ERROR)  {
              UI[i].SetRelay();
              send(msgUI.setSensor(i).set(UI[i].NewState));
            }
          }
          #ifdef SPECIAL_BUTTON
            else if (UI[i].NewState == 2)  {
              send(msgUI.setSensor(SPECIAL_BUTTON_ID).set(true));
              UI[i].NewState = UI[i].OldState;
            }
          #endif
        }
      }
    }
  }
}

/*  *******************************************************************************************
                                    Roller Shutter Calibration
 *  *******************************************************************************************/
void RSCalibration(float Vcc)  {

  #ifdef ROLLER_SHUTTER

  uint16_t DownTimeCumulated = 0;
  uint16_t UpTimeCumulated = 0;
  float Current = 0;
  unsigned long TIME_1 = 0;
  unsigned long TIME_2 = 0;
  unsigned long TIME_3 = 0;

  RS.Move(0);

  delay(1000);

  do  {
    Current = PS.MeasureAC(Vcc);
    delay(80);
  } while(Current > PS_OFFSET);

  RS.Stop();

  for(int i=0; i<CALIBRATION_SAMPLES; i++) {
    TIME_1 = millis();
    RS.Move(1);
    delay(1000);

    do  {
      Current = PS.MeasureAC(Vcc);
      TIME_2 = millis();
      delay(80);
    } while(Current > PS_OFFSET);
    
    RS.Stop();

    TIME_3 = TIME_2 - TIME_1;
    DownTimeCumulated += (int)(TIME_3 / 1000);

    delay(1000);

    TIME_1 = millis();
    RS.Move(0);
    delay(1000);
      
    do  {
      Current = PS.MeasureAC(Vcc);
      TIME_2 = millis();
      delay(80);
    } while(Current > PS_OFFSET);
    
    RS.Stop();

    TIME_3 = TIME_2 - TIME_1;
    UpTimeCumulated += (int)(TIME_3 / 1000);
    delay(1000);
    }

  RS.Position = 0;

  int DownTime = (int)(DownTimeCumulated / CALIBRATION_SAMPLES);
  int UpTime = (int)(UpTimeCumulated / CALIBRATION_SAMPLES);

  RS.Calibration(UpTime, DownTime);

  #endif
    
}

/*  *******************************************************************************************
                                        Roller Shutter
 *  *******************************************************************************************/
void RSUpdate() {

  #ifdef ROLLER_SHUTTER
  // Handling movement ordered by controller (new position percentage)
    if (NewPosition != RS.Position)  {
      float MovementRange = ((float)NewPosition - (float)RS.Position) / 100;       // Downward => MR > 0; Upward MR < 0
      int MovementDirection = MovementRange > 0 ? 1 : 0;                           // MovementDirection: 1 -> Down; 0 -> Up

      int MovementTime = RS.Move(MovementDirection) * (abs(MovementRange) * 1000);
      wait(MovementTime);
      RS.Stop();

      RS.Position = NewPosition;
      EEPROM.put(EEA_RS_POSITION, RS.Position);
      send(msgRS4.set(RS.Position));
    }
  // If no new position set by percentage, check buttons
    else  {
      for (int i = RS_ID; i < RS_ID + 2; i++)  {
        UI[i].CheckInput();
        if (UI[i].NewState != UI[i].OldState)  {
        // Handling regular upwards/downwards movement of the roller shutter
          if (UI[i].NewState == 1) {
            int Time = RS.Move(i);
            UI[i].OldState = UI[i].NewState;

            unsigned long TIME_1 = millis();
            unsigned long TIME_2 = 0;
            unsigned long TIME_3 = 0;

            while (UI[RS_ID].NewState == UI[RS_ID].OldState && UI[RS_ID + 1].NewState == UI[RS_ID + 1].OldState) {
              UI[RS_ID].CheckInput();
              UI[RS_ID + 1].CheckInput();
              wait(100);

              TIME_2 = millis();
              TIME_3 = TIME_2 - TIME_1;
              TIME_3 = TIME_3 / 1000;

              if (TIME_3 > Time) {
                RS.Stop();
                RS.Position = (i == 1 ? 100 : 0);
                UI[RS_ID].NewState = 0; UI[RS_ID + 1].NewState = 0;
                break;
              }
            }
            if (TIME_3 < Time)  {
              RS.Stop();
              UI[RS_ID].NewState = 0; UI[RS_ID + 1].NewState = 0;
              int PositionChange = (float) TIME_3 / (float) Time * 100;
              RS.Position += (i == 1 ? PositionChange : -PositionChange);
              RS.Position = RS.Position > 100 ? 100 : RS.Position;
              RS.Position = RS.Position < 0 ? 0 : RS.Position;
            }
            
            UI[RS_ID].OldState = UI[RS_ID].NewState;
            UI[RS_ID + 1].OldState = UI[RS_ID + 1].NewState;
            NewPosition = RS.Position;
            EEPROM.put(EEA_RS_POSITION, RS.Position);
            send(msgRS4.set(RS.Position));
          }
          // Procedure to call out calibration process
          else if (UI[i].NewState == 2)  {
            int SecondButton = (i == RS_ID ? RS_ID + 1 : RS_ID);
            for (int j = 0; j < 10; j++) {
              UI[SecondButton].CheckInput();
              wait(100);
            }
            if (UI[SecondButton].NewState == 2)  {
              UI[SecondButton].NewState = UI[SecondButton].OldState;
              #ifdef RS_AUTO_CALIBRATION
                float Vcc = ReadVcc();
                RSCalibration(Vcc);
              #endif
            }
            else  {
              send(msgUI.setSensor(SPECIAL_BUTTON_ID).set(true));
              UI[i].NewState = UI[i].OldState;
            }
          }
        }
      }
    }
  #endif
}

/*  *******************************************************************************************
                                        Power Sensor
 *  *******************************************************************************************/
void PSUpdate(float Current, uint8_t Sensor = 0)  {
  
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    send(msgPS.set(PS.CalculatePower(Current, COSFI), 0));
    PS.OldValue = Current;
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    send(msgPS.setSensor(Sensor+4).set(PS[Sensor].CalculatePower(Current, COSFI), 0));
    PS[Sensor].OldValue = Current;
  #endif

}

/*  *******************************************************************************************
                                     Internal Thermometer
 *  *******************************************************************************************/
void ITUpdate(float Vcc) {

  #ifdef INTERNAL_TEMP
    send(msgIT.set((int)IT.MeasureT(Vcc)));
  #endif
}

/*  *******************************************************************************************
 *                                    Read Vcc
 *  *******************************************************************************************/
long ReadVcc() {
  
  long result;
  
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  
  delay(2);
  
  ADCSRA |= _BV(ADSC); // Convert
  
  while (bit_is_set(ADCSRA,ADSC));
  
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  result = result;
  
  return result;
}

/*  *******************************************************************************************
                                        Main Loop
 *  *******************************************************************************************/
void loop() {

  float Vcc = ReadVcc(); // mV
  float Current;

  if (!InitConfirm)  {
    InitConfirmation();
  }

  #ifdef ROLLER_SHUTTER
    if(!RS.Calibrated)  {
    #ifdef RS_AUTO_CALIBRATION
      RSCalibration(Vcc);
    #else
      RS.Calibration(UP_TIME, DOWN_TIME);
    #endif
    }
  #endif

  #ifdef POWER_SENSOR
    #ifdef DOUBLE_RELAY
      if (digitalRead(RELAY_1) == RELAY_ON || digitalRead(RELAY_2) == RELAY_ON)  {
        Current = PS.MeasureAC(Vcc);
      }
      else  {
        Current = 0;
      }
      #ifdef ERROR_REPORTING
        if (!OVERCURRENT_ERROR[0]) {
          OVERCURRENT_ERROR[0] = PS.ElectricalStatus(Current);
        }
      #endif
      if (Current < 0.5) {
        if (Current == 0 && PS.OldValue != 0)  {
          PSUpdate(Current);
        }
        else if (abs(PS.OldValue - Current) > 0.05) {
          PSUpdate(Current);
        }
      }
      else  {
        if(abs(PS.OldValue - Current) > (0.1 * PS.OldValue))  {
          PSUpdate(Current);
        }
      }
    #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
      if (Dimmer.DimmerStatus)  {
        Current = PS.MeasureDC(Vcc);
      }
      else  {
        Current = 0;
      }
      #ifdef ERROR_REPORTING
        if (!OVERCURRENT_ERROR[0]) {
          OVERCURRENT_ERROR[0] = PS.ElectricalStatus(Current);
        }
      #endif
      if (Current < 0.5) {
        if (Current == 0 && PS.OldValue != 0)  {
          PSUpdate(Current);
        }
        else if (abs(PS.OldValue - Current) > 0.05) {
          PSUpdate(Current);
        }
      }
      else  {
        if(abs(PS.OldValue - Current) > (0.1 * PS.OldValue))  {
          PSUpdate(Current);
        }
      }
    #elif defined(FOUR_RELAY)
      for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++) {
        if (UI[i].OldState == RELAY_ON)  {
          Current = PS[i].MeasureAC(Vcc);
        }
        else  {
          Current = 0;
        }
        #ifdef ERROR_REPORTING
          if (!OVERCURRENT_ERROR[i])  {
          OVERCURRENT_ERROR[i] = PS[i].ElectricalStatus(Current);
          }
        #endif
        if (Current < 0.5)  {
          if (Current == 0 && PS[i].OldValue != 0)  {
            PSUpdate(Current, i);
          }
          else if (abs(PS[i].OldValue - Current) > 0.05)  {
            PSUpdate(Current, i);
          }
        }
        else  {
          if (abs(PS[i].OldValue - Current) > (0.1 * PS[i].OldValue)) {
            PSUpdate(Current, i);
          }
        }
      }
    #endif
  #endif

  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
    THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT(Vcc));
  #endif

  // Regular main loop
  #ifdef ROLLER_SHUTTER
    RSUpdate();
  #endif

  if (UI_SENSORS_NUMBER > 0) {
    UIUpdate();
  }

  if ((millis() > LastUpdate + INTERVAL) || CheckNow == true)  {
    #ifdef INTERNAL_TEMP
      ITUpdate(Vcc);
    #endif
    #ifdef EXTERNAL_TEMP
      ETUpdate();
    #endif
    LastUpdate = millis();
    CheckNow = false;
  }

  // Handling safety procedures
  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
    if (THERMAL_ERROR == true && InformControllerTS == false) {
    // Board temperature to hot
    // Turn off relays
      #ifdef NUMBER_OF_RELAYS
        for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
          UI[i].NewState = RELAY_OFF;
          UI[i].SetRelay();
          send(msgUI.setSensor(i).set(UI[i].NewState));
        }
      #endif
      send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = true;
      CheckNow = true;
    }
    else if (THERMAL_ERROR == false && InformControllerTS == true) {
      send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = false;
    }
  #endif
  #if defined(ERROR_REPORTING) && defined(POWER_SENSOR)
  // AC current to big
  // Turn off relays
    #ifdef FOUR_RELAY
      for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
        if (OVERCURRENT_ERROR[i] == true && InformControllerES == false) {
          UI[i].NewState = RELAY_OFF;
          UI[i].SetRelay();
          send(msgUI.setSensor(i).set(UI[i].NewState));
          send(msgSI.setSensor(ES_ID).set(OVERCURRENT_ERROR[i]));
          InformControllerES = true;
        }
      }
      if (OVERCURRENT_ERROR[0] == false && OVERCURRENT_ERROR[1] == false && OVERCURRENT_ERROR[2] == false && OVERCURRENT_ERROR[3] == false && InformControllerES == true) {
        InformControllerES = false;
      }
    #elif defined(DOUBLE_RELAY)
      if (OVERCURRENT_ERROR[0] == true && InformControllerES == false) {
        for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
          UI[i].NewState = RELAY_OFF;
          UI[i].SetRelay();
          send(msgUI.setSensor(i).set(UI[i].NewState));
          send(msgSI.setSensor(ES_ID).set(OVERCURRENT_ERROR[0]));
          InformControllerES = true;
        }
      }
      if (OVERCURRENT_ERROR[0] == false && InformControllerES == true) {
        InformControllerES = false;
      }
    #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
      if (OVERCURRENT_ERROR[0] == true && InformControllerES == false) {
        bool NewStatus = false;
        Dimmer.ChangeStatus(NewStatus);
        send(msgUI.setSensor(i).set(UI[i].NewState));
        send(msgSI.setSensor(ES_ID).set(OVERCURRENT_ERROR[0]));
        InformControllerES = true;
      }
      if (OVERCURRENT_ERROR[0] == false && InformControllerES == true) {
        InformControllerES = false;
      }
    #endif
  #endif

  wait(LOOP_TIME);
}
/*

   EOF

*/