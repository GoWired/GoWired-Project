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
 * This is source code for Heating Controller, which can work on GoWired MCU with any shield.
 * 
 */

/***** INCLUDES *****/
#include "Configuration.h"
#include <GoWired2.h>
#include <MySensors.h>
#include <Wire.h>
#include <PCF8575.h>
#include "SHTSensor.h"
#include <avr/wdt.h>

/***** Globals *****/
// Timer
unsigned long LastUpdate = 0;                      // Interval for updating internal thermometer values to controller

// Heating Values
bool NewState;
bool HeatingStatus = false;                        // Autonomous Heating Master Switch
uint8_t HeatingMode;                               // 0: OFF / 10: Day / 20: Night / 30: Holiday / 40: Manual control
float SetPointNight;                               // Temperature set by controller for nighttime
float SetPointHoliday;                             // Temperature set by controller for departures
float Hysteresis;                                  // Histeresis value, default 0

#ifdef DIRECT_SHIELD
  uint8_t OutputPins[4] = {OUTPUT_PIN_1, OUTPUT_PIN_2, OUTPUT_PIN_3, OUTPUT_PIN_4};
#endif

// Additional presentation status required by Home Assistant
bool InitConfirm = false;

// EEPROM
uint16_t EEPROM_ADDRESS = EA_FIRST_SECTION;        // Initial EEPROM Address

// Module Safety Indicators
bool THERMAL_ERROR = false;                        // Thermal error status
bool InformControllerTS = false;                   // Was controller informed about error?
bool IT_STATUS = false;

/***** Constructors *****/
// Heating constructor
Heating Section[HEATING_SECTIONS];
MyMessage msgSTATUS(0, V_STATUS);
MyMessage msgPERCENTAGE(0, V_PERCENTAGE);
MyMessage msgHVAC1(0, V_HVAC_SETPOINT_HEAT);
MyMessage msgHVAC2(0, V_HVAC_FLOW_STATE);
MyMessage msgTEMP(0, V_TEMP);

// I2C expander
#ifdef EXPANDER_SHIELD
  PCF8575 Exp;
#endif

// SHT30 sensor
#ifdef INTERNAL_TEMP
  SHTSensor sht;
  MyMessage msgHUM(0, V_HUM);
#endif

/**
 * @brief Function called before setup(); resets wdt
 * 
 */
void before() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif
}

/**
 * @brief Setups software components: wdt, expander, inputs, outputs
 * 
 */
void setup() {

  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_4S);
  #endif

  #ifdef EXPANDER_SHIELD
    Exp.begin(EXPANDER_ADDRESS);
  #endif

  #ifdef INTERNAL_TEMP
    Wire.begin();
    sht.init();
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
  #endif

  uint8_t T_ID[8] = {T_ID_1, T_ID_2, T_ID_3, T_ID_4, T_ID_5, T_ID_6, T_ID_7, T_ID_8};
  
  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    EEPROM_ADDRESS = Section[i].SetSectionValues(T_ID[i], DEFAULT_DAY_SP, EEPROM_ADDRESS, RELAY_OFF);
    Section[i].RelayState = RELAY_OFF;
    #ifdef EXPANDER_SHIELD
      Exp.pinMode(i, OUTPUT);
      Exp.digitalWrite(i, Section[i].RelayState);
    #elif defined(DIRECT_SHIELD)
      pinMode(OutputPins[i], OUTPUT);
      digitalWrite(OutputPins[i], Section[i].RelayState);
    #endif
  }

  EEPROM.get(EA_HM, HeatingMode);

  HeatingMode = HeatingMode > 40 ? 0 : HeatingMode;
  HeatingMode = HeatingMode < 0 ? 0 : HeatingMode;
  HeatingStatus = HeatingMode == 0 ? false : true;
  
  EEPROM.get(EA_SPN, SetPointNight);

  SetPointNight = SetPointNight < 100 ? SetPointNight : DEFAULT_NIGHT_SP;
  
  EEPROM.get(EA_SPH, SetPointHoliday);

  SetPointHoliday = SetPointHoliday < 100 ? SetPointHoliday : DEFAULT_HOLIDAY_SP;
  
  EEPROM.get(EA_HYSTERESIS, Hysteresis);

  Hysteresis = Hysteresis < 100 ? Hysteresis : DEFAULT_HYSTERESIS;
  
}

/**
 * @brief Presents module to the controller, send name, software version, info about sensors
 * 
 */
void presentation() {

  sendSketchInfo(SN, SV);

  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    present(i, S_HVAC, "Heating Section");
    wait(PRESENTATION_DELAY);
  }

  present(SELECTOR_SWITCH_ID, S_DIMMER, "Heating Mode");  wait(PRESENTATION_DELAY);
  present(SPN_ID, S_HVAC, "SetPoint Night");  wait(PRESENTATION_DELAY);
  present(SPH_ID, S_HVAC, "SetPoint Holidays"); wait(PRESENTATION_DELAY);
  present(HYSTERESIS_ID, S_HVAC, "SetPoint Hysteresis");  wait(PRESENTATION_DELAY);

  #ifdef INTERNAL_TEMP
    present(ITT_ID, S_TEMP, "Onboard SHT30 temperature"); wait(PRESENTATION_DELAY);
    present(ITH_ID, S_HUM, "Onboard SHT30 humidity"); wait(PRESENTATION_DELAY);
  #endif
  
}

/**
 * @brief Sends initial value of sensors as required by Home Assistant
 * 
 */
void InitConfirmation() {

  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {   
    send(msgHVAC1.setSensor(i).set(Section[i].SetPointDay, 1));
    request(i, V_HVAC_SETPOINT_HEAT);
    wait(2000, C_SET, V_HVAC_SETPOINT_HEAT);

    send(msgHVAC2.setSensor(i).set("Off"));
    request(i, V_HVAC_FLOW_STATE);
    wait(2000, C_SET, V_HVAC_FLOW_STATE);
  }

  send(msgSTATUS.setSensor(SELECTOR_SWITCH_ID).set(HeatingStatus));
  request(SELECTOR_SWITCH_ID, V_STATUS);
  wait(2000, C_SET, V_STATUS);
  
  send(msgPERCENTAGE.setSensor(SELECTOR_SWITCH_ID).set(HeatingMode));
  request(SELECTOR_SWITCH_ID, V_PERCENTAGE);
  wait(2000, C_SET, V_PERCENTAGE);
  
  send(msgHVAC1.setSensor(SPN_ID).set(SetPointNight, 1));
  request(SPN_ID, V_HVAC_SETPOINT_HEAT);
  wait(2000, C_SET, SPN_ID);

  send(msgHVAC2.setSensor(SPN_ID).set("Off"));
  request(SPN_ID, V_HVAC_FLOW_STATE);
  wait(2000, C_SET, V_HVAC_FLOW_STATE);
  
  send(msgHVAC1.setSensor(SPH_ID).set(SetPointHoliday, 1));
  request(SPH_ID, V_HVAC_SETPOINT_HEAT);
  wait(2000, C_SET, SPH_ID);

  send(msgHVAC2.setSensor(SPH_ID).set("Off"));
  request(SPH_ID, V_HVAC_FLOW_STATE);
  wait(2000, C_SET, V_HVAC_FLOW_STATE);
  
  send(msgHVAC1.setSensor(HYSTERESIS_ID).set(Hysteresis, 1));
  request(HYSTERESIS_ID, V_HVAC_SETPOINT_HEAT);
  wait(2000, C_SET, HYSTERESIS_ID);

  send(msgHVAC2.setSensor(HYSTERESIS_ID).set("Off"));
  request(HYSTERESIS_ID, V_HVAC_FLOW_STATE);
  wait(2000, C_SET, V_HVAC_FLOW_STATE);
    
  InitConfirm = true;
  
}

/**
 * @brief Handles incoming messages
 * 
 * @param message incoming message data
 */
void receive(const MyMessage &message)  {

  switch(message.type)  {
    // Messages about relay and error status
    case V_STATUS:
      if(message.sensor == SELECTOR_SWITCH_ID) {
        HeatingStatus = message.getBool();
        if(!HeatingStatus) {
          for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
            send(msgHVAC2.setSensor(i+HEATING_SECTIONS).set("Off"));
          }
          send(msgHVAC2.setSensor(SPN_ID).set("Off"));
          send(msgHVAC2.setSensor(SPH_ID).set("Off"));
        }
      }
      break;
    // Messages to Selector Switch
    case V_PERCENTAGE:
      if(message.sensor == SELECTOR_SWITCH_ID) {
        int NewValue = message.getInt();
        if(NewValue == 0 || NewValue == 10 || NewValue == 20 || NewValue == 30 || NewValue == 40)  {
          HeatingMode = NewValue;
          EEPROM.put(EA_HM, HeatingMode);
        }        
        if(NewValue == 10)  {
          send(msgHVAC2.setSensor(SPN_ID).set("Off"));
          send(msgHVAC2.setSensor(SPH_ID).set("Off")); 
        }
        else if(NewValue == 20)  {
          send(msgHVAC2.setSensor(SPN_ID).set("HeatOn"));
          send(msgHVAC2.setSensor(SPH_ID).set("Off"));
        }
        else if(NewValue == 30)  {
          send(msgHVAC2.setSensor(SPH_ID).set("HeatOn"));
          send(msgHVAC2.setSensor(SPN_ID).set("Off"));
        }
        else if(NewValue == 40) {
          send(msgHVAC2.setSensor(SPN_ID).set("Off"));
          send(msgHVAC2.setSensor(SPH_ID).set("Off"));
        }
      }
      break;
    // Messages to Thermostats
    case V_HVAC_SETPOINT_HEAT:
      if(message.sensor == SPN_ID) {
        SetPointNight = message.getFloat();
        EEPROM.put(EA_SPN, SetPointNight);
      }
      else if(message.sensor == SPH_ID)  {
        SetPointHoliday = message.getFloat();
        EEPROM.put(EA_SPH, SetPointHoliday);
      }
      else if(message.sensor == HYSTERESIS_ID) {
        Hysteresis = message.getFloat();
        EEPROM.put(EA_HYSTERESIS, Hysteresis);
      }
      else  {
        for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
          if(message.sensor == i)  {
            Section[i].SetTemperature(message.getFloat());
          }
        }
      }
      break;
    // Messages from remote sensors reporting temperature
    case V_TEMP:
      for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
        if(message.sender == Section[i].GetTID()) {
          float Temp = message.getFloat();
          Section[i].ReadTemperature(Temp);
          send(msgTEMP.setSensor(i).set(Temp, 1));
        }
      }
      break;
    case V_HVAC_FLOW_STATE:
      // Autonomous mode; impossible to change section states by controller
      if(HeatingMode != 40)  {
        if(message.sensor >= FIRST_SECTION_ID && message.sensor <= HYSTERESIS_ID)  {
          uint8_t i = message.sensor;
          send(msgHVAC2.setSensor(i).set("Off"));
        }
      }
      // Manual control; possible to change section states by controller
      else  {
        for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
          if(message.sensor == i) {
            const char *Payload = message.getString();
            if(Payload == "Off")  {
              NewState = RELAY_OFF;
            }
            else if(Payload == "HeatOn")  {
              NewState = RELAY_ON;
            }
            if(NewState != Section[i].RelayState) {
              #ifdef EXPANDER_SHIELD
                Exp.digitalWrite(i, NewState);
              #elif defined(DIRECT_SHIELD)
                digitalWrite(OutputPins[i], NewState);
              #endif
              Section[i].RelayState = NewState;
            }
          }
        }
        if(message.sensor >= FIRST_SECTION_ID+HEATING_SECTIONS)  {
          send(msgHVAC2.setSensor(message.sensor).set("Off"));
        }
      }
    default:
      break;
  }
}

/**
 * @brief Heating controlling backend
 * 
 */
void HeatingUpdate()  {

  bool NewState;

  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    switch(HeatingMode) {
      case 0:
        // Heating inactive
        NewState = Section[i].RelayState;
        break;
      case 10:
        // DAY MODE
        NewState = Section[i].TemperatureCompare(Section[i].SetPointDay, Hysteresis);
        break;
      case 20:
        // NIGHT MODE
        NewState = Section[i].TemperatureCompare(SetPointNight, Hysteresis);
        break;
      case 30:
        // HOLIDAY MODE
        NewState = Section[i].TemperatureCompare(SetPointHoliday, Hysteresis);
        break;
      default:
        // Nothing to do here
        break;
    }
    if(NewState != Section[i].RelayState) {
      #ifdef EXPANDER_SHIELD
        Exp.digitalWrite(i, NewState);
      #elif defined(DIRECT_SHIELD)
        digitalWrite(OutputPins[i], NewState);
      #endif
      
      Section[i].RelayState = NewState;
      uint8_t counter = 0;
      bool MessageDelivered;

      if(NewState == RELAY_ON)  {
        do  {
          MessageDelivered = send(msgHVAC2.setSensor(i).set("HeatOn"));
          counter++;
          wait(10);
        } while (!MessageDelivered || counter < 3);
        counter = 0;
      }
      else  {
        do  {
          MessageDelivered = send(msgHVAC2.setSensor(i).set("Off"));
          counter++;
          wait(10);
        } while (!MessageDelivered || counter < 3);
      }
    }
  }
}

/**
 * @brief Sensing temperature and humidity from attached SHT30 sensor
 * 
 */
void ITUpdate()  {

  #ifdef INTERNAL_TEMP
    if(sht.readSample()) {
      if(IT_STATUS == false)  {
        IT_STATUS = true;
        send(msgSTATUS.setSensor(IT_STATUS_ID).set(IT_STATUS));
      }
      send(msgTEMP.setSensor(ITT_ID).set(sht.getTemperature(), 1));
      send(msgHUM.setSensor(ITH_ID).set(sht.getHumidity(), 1));
    }
    else  {
      IT_STATUS = false;
      send(msgSTATUS.setSensor(IT_STATUS_ID).set(IT_STATUS));
    }
  #endif
}

/**
 * @brief main loop
 * 
 */
void loop() {

  // Extended presentation as required by Home Assistant; runs only after startup
  if (!InitConfirm)  {
    InitConfirmation();
  }

  #ifdef INTERNAL_TEMP
    // Safety check
    THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT());

    // Handling safety procedures
    if(THERMAL_ERROR == true && InformControllerTS == false) {
      // Board temperature to hot
      // Turn off relays/triacs
      HeatingStatus = false;
      for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
        Exp.digitalWrite(i, RELAY_OFF);
        send(msgSTATUS.setSensor(i).set(RELAY_OFF));
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
  #endif
  
  // Heating logic
  if(HeatingStatus == true && HeatingMode != 40)  {
    HeatingUpdate();
  }
  
  wait(LOOP_TIME);
}
/*
 * 
 * EOF
 * 
 */
