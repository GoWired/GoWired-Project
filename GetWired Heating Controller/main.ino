/*
 * This software is designed for various home automation nodes.
 * This is a the main file which makes use of some custom libraries that handles various sensors
 * (relays, buttons, thermometers, power meters, roller shutters) and communicates them 
 * with a controller using MySensors protocol.
 * 
 * To define some important variables, look at Configuration.h
 * 
 * Created by feanaro (Domatic.org)
 * 
 */

/*  *******************************************************************************************
 *                                      Includes
 *  *******************************************************************************************/
#include "Configuration.h"
#include "Heating.h"
#include <MySensors.h>
#include <PCF8574.h>
#include <Wire.h>
#include "SHTSensor.h"

/*  *******************************************************************************************
 *                                      Globals
 *  *******************************************************************************************/
// Timer
unsigned long LastUpdate = 0;                      // Interval for updating internal thermometer values to controller

// Heating Values
bool HeatingStatus;                                // Autonomous Heating Master Switch
uint8_t HeatingMode;                               // 0: Day / 1: night / 2: holiday
float TemperatureSetNight;                         // Temperature set by controller for nighttime
float TemperatureSetHoliday;                       // Temperature set by controller for departures
float Hysteresis;                                  // Histeresis value, default 0
bool PresentationExt = false;

// EEPROM
uint16_t EEPROM_ADDRESS = EA_FIRST_SECTION;             // Initial EEPROM Address

// Module Safety Indicators
bool THERMAL_ERROR = false;                        // Thermal error status
bool InformControllerTS = false;                   // Was controller informed about error?
bool IT_STATUS = false;

/*  *******************************************************************************************
 *                                      Constructors
 *  *******************************************************************************************/
// Heating constructor
Heating Section[HEATING_SECTIONS];
MyMessage msgH1(0, V_STATUS);
MyMessage msgH2(0, V_PERCENTAGE);
MyMessage msgH3(0, V_HVAC_SETPOINT_HEAT);
MyMessage msgH4(0, V_TEMP);

// I2C expander
PCF8574 Expander;

// SHT30 sensor
#ifdef INTERNAL_TEMP
  SHTSensor sht;
  MyMessage msgITT(ITT_ID, V_TEMP);
  MyMessage msgITH(ITH_ID, V_HUM);
#endif

// Module Safety Indicators
MyMessage msgSI(0, V_STATUS);

/*  *******************************************************************************************
 *                                          Setup
 *  *******************************************************************************************/
void setup() {

  Expander.begin(0x38);

  #ifdef INTERNAL_TEMP
    Wire.begin();
    sht.init();
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
  #endif

  uint8_t T_ID[8] = {T_ID_1, T_ID_2, T_ID_3, T_ID_4, T_ID_5, T_ID_6, T_ID_7, T_ID_8};
  
  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    EEPROM_ADDRESS = Section[i].SetSectionValues(T_ID[i], DEFAULT_DAY_SP, EEPROM_ADDRESS, RELAY_ON, RELAY_OFF);
    Section[i].RelayState = RELAY_OFF;
    Expander.pinMode(i, OUTPUT);
    Expander.digitalWrite(i, Section[i].RelayState);
  }

  EEPROM.get(EA_HS, HeatingStatus);
  EEPROM.get(EA_HM, HeatingMode);
  EEPROM.get(EA_SPN, TemperatureSetNight);
  EEPROM.get(EA_SPH, TemperatureSetHoliday);
  EEPROM.get(EA_HYSTERESIS, Hysteresis);

}

/*  *******************************************************************************************
 *                                          Presentation
 *  *******************************************************************************************/
void presentation() {

  sendSketchInfo(SN, SV);

  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    present(i, S_BINARY, "Section Relay");
    present(i+HEATING_SECTIONS, S_HEATER, "Section Setpoint");
  }

  for(int i=FIRST_TSP_ID; i<FIRST_TSP_ID+HEATING_SECTIONS; i++) {
    present(i, S_TEMP, "Section Thermometer");
  }

  present(MASTER_SWITCH_ID, S_BINARY, "Heating Master Switch");
  present(SELECTOR_SWITCH_ID, S_DIMMER, "Heating Mode");
  present(SPN_ID, S_HEATER, "SetPoint Night");
  present(SPH_ID, S_HEATER, "SetPoint Holidays");
  present(HYSTERESIS_ID, S_HEATER, "SetPoint Hysteresis");

  #ifdef INTERNAL_TEMP
    present(ITT_ID, S_TEMP, "Onboard SHT30 temperature");
    present(ITH_ID, S_HUM, "Onboard SHT30 humidity");
  #endif
  
}

/*  *******************************************************************************************
 *                                      MySensors Receive
 *  *******************************************************************************************/
void receive(const MyMessage &message)  {

  switch(message.type)  {
    // Messages about relay and error status
    case V_STATUS:
      if(message.sensor == MASTER_SWITCH_ID) {
        HeatingStatus = message.getBool();
        EEPROM.put(EA_HS, HeatingStatus);
      }
      else  {
        for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
          if(message.sensor == i) {
            Expander.digitalWrite(i, message.getBool());
            #ifdef RS485_DEBUG
              //
            #endif  
          }
        }
      }
      break;
    // Messages to Selector Switch
    case V_PERCENTAGE:
      if(message.sensor == SELECTOR_SWITCH_ID) {
        int NewValue = message.getInt();
        if(NewValue == 0 || NewValue == 10 || NewValue == 20)  {
          HeatingMode = NewValue;
          EEPROM.put(EA_HM, HeatingMode);
        }
        else  {
          #ifdef RS485_DEBUG
            //
          #endif
        }
      }
      break;
    // Messages to Thermostats
    case V_HVAC_SETPOINT_HEAT:
      if(message.sensor == SPN_ID) {
        TemperatureSetNight = message.getFloat();
        EEPROM.put(EA_SPN, TemperatureSetNight);
        #ifdef RS485_DEBUG
          //
        #endif
      }
      else if(message.sensor == SPH_ID)  {
        TemperatureSetHoliday = message.getFloat();
        EEPROM.put(EA_SPH, TemperatureSetHoliday);
        send(msgH3.setSensor(SPH_ID).set(TemperatureSetHoliday, 1));
        #ifdef RS485_DEBUG
          //
        #endif
      }
      else if(message.sensor == HYSTERESIS_ID) {
        Hysteresis = message.getFloat();
        EEPROM.put(EA_HYSTERESIS, Hysteresis);
        send(msgH3.setSensor(HYSTERESIS_ID).set(Hysteresis, 1));
        #ifdef RS485_DEBUG
          //
        #endif
      }
      else  {
        for(int i=FIRST_SPD_ID; i<FIRST_SPD_ID+HEATING_SECTIONS; i++)  {
          if(message.sensor == i)  {
            Section[i-HEATING_SECTIONS].SetTemperature(message.getFloat());
            #ifdef RS485_DEBUG
              //
            #endif
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
          wait(1000);
          send(msgH4.setSensor(i+(2*HEATING_SECTIONS)).set(Temp, 1));
          #ifdef RS485_DEBUG
            //
          #endif
        }
      }
      break;
    default:
      break;
  }
}

/*  *******************************************************************************************
 *                                      Heating Procedure
 *  *******************************************************************************************/
void HeatingUpdate()  {

  bool NewState;

  for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
    switch(HeatingMode) {
      case 0:
        // Daytime
        NewState = Section[i].TemperatureCompare(Section[i].GetSetTemp(), Hysteresis);
        if(NewState != Section[i].RelayState) {
          Expander.digitalWrite(i, NewState);
          send(msgH1.setSensor(i).set(NewState));
          Section[i].RelayState = NewState;
        }
        break;
      case 10:
        // Nighttime
        NewState = Section[i].TemperatureCompare(TemperatureSetNight, Hysteresis);
        if(NewState != Section[i].RelayState) {
          Expander.digitalWrite(i, NewState);
          send(msgH1.setSensor(i).set(NewState));
          Section[i].RelayState = NewState;
        }
        break;
      case 20:
        // Departure
        NewState = Section[i].TemperatureCompare(TemperatureSetHoliday, Hysteresis);
        if(NewState != Section[i].RelayState) {
          Expander.digitalWrite(i, NewState);
          send(msgH1.setSensor(i).set(NewState));
          Section[i].RelayState = NewState;
        }
        break;
      default:
        // Nothing to do here
        break;
    }
  }
}

/*  *******************************************************************************************
 *                                      SHT30 Sensor
 *  *******************************************************************************************/
void ITUpdate()  {
  
  if (sht.readSample()) {
    if(IT_STATUS == false)  {
      IT_STATUS = true;
      send(msgH1.setSensor(IT_STATUS_ID).set(IT_STATUS));
    }
    send(msgITT.set(sht.getTemperature(), 1));
    send(msgITH.set(sht.getHumidity(), 1));
  }
  else  {
    IT_STATUS = false;
    send(msgH1.setSensor(IT_STATUS_ID).set(IT_STATUS));
  }
}

/*  *******************************************************************************************
 *                                      Main Loop
 *  *******************************************************************************************/
void loop() {

  if(PresentationExt == false)  {
  
    for(int i=FIRST_SECTION_ID; i<FIRST_SECTION_ID+HEATING_SECTIONS; i++)  {
      send(msgH1.setSensor(i).set(Section[i].RelayState), true);
      wait(100);
      send(msgH3.setSensor(i+HEATING_SECTIONS).set(Section[i].GetSetTemp(), 1), true);
      wait(100);
    }

    send(msgH1.setSensor(MASTER_SWITCH_ID).set(HeatingStatus), true);
    wait(100);
    send(msgH2.setSensor(SELECTOR_SWITCH_ID).set(HeatingMode), true);
    wait(100);
    send(msgH3.setSensor(SPN_ID).set(TemperatureSetNight, 1), true);
    wait(100);
    send(msgH3.setSensor(SPH_ID).set(TemperatureSetHoliday, 1), true);
    wait(100);
    send(msgH3.setSensor(HYSTERESIS_ID).set(Hysteresis, 1), true);
    wait(100);
    PresentationExt = true;
  }

  // Safety check
  //OVERCURRENT_ERROR = PS.ElectricalStatus(PS.MeasureAC(PS_PIN));
  //THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT());
  
  // Regular main loop
  if(HeatingStatus == true)  {
    HeatingUpdate();
  }

  if(millis() > LastUpdate + INTERVAL)  {
    ITUpdate();
    LastUpdate = millis();
  }
/*
  // Handling safety procedures
  if(THERMAL_ERROR == true && InformControllerTS == false) {
    // Board temperature to hot
    // Turn off relays/triacs
    HeatingStatus = false;
    for(int i=FIRST_RELAY_ID; i<FIRST_RELAY_ID+HEATING_SECTIONS; i++)  {
      Section[i].SetRelay(RELAY_OFF);
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
*/
  
  wait(LOOP_TIME);
}
/*
 * 
 * EOF
 * 
 */
