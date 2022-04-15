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
 * ******************************
 * This is source code for GoWired MCU working with 2SSR, RGBW & 4RelayDin Shields.
 * 
 * 
 */

/*  *******************************************************************************************
                                        Includes
 *  *******************************************************************************************/
#include "PowerSensor.h"
#include "InternalTemp.h"
#include "IODigital.h"
#include "Dimmer.h"
#include "RShutterControl.h"
#include "Configuration.h"
#include <MySensors.h>
#include <dht.h>
#include <Wire.h>
#include <avr/wdt.h>
#include "SHTSensor.h"

/*  *******************************************************************************************
                                        Globals
 *  *******************************************************************************************/
// RShutter
#ifdef ROLLER_SHUTTER
  uint32_t MovementTime;
  uint32_t StartTime;
#endif

// Timer
uint32_t LastUpdate = 0;               // Time of last update of interval sensors
bool CheckNow = false;

// Module Safety Indicators
bool THERMAL_ERROR = false;                 // Thermal error status
bool InformControllerTS = false;            // Was controller informed about error?
bool OVERCURRENT_ERROR[4] = {false, false, false, false};             // Overcurrent error status
bool InformControllerES = false;            // Was controller informed about error?
uint8_t ET_ERROR = 3;                       // External thermometer status (0 - ok, 1 - checksum error, 2 - timeout error, 3 - default/initialization)

// Initialization
bool InitConfirm = false;

/*  *******************************************************************************************
                                        Constructors
 *  *******************************************************************************************/
//Universal input constructor
#if (NUMBER_OF_RELAYS + NUMBER_OF_INPUTS > 0)
  IODigital IOD[NUMBER_OF_RELAYS+NUMBER_OF_INPUTS];
#endif

MyMessage msgSTATUS(0, V_STATUS);
MyMessage msgPERCENTAGE(0, V_PERCENTAGE);
MyMessage msgWATT(0, V_WATT);
MyMessage msgTEMP(0, V_TEMP);
MyMessage msgHUM(0, V_HUM);

// RShutter Control Constructor
#ifdef ROLLER_SHUTTER
  RShutterControl RS(RELAY_1, RELAY_2, RELAY_ON, RELAY_OFF);
  MyMessage msgUP(RS_ID, V_UP);
  MyMessage msgDOWN(RS_ID, V_DOWN);
  MyMessage msgSTOP(RS_ID, V_STOP);
#endif

// Dimmer
#if defined(DIMMER) || defined(RGB) || defined(RGBW)
  Dimmer Dimmer;
  MyMessage msgRGB(DIMMER_ID, V_RGB);
  MyMessage msgRGBW(DIMMER_ID, V_RGBW);
#endif

// Power sensor constructor
#if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
  PowerSensor PS;
#elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
  PowerSensor PS[NUMBER_OF_RELAYS];
#endif

// Internal thermometer constructor
#ifdef INTERNAL_TEMP
  InternalTemp IT(IT_PIN, MAX_TEMPERATURE, MVPERC, ZEROVOLTAGE);
#endif

// External thermometer constructor
#ifdef EXTERNAL_TEMP
  #ifdef DHT22
    dht DHT;
  #endif
  #ifdef SHT30
    SHTSensor sht;
  #endif
#endif

#ifdef RS485_DEBUG
  MyMessage msgDEBUG(DEBUG_ID, V_TEXT);
  MyMessage msgDEBUG2(DEBUG_ID, V_WATT);
  MyMessage msgCUSTOM(0, V_CUSTOM);
#endif

/*  *******************************************************************************************
                                            Before
 *  *******************************************************************************************/
void before() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif

  uint32_t InitDelay = MY_NODE_ID * INIT_DELAY;
  
  wait(InitDelay);
}

/*  *******************************************************************************************
                                            Setup
 *  *******************************************************************************************/
void setup() {

  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_8S);
  #endif

  float Vcc = ReadVcc();  // mV

  // POWER SENSOR
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    PS.SetValues(PS_PIN, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    PS[RELAY_ID_1].SetValues(PS_PIN_1, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_2].SetValues(PS_PIN_2, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_3].SetValues(PS_PIN_3, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    PS[RELAY_ID_4].SetValues(PS_PIN_4, MVPERAMP, RECEIVER_VOLTAGE, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
  #endif

  // OUTPUT
  #ifdef DOUBLE_RELAY
    IOD[RELAY_ID_1].SetValues(RELAY_OFF, 4, BUTTON_1, RELAY_1);
    IOD[RELAY_ID_2].SetValues(RELAY_OFF, 4, BUTTON_2, RELAY_2);
  #endif

  #ifdef ROLLER_SHUTTER
    IOD[RS_ID].SetValues(RELAY_OFF, 3, BUTTON_1);
    IOD[RS_ID + 1].SetValues(RELAY_OFF, 3, BUTTON_2);
    if(!RS.Calibrated) {
      RS.Calibration(UP_TIME, DOWN_TIME);
    }
  #endif

  #ifdef FOUR_RELAY
    IOD[RELAY_ID_1].SetValues(RELAY_OFF, 2, RELAY_1);
    IOD[RELAY_ID_2].SetValues(RELAY_OFF, 2, RELAY_2);
    IOD[RELAY_ID_3].SetValues(RELAY_OFF, 2, RELAY_3);
    IOD[RELAY_ID_4].SetValues(RELAY_OFF, 2, RELAY_4);
  #endif

  #ifdef DIMMER
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4);
    IOD[0].SetValues(0, 3, BUTTON_1);
    IOD[1].SetValues(0, 3, BUTTON_2);
  #endif

  #ifdef RGB
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3);
    IOD[0].SetValues(0, 3, BUTTON_1);
    IOD[1].SetValues(0, 3, BUTTON_2);
  #endif

  #ifdef RGBW
    Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4);
    IOD[0].SetValues(0, 3, BUTTON_1);
    IOD[1].SetValues(0, 3, BUTTON_2);
  #endif

  // INPUT
  #ifdef INPUT_1
    #ifdef PULLUP_1
      IOD[INPUT_ID_1].SetValues(RELAY_OFF, 0, PIN_1);
    #else
      IOD[INPUT_ID_1].SetValues(RELAY_OFF, 1, PIN_1);
    #endif
  #endif

  #ifdef INPUT_2
    #ifdef PULLUP_2
      IOD[INPUT_ID_2].SetValues(RELAY_OFF, 0, PIN_2);
    #else
      IOD[INPUT_ID_2].SetValues(RELAY_OFF, 1, PIN_2);
    #endif
  #endif

  #ifdef INPUT_3
    #ifdef PULLUP_3
      IOD[INPUT_ID_3].SetValues(RELAY_OFF, 0, PIN_3);
    #else
      IOD[INPUT_ID_3].SetValues(RELAY_OFF, 1, PIN_3);
    #endif
  #endif

  #ifdef INPUT_4
    #ifdef PULLUP_4
      IOD[INPUT_ID_4].SetValues(RELAY_OFF, 0, PIN_4);
    #else
      IOD[INPUT_ID_4].SetValues(RELAY_OFF, 1, PIN_4);
    #endif
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
    present(RELAY_ID_1, S_BINARY, "Relay 1");   wait(PRESENTATION_DELAY);
    present(RELAY_ID_2, S_BINARY, "Relay 2");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef ROLLER_SHUTTER
    present(RS_ID, S_COVER, "Roller Shutter");  wait(PRESENTATION_DELAY);
  #endif

  #ifdef FOUR_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1");   wait(PRESENTATION_DELAY);
    present(RELAY_ID_2, S_BINARY, "Relay 2");   wait(PRESENTATION_DELAY);
    present(RELAY_ID_3, S_BINARY, "Relay 3");   wait(PRESENTATION_DELAY);
    present(RELAY_ID_4, S_BINARY, "Relay 4");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef DIMMER
    present(DIMMER_ID, S_DIMMER, "Dimmer"); wait(PRESENTATION_DELAY);
  #endif

  #ifdef RGB
    present(DIMMER_ID, S_RGB_LIGHT, "RGB"); wait(PRESENTATION_DELAY);
  #endif

  #ifdef RGBW
    present(DIMMER_ID, S_RGBW_LIGHT, "RGBW");   wait(PRESENTATION_DELAY);
  #endif

  // DIGITAL INPUT
  #ifdef INPUT_1
    present(INPUT_ID_1, S_BINARY, "Input 1");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef INPUT_2
    present(INPUT_ID_2, S_BINARY, "Input 2");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef INPUT_3
    present(INPUT_ID_3, S_BINARY, "Input 3");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef INPUT_4
    present(INPUT_ID_4, S_BINARY, "Input 4");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef SPECIAL_BUTTON
    present(SPECIAL_BUTTON_ID, S_BINARY, "Special Button"); wait(PRESENTATION_DELAY);
  #endif

  // POWER SENSOR
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    present(PS_ID, S_POWER, "Power Sensor");    wait(PRESENTATION_DELAY);
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    present(PS_ID_1, S_POWER, "Power Sensor 1");    wait(PRESENTATION_DELAY);
    present(PS_ID_2, S_POWER, "Power Sensor 2");    wait(PRESENTATION_DELAY);
    present(PS_ID_3, S_POWER, "Power Sensor 3");    wait(PRESENTATION_DELAY);
    present(PS_ID_4, S_POWER, "Power Sensor 4");    wait(PRESENTATION_DELAY);
  #endif

  // Internal Thermometer
  #ifdef INTERNAL_TEMP
    present(IT_ID, S_TEMP, "Internal Thermometer"); wait(PRESENTATION_DELAY);
  #endif

  // External Thermometer
  #ifdef EXTERNAL_TEMP
    present(ETT_ID, S_TEMP, "External Thermometer"); wait(PRESENTATION_DELAY);
    present(ETH_ID, S_HUM, "External Hygrometer");  wait(PRESENTATION_DELAY);
  #endif

  // I2C


  // Error Reporting
  #ifdef ERROR_REPORTING
    #ifdef POWER_SENSOR
      present(ES_ID, S_BINARY, "OVERCURRENT ERROR");    wait(PRESENTATION_DELAY);
    #endif
    #ifdef INTERNAL_TEMP
      present(TS_ID, S_BINARY, "THERMAL ERROR");    wait(PRESENTATION_DELAY);
    #endif
    #ifdef EXTERNAL_TEMP
      present(ETS_ID, S_BINARY, "ET STATUS");   wait(PRESENTATION_DELAY);
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
    send(msgSTATUS.setSensor(RELAY_ID_1).set(IOD[RELAY_ID_1].NewState));
    request(RELAY_ID_1, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgSTATUS.setSensor(RELAY_ID_2).set(IOD[RELAY_ID_2].NewState));
    request(RELAY_ID_2, V_STATUS);
    wait(2000, C_SET, V_STATUS);

  #endif

  #ifdef ROLLER_SHUTTER
    send(msgUP.set(0));
    request(RS_ID, V_UP);
    wait(2000, C_SET, V_UP);

    send(msgDOWN.set(0));
    request(RS_ID, V_DOWN);
    wait(2000, C_SET, V_DOWN);

    send(msgSTOP.set(0));
    request(RS_ID, V_STOP);
    wait(2000, C_SET, V_STOP);

    send(msgPERCENTAGE.setSensor(RS_ID).set(RS.Position));
    request(RS_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

  #endif

  #ifdef FOUR_RELAY
    send(msgSTATUS.setSensor(RELAY_ID_1).set(IOD[RELAY_ID_1].NewState));
    request(RELAY_ID_1, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgSTATUS.setSensor(RELAY_ID_2).set(IOD[RELAY_ID_2].NewState));
    request(RELAY_ID_2, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgSTATUS.setSensor(RELAY_ID_3).set(IOD[RELAY_ID_3].NewState));
    request(RELAY_ID_3, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgSTATUS.setSensor(RELAY_ID_4).set(IOD[RELAY_ID_4].NewState));
    request(RELAY_ID_4, V_STATUS);
    wait(2000, C_SET, V_STATUS);

  #endif

  #ifdef DIMMER
    send(msgSTATUS.setSensor(DIMMER_ID).set(false));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(msgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

  #endif

  #ifdef RGB
    send(msgSTATUS.setSensor(DIMMER_ID).set(false));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    send(msgRGB.setSensor(DIMMER_ID).set("ffffff"));
    request(DIMMER_ID, V_RGB);
    wait(2000, C_SET, V_RGB);

  #endif

  #ifdef RGBW
    send(msgSTATUS.setSensor(DIMMER_ID).set(false));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);

    send(msgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    send(msgRGBW.setSensor(DIMMER_ID).set("ffffffff"));
    request(DIMMER_ID, V_RGBW);
    wait(2000, C_SET, V_RGBW);

  #endif

  // DIGITAL INPUT
  #ifdef INPUT_1
    send(msgSTATUS.setSensor(INPUT_ID_1).set(IOD[INPUT_ID_1].NewState));
  #endif

  #ifdef INPUT_2
    send(msgSTATUS.setSensor(INPUT_ID_2).set(IOD[INPUT_ID_2].NewState));
  #endif

  #ifdef INPUT_3
    send(msgSTATUS.setSensor(INPUT_ID_3).set(IOD[INPUT_ID_3].NewState));
  #endif

  #ifdef INPUT_4
    send(msgSTATUS.setSensor(INPUT_ID_4).set(IOD[INPUT_ID_4].NewState));
  #endif

  #ifdef SPECIAL_BUTTON
    send(msgSTATUS.setSensor(SPECIAL_BUTTON_ID).set(0));
  #endif

  // Built-in sensors
  #ifdef POWER_SENSOR
    #if !defined(FOUR_RELAY)
      send(msgWATT.setSensor(PS_ID).set("0"));
    #elif defined(FOUR_RELAY)
      for(int i=PS_ID_1; i<=PS_ID_4; i++)  {
        send(msgWATT.setSensor(i).set("0"));
      }
    #endif
  #endif

  #ifdef INTERNAL_TEMP
    send(msgTEMP.setSensor(IT_ID).set((int)IT.MeasureT(ReadVcc())));
  #endif

  // External sensors
  #ifdef EXTERNAL_TEMP
    ETUpdate();
  #endif

  // Error Reporting
  #ifdef ERROR_REPORTING
    #ifdef POWER_SENSOR
      send(msgSTATUS.setSensor(ES_ID).set(0));
    #endif
    #ifdef INTERNAL_TEMP
      send(msgSTATUS.setSensor(TS_ID).set(0));
    #endif
    #ifdef EXTERNAL_TEMP
      send(msgSTATUS.setSensor(ETS_ID).set(0));
    #endif
  #endif

  #ifdef RS485_DEBUG
    send(msgDEBUG.setSensor(DEBUG_ID).set("DEBUG MESSAGE"));
  #endif

  InitConfirm = true;

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
    /*#ifdef ROLLER_SHUTTER
      if (message.sensor == RS_ID)  {
        if (message.getBool() == true) {
          IOD[RS_ID + 1].NewState = message.getBool();
        }
        else  {
          IOD[RS_ID].NewState = message.getBool();
        }
      }
    #endif*/
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      if (message.sensor == DIMMER_ID) {
        //Dimmer.NewState = message.getBool();
        Dimmer.ChangeState(message.getBool());
      }
    #endif
    #if defined(DOUBLE_RELAY)
      if (message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS)  {
        if (!OVERCURRENT_ERROR[0] && !THERMAL_ERROR) {
          IOD[message.sensor].NewState = message.getBool();
          IOD[message.sensor].SetRelay();
        }
      }
    #endif
    #ifdef FOUR_RELAY
      if (message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS) {
        for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++) {
          if (message.sensor == i) {
            if (!OVERCURRENT_ERROR[i] && !THERMAL_ERROR) {
              IOD[message.sensor].NewState = message.getBool();
              IOD[message.sensor].SetRelay();
            }
          }
        }
      }
    #endif
    // Secret configuration
    if(message.sensor == SECRET_CONFIG_ID_1)  {
      #ifdef ROLLER_SHUTTER
        // Roller shutter: calibration
        float Vcc = ReadVcc();
        RSCalibration(Vcc);
      #endif
    }
  }
  else if (message.type == V_PERCENTAGE) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        int NewPosition = atoi(message.data);
        NewPosition = NewPosition > 100 ? 100 : NewPosition;
        NewPosition = NewPosition < 0 ? 0 : NewPosition;
        RS.NewState = 2;
        RSUpdate();
        MovementTime = RS.ReadNewPosition(NewPosition) * 10;
      }
    #endif
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      if(message.sensor == DIMMER_ID) {
        Dimmer.NewDimmingLevel = atoi(message.data);
        Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel > 100 ? 100 : Dimmer.NewDimmingLevel;
        Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel < 0 ? 0 : Dimmer.NewDimmingLevel;
      }
    #endif
  }
  else if (message.type == V_RGB || message.type == V_RGBW) {
    #if defined(RGB) || defined(RGBW)
      if(message.sensor == DIMMER_ID) {
        const char *rgbvalues = message.getString();

        Dimmer.NewColorValues(rgbvalues);
      }
    #endif
  }
  else if(message.type == V_UP) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        MovementTime = RS.ReadMessage(0);
      }
    #endif
  }
  else if(message.type == V_DOWN) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        MovementTime = RS.ReadMessage(1);
      }
    #endif
  }
  else if(message.type == V_STOP) {
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID) {
        MovementTime = RS.ReadMessage(2);
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
          send(msgTEMP.setSensor(ETT_ID).setDestination(0).set(DHT.temperature, 1));
          send(msgHUM.setSensor(ETH_ID).set(DHT.humidity, 1));
          #ifdef HEATING_SECTION_SENSOR
            send(msgTEMP.setSensor(ETT_ID).setDestination(MY_HEATING_CONTROLLER).set(DHT.temperature, 1));
          #endif
          #ifdef ERROR_REPORTING
            if (ET_ERROR != 0) {
              ET_ERROR = 0;
              send(msgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
            }
          #endif
        break;
        case DHTLIB_ERROR_CHECKSUM:
          #ifdef ERROR_REPORTING
            ET_ERROR = 1;
            send(msgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
        case DHTLIB_ERROR_TIMEOUT:
          #ifdef ERROR_REPORTING
            ET_ERROR = 2;
            send(msgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
        default:
          #ifdef ERROR_REPORTING
            ET_ERROR = 3;
            send(msgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
          #endif
        break;
      }
    #elif defined(SHT30)
      if(sht.readSample())  {
        send(msgTEMP.setSensor(ETT_ID).setDestination(0).set(sht.getTemperature(), 1));
        send(msgHUM.setSensor(ETH_ID).set(sht.getHumidity(), 1));
        #ifdef HEATING_SECTION_SENSOR
          send(msgTEMP.setSensor(ETT_ID).setDestination(MY_HEATING_CONTROLLER).set(sht.getTemperature(), 1));
        #endif
      }
      else  {
        #ifdef ERROR_REPORTING
          ET_ERROR = 1;
          send(msgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
        #endif
      }
    #endif
  #endif
}

/*  *******************************************************************************************
                                        Universal Input
 *  *******************************************************************************************/
void IODUpdate() {

  int FirstSensor = 0;
  int Iterations = NUMBER_OF_RELAYS+NUMBER_OF_INPUTS;

  if (Iterations > 0)  {
    for (int i = FirstSensor; i < FirstSensor + Iterations; i++)  {
      IOD[i].CheckInput();
      if (IOD[i].NewState != IOD[i].OldState)  {
        switch(IOD[i].SensorType)  {
          case 0:
            // Door/window/button
          case 1:
            // Motion sensor
            send(msgSTATUS.setSensor(i).set(IOD[i].NewState));
            IOD[i].OldState = IOD[i].NewState;
            break;
          case 2:
            // Relay output
            // Nothing to do here
            break;
          case 3:
            // Button input
            #ifdef DIMMER_ID
              if(i == 0)  {
                if(IOD[i].NewState != 2) {
                  // Change dimmer state
                  Dimmer.ChangeState(!Dimmer.CurrentState);
                  send(msgSTATUS.setSensor(DIMMER_ID).set(Dimmer.CurrentState));
                  IOD[i].OldState = IOD[i].NewState;
                }
                #ifdef SPECIAL_BUTTON
                  if(IOD[i].NewState == 2) {
                    send(msgSTATUS.setSensor(SPECIAL_BUTTON_ID).set(true));
                    IOD[i].NewState = IOD[i].OldState;
                  }
                #endif  
              }
              else if(i == 1) {
                if(IOD[i].NewState != 2)  {
                  if(Dimmer.CurrentState) {
                    // Toggle dimming level by DIMMING_TOGGLE_STEP
                    Dimmer.NewDimmingLevel += DIMMING_TOGGLE_STEP;
                    Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel > 100 ? DIMMING_TOGGLE_STEP : Dimmer.NewDimmingLevel;
                    send(msgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
                  }
                  IOD[i].NewState = IOD[i].OldState;
                }
              }
            #endif
            #ifdef ROLLER_SHUTTER
              if(IOD[i].NewState != 2)  {
                MovementTime = RS.ReadButtons(i) * 1000;
                IOD[i].OldState = IOD[i].NewState;
              }
              else  {
                #ifdef SPECIAL_BUTTON
                  send(msgSTATUS.setSensor(SPECIAL_BUTTON_ID).set(true));
                  IOD[i].NewState = IOD[i].OldState;
                #endif
              }
            #endif
            break;
          case 4:
          // Button input + Relay output
          if (IOD[i].NewState != 2)  {
            if (!OVERCURRENT_ERROR[0] && !THERMAL_ERROR)  {
              IOD[i].SetRelay();
              send(msgSTATUS.setSensor(i).set(IOD[i].NewState));
            }
          }
          #ifdef SPECIAL_BUTTON
            else if (IOD[i].NewState == 2)  {
              send(msgSTATUS.setSensor(SPECIAL_BUTTON_ID).set(true));
              IOD[i].NewState = IOD[i].OldState;
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
}

/*  *******************************************************************************************
                                    Roller Shutter Calibration
 *  *******************************************************************************************/
void RSCalibration(float Vcc)  {

  #ifdef ROLLER_SHUTTER

  float Current = 0;
  uint32_t DownTimeCumulated = 0;
  uint32_t UpTimeCumulated = 0;
  uint32_t StartTime = 0;
  uint32_t StopTime = 0;
  uint32_t MeasuredTime = 0;

  // Opening the shutter  
  RS.NewState = 0;
  RS.Movement();

  do  {
    delay(500);
    wdt_reset();
    Current = PS.MeasureAC(Vcc);
  } while(Current > PS_OFFSET);

  RS.NewState = 2;
  RS.Movement();

  delay(1000);

  // Calibrating
  for(int i=0; i<CALIBRATION_SAMPLES; i++) {
    for(int j=1; j>=0; j--)  {
      RS.NewState = j;
      RS.Movement();
      StartTime = millis();

      do  {
        delay(250);
        Current = PS.MeasureAC(Vcc);
        StopTime = millis();
        wdt_reset();
      } while(Current > PS_OFFSET);

      RS.NewState = 2;
      RS.Movement();

      MeasuredTime = StopTime - StartTime;

      if(j) {
        DownTimeCumulated += (int)(MeasuredTime / 1000);
      }
      else  {
        UpTimeCumulated += (int)(MeasuredTime / 1000);
      }

      delay(1000);
    }
  }

  RS.Position = 0;

  uint8_t DownTime = (int)(DownTimeCumulated / CALIBRATION_SAMPLES);
  uint8_t UpTime = (int)(UpTimeCumulated / CALIBRATION_SAMPLES);

  RS.Calibration(UpTime, DownTime);

  // Inform Controller about the current state of roller shutter
  send(msgSTOP);
  send(msgPERCENTAGE.setSensor(RS_ID).set(RS.Position));
  #ifdef RS485_DEBUG
    send(msgDEBUG.set("DownTime ; UpTime"));
    send(msgCUSTOM.set(DownTime)); send(msgCUSTOM.set(UpTime));
  #endif

  #endif
    
}

/*  *******************************************************************************************
                                        Roller Shutter
 *  *******************************************************************************************/
void RSUpdate() {

  #ifdef ROLLER_SHUTTER

  uint32_t StopTime = 0;
  uint32_t MeasuredTime;
  bool Direction;

  if(RS.State != RS.NewState) {
    if(RS.NewState != 2)  {
      #ifdef RS485_DEBUG
        send(msgDEBUG.set("MovementTime"));
        send(msgCUSTOM.set(MovementTime));
      #endif
      RS.Movement();
      StartTime = millis();
      if(RS.NewState == 0)  {
        send(msgUP);
      }
      else if(RS.NewState == 1) {
        send(msgDOWN);
      }
    }
    else  {
      Direction = RS.State;
      RS.Movement();
      StopTime = millis();
      send(msgSTOP);
    }
  }

  if(RS.State != 2) {
    if(millis() >= StartTime + MovementTime) {
      Direction = RS.State;
      RS.NewState = 2;
      RS.Movement();
      StopTime = millis();
      send(msgSTOP);
    }
    if(millis() < StartTime)  {
      uint32_t Temp = 4294967295 - StartTime + millis();
      wait(MovementTime - Temp);
      RS.NewState = 2;
      RS.Movement();
      send(msgSTOP);
      StartTime = 0;
      StopTime = MovementTime;
    }
  }

  if(StopTime > 0)  {
    MeasuredTime = StopTime - StartTime;
    RS.CalculatePosition(Direction, MeasuredTime);
  
    send(msgPERCENTAGE.setSensor(RS_ID).set(RS.Position));
  }

  #endif
}

/*  *******************************************************************************************
                                        Power Sensor
 *  *******************************************************************************************/
void PSUpdate(float Current, uint8_t Sensor = 0)  {
  
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    send(msgWATT.setSensor(PS_ID).set(PS.CalculatePower(Current, COSFI), 0));
    PS.OldValue = Current;
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    send(msgWATT.setSensor(Sensor+4).set(PS[Sensor].CalculatePower(Current, COSFI), 0));
    PS[Sensor].OldValue = Current;
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
  float Current = 0;

  // Sending out states for the first time (as required by Home Assistant)
  if (!InitConfirm)  {
    InitConfirmation();
  }

  // Reading inputs / activating outputs
  if (NUMBER_OF_RELAYS + NUMBER_OF_INPUTS > 0) {
    IODUpdate();
  }

  // Updating roller shutter
  #ifdef ROLLER_SHUTTER
    RSUpdate();
  #endif

  #if defined(DIMMER) || defined(RGB) || defined(RGBW)
    Dimmer.UpdateDimmer();
  #endif

  // Reading power sensor(s)
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    #if defined(DOUBLE_RELAY) || defined(ROLLER_SHUTTER)
      if (digitalRead(RELAY_1) == RELAY_ON || digitalRead(RELAY_2) == RELAY_ON)  {
        Current = PS.MeasureAC(Vcc);
      }
    #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
      if (Dimmer.CurrentState)  {
        Current = PS.MeasureDC(Vcc);
      }
    #endif
      
    #ifdef ERROR_REPORTING
      OVERCURRENT_ERROR[0] = PS.ElectricalStatus(Current);
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
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++) {
      if (IOD[i].OldState == RELAY_ON)  {
        Current = PS[i].MeasureAC(Vcc);
      }
      else  {
        Current = 0;
      }
      #ifdef ERROR_REPORTING
        OVERCURRENT_ERROR[i] = PS[i].ElectricalStatus(Current);
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

  // Current safety
  #if defined(ERROR_REPORTING) && defined(POWER_SENSOR)
    #ifdef FOUR_RELAY
      for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
        if (OVERCURRENT_ERROR[i]) {
          // Current to high
          IOD[i].NewState = RELAY_OFF;
          IOD[i].SetRelay();
          send(msgSTATUS.setSensor(i).set(IOD[i].NewState));
          send(msgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR[i]));
          InformControllerES = true;
        }
        else if(!OVERCURRENT_ERROR[i] && InformControllerES) {
          // Current normal (only after reporting error)
          send(msgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR[i]));
          InformControllerES = false;
        }
      }
    #else
      if(OVERCURRENT_ERROR[0])  {
        // Current to high
        #ifdef DOUBLE_RELAY
          for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
            IOD[i].NewState = RELAY_OFF;
            IOD[i].SetRelay();
            send(msgSTATUS.setSensor(i).set(IOD[i].NewState));
          }
        #elif defined(ROLLER_SHUTTER)
          RS.NewState = 2;
          RSUpdate();
        #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
          //Dimmer.NewState = false;
          Dimmer.ChangeState(false);
          send(msgSTATUS.setSensor(DIMMER_ID).set(Dimmer.CurrentState));
        #endif

        send(msgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR[0]));
        InformControllerES = true;
      }
      else if(!OVERCURRENT_ERROR[0] && InformControllerES)  {
        // Current normal (only after reporting error)
        send(msgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR[0]));
        InformControllerES = false;
      }
    #endif
  #endif

  // Reading internal temperature sensor
  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
    THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT(Vcc));
  #endif

  // Thermal safety
  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
    if (THERMAL_ERROR && !InformControllerTS) {
    // Board temperature to high
      #ifdef DOUBLE_RELAY
        for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
          IOD[i].NewState = RELAY_OFF;
          IOD[i].SetRelay();
          send(msgSTATUS.setSensor(i).set(IOD[i].NewState));
        }
      #elif defined(ROLLER_SHUTTER)
        RS.NewState = 2;
        RSUpdate();
      #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
        //Dimmer.NewState = false;
        Dimmer.ChangeState(false);
        send(msgSTATUS.setSensor(DIMMER_ID).set(Dimmer.CurrentState));
      #endif
      send(msgSTATUS.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = true;
      CheckNow = true;
    }
    else if (!THERMAL_ERROR && InformControllerTS) {
      send(msgSTATUS.setSensor(TS_ID).set(THERMAL_ERROR));
      InformControllerTS = false;
    }
  #endif

  // Reset LastUpdate if millis() has overflowed
  if(LastUpdate > millis()) {
    LastUpdate = millis();
  }  
  
  // Checking out sensors which report at a defined interval
  if ((millis() > LastUpdate + INTERVAL) || CheckNow == true)  {
    #ifdef INTERNAL_TEMP
      send(msgTEMP.setSensor(IT_ID).set((int)IT.MeasureT(Vcc)));
    #endif
    #ifdef EXTERNAL_TEMP
      ETUpdate();
    #endif
    LastUpdate = millis();
    CheckNow = false;
  }

  wait(LOOP_TIME);
}
/*

   EOF

*/
