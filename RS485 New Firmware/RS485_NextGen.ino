/*
 * This software is designed for various home automation nodes.
 * This is the main file which makes use of some custom libraries that handles various sensors
 * (relays, buttons, thermometers, power meters, roller shutters) and communicates them 
 * with a controller using MySensors protocol.
 * 
 * To define some important variables, look at Configuration.h
 * 
 * Created by feanor-anglin (Domatic.org), 2019-01
 * 
 */

/*  *******************************************************************************************
 *                                      Includes
 *  *******************************************************************************************/
#include "Configuration.h"
#include "PowerSensor.h"
#include "InternalTemp.h"
#include "UniversalInput.h"
#include "Dimmer.h"
#include "RShutterControl.h"
#include <MySensors.h>
#include <SPI.h>
#include <dht.h>

/*  *******************************************************************************************
 *                                      Globals
 *  *******************************************************************************************/
// Power Sensor
int ACVoltage = 230;                        // Typical AC voltage value for Power consumption calculation

// RShutter
int NewPosition;

// Dimmer
int CurrentLevel = 0;
bool DimmerStatus = false;

// Timer
unsigned long LastUpdate = 0;               // Time of last update of interval sensors
bool CheckNow = true;

// Module Safety Indicators
bool THERMAL_ERROR = false;                 // Thermal error status
bool InformControllerTS = false;            // Was controller informed about error?
bool OVERCURRENT_ERROR[4] = {false};             // Overcurrent error status
bool InformControllerES = false;            // Was controller informed about error?
int ET_ERROR = false;                     // External thermometer status (0 - ok, 1 - checksum error, 2 - timeout error)

/*  *******************************************************************************************
 *                                      Constructors
 *  *******************************************************************************************/
//Universal input constructor
#if (UI_SENSORS_NUMBER > 0)
	UniversalInput UI[UI_SENSORS_NUMBER];
	MyMessage msgUI(0, V_LIGHT);
#endif

// Dimmer
#if defined(DIMMER) || defined(RGB) || defined(RGBW)
  Dimmer Dimmer;
  //MyMessage msgDIM(DIMMER_ID, V_PERCENTAGE);
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
  RShutterControl RS(RELAY_1, RELAY_2);
  MyMessage msgRS1(RS_ID, V_UP);
  MyMessage msgRS2(RS_ID, V_DOWN);
  MyMessage msgRS3(RS_ID, V_STOP);
  MyMessage msgRS4(RS_ID, V_PERCENTAGE);
#endif

// Internal thermometer constructor
#ifdef INTERNAL_TEMP
	InternalTemp IT(IT_PIN);
	MyMessage msgIT(IT_ID, V_TEMP);
#endif

// External thermometer constructor 
#ifdef EXTERNAL_TEMP
  dht DHT;
	MyMessage msgETT(ETT_ID, V_TEMP);
	MyMessage msgETH(ETH_ID, V_HUM);
#endif

// Error Reporting
#ifdef ERROR_REPORTING
	MyMessage msgSI(0, V_STATUS);
#endif

#ifdef MY_DEBUG
	MyMessage msgDEBUG(DEBUG_ID, V_TEXT);
#endif

/*  *******************************************************************************************
 *                                          Setup
 *  *******************************************************************************************/
void setup() {
  
  // OUTPUT
  #ifdef SINGLE_RELAY
	  UI[RELAY_ID_1].SetValues(3, BUTTON_1, RELAY_1);
  #endif
  
  #ifdef DOUBLE_RELAY
	  UI[RELAY_ID_1].SetValues(3, BUTTON_1, RELAY_1);
	  UI[RELAY_ID_2].SetValues(3, BUTTON_2, RELAY_2);
  #endif

  #ifdef ROLLER_SHUTTER
	  UI[RELAY_ID_1].SetValues(3, BUTTON_1, RELAY_1);
	  UI[RELAY_ID_2].SetValues(3, BUTTON_2, RELAY_2);
  #endif
  
  #ifdef FOUR_RELAY
	  UI[RELAY_ID_1].SetValues(2, RELAY_1);
	  UI[RELAY_ID_2].SetValues(2, RELAY_2);
	  UI[RELAY_ID_3].SetValues(2, RELAY_3);
	  UI[RELAY_ID_4].SetValues(2, RELAY_4);
  #endif
  
  #ifdef DIMMER
    Dimmer.SetValues(LED_PIN_1);
	  request(DIMMER_ID, V_PERCENTAGE);
  #endif
  
  #ifdef RGB
    Dimmer.SetValues(LED_PIN_1, LED_PIN_2, LED_PIN_3);
    request(DIMMER_ID, V_PERCENTAGE);
  #endif
  
  #ifdef RGBW
    Dimmer.SetValues(LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4);
    request(DIMMER_ID, V_PERCENTAGE);
  #endif
    
  // INPUT
  #ifdef INPUT_1
    #ifdef PULLUP_1
      UI[INPUT_ID_1].SetValues(0, PIN_1);
    #else
      UI[INPUT_ID_1].SetValues(1, PIN_1);
    #endif
  #endif

  #ifdef INPUT_2
    #ifdef PULLUP_2
      UI[INPUT_ID_2].SetValues(0, PIN_2);
    #else
      UI[INPUT_ID_2].SetValues(1, PIN_2);
    #endif
  #endif

  #ifdef INPUT_3
    #ifdef PULLUP_3
      UI[INPUT_ID_3].SetValues(0, PIN_3);
    #else
      UI[INPUT_ID_3].SetValues(1, PIN_3);  
    #endif
  #endif

  #ifdef INPUT_4
    #ifdef PULLUP_4
      UI[INPUT_ID_4].SetValues(0, PIN_4);
    #else
      UI[INPUT_ID_4].SetValues(1, PIN_4);
    #endif
  #endif

  // POWER SENSOR
  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    PS.SetValues(PS_PIN);
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
      PS[RELAY_ID_1].SetValues(PS_PIN_1);
      PS[RELAY_ID_2].SetValues(PS_PIN_2);
      PS[RELAY_ID_3].SetValues(PS_PIN_3);
      PS[RELAY_ID_4].SetValues(PS_PIN_4);
  #endif
    
  // 1WIRE THERMOMETER
  #ifdef EXTERNAL_TEMP
    pinMode(ET_PIN, INPUT);
  #endif
  
}

/*  *******************************************************************************************
 *                                          Presentation
 *  *******************************************************************************************/
void presentation() {

  sendSketchInfo(SN, SV);
  
  // OUTPUT
  #ifdef SINGLE_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1");
      send(msgUI.setSensor(RELAY_ID_1).set(UI[RELAY_ID_1].NewState));
  #endif

  #ifdef DOUBLE_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1");
      send(msgUI.setSensor(RELAY_ID_1).set(UI[RELAY_ID_1].NewState));    
    present(RELAY_ID_2, S_BINARY, "Relay 2");
      send(msgUI.setSensor(RELAY_ID_2).set(UI[RELAY_ID_2].NewState));
  #endif

  #ifdef ROLLER_SHUTTER
    present(RS_ID, S_COVER, "Roller Shutter");
      send(msgRS4.set(RS.Position));
  #endif
  
  #ifdef FOUR_RELAY
    present(RELAY_ID_1, S_BINARY, "Relay 1");
      send(msgUI.setSensor(RELAY_ID_1).set(UI[RELAY_ID_1].NewState));
    present(RELAY_ID_2, S_BINARY, "Relay 2");
      send(msgUI.setSensor(RELAY_ID_2).set(UI[RELAY_ID_2].NewState));
    present(RELAY_ID_3, S_BINARY, "Relay 3");
      send(msgUI.setSensor(RELAY_ID_3).set(UI[RELAY_ID_3].NewState));    
    present(RELAY_ID_4, S_BINARY, "Relay 4");
      send(msgUI.setSensor(RELAY_ID_4).set(UI[RELAY_ID_4].NewState));
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

  #ifdef MY_DEBUG
    present(DEBUG_ID, S_INFO, "DEBUG INFO");
  #endif
    
}

/*  *******************************************************************************************
 *                                      MySensors Receive
 *  *******************************************************************************************/
void receive(const MyMessage &message)  {

  if(message.type == V_STATUS) {
    #if defined(POWER_SENSOR) && defined(ERROR_REPORTING)
      if(message.sensor == ES_ID)  {
        for(int i=0; i<4; i++)  {
          OVERCURRENT_ERROR[i] = message.getBool();
        }
        InformControllerES = false;
      }
    #endif
    #if defined(INTERNAL_TEMP) && defined(ERROR_REPORTING)
      if(message.sensor == TS_ID)  {
        THERMAL_ERROR = message.getBool();
        if(THERMAL_ERROR == false)  {
          InformControllerTS = false;
        }
      }
    #endif
    #ifdef SPECIAL_BUTTON
      if(message.sensor == SPECIAL_BUTTON_ID)  {
        // Ignore this message
      }
    #endif
    #ifdef ROLLER_SHUTTER
      if(message.sensor == RS_ID)  {
        if(message.getBool() == true) {
          UI[RS_ID+1].NewState = message.getBool();
        }
        else  {
          UI[RS_ID].NewState = message.getBool();
        }
      }
    #endif
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      if(message.sensor == DIMMER_ID) {
        Dimmer.ChangeStatus(message.getBool());
        CheckNow = true;
      }
    #endif
    #if defined(SINGLE_RELAY) || defined(DOUBLE_RELAY) //|| defined(FOUR_RELAY)
      if(message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS)  {
          if(!OVERCURRENT_ERROR[0] && !THERMAL_ERROR) {
            send(msgDEBUG.set("Status Received"));
            UI[message.sensor].NewState = message.getBool();
            UI[message.sensor].SetRelay();
            CheckNow = true;
          }
      }
    #endif
    #ifdef FOUR_RELAY
      if(message.sensor >= RELAY_ID_1 && message.sensor < NUMBER_OF_RELAYS) {
        for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++) {
          if(message.sensor == i) {
            if(!OVERCURRENT_ERROR[i]) {
              UI[message.sensor].NewState = message.getBool();
              UI[message.sensor].SetRelay();
              CheckNow = true;
            }
          }
        }
      }
    #endif
  }
  else if(message.type == V_PERCENTAGE) {
    #ifdef ROLLER_SHUTTER
      NewPosition = atoi(message.data);
      NewPosition = NewPosition > 100 ? 100 : NewPosition;
      NewPosition = NewPosition < 0 ? 0 : NewPosition;
    #endif
    #if defined(DIMMER) || defined(RGB) || defined(RGBW)
      int NewLevel = atoi(message.data);
      NewLevel = NewLevel > 100 ? 100 : NewLevel;
      NewLevel = NewLevel < 0 ? 0 : NewLevel;

      Dimmer.DimmerStatus = true;
      Dimmer.ChangeLevel(NewLevel);
      CheckNow = true;
    #endif
  }
  #if defined(RGB) || defined(RGBW)
    if(message.type == V_RGB || message.type == V_RGBW) {
      const char *rgbvalues = message.getString();

      Dimmer.DimmerStatus = true;
      Dimmer.NewColorValues(rgbvalues);
      Dimmer.ChangeColors();
      CheckNow = true;
    }
  #endif
  #ifdef ROLLER_SHUTTER
    if(message.type == V_DOWN) {
      UI[RS_ID+1].NewState = 1;
    }
    else if(message.type == V_UP) {
      UI[RS_ID].NewState = 1;
    }
    else if(message.type == V_STOP) {
      UI[RS_ID].NewState = 0;
      UI[RS_ID+1].NewState = 0;
    }
  #endif
}

/*  *******************************************************************************************
 *                                    External Thermometer
 *  *******************************************************************************************/
void ETUpdate()  {

  #ifdef EXTERNAL_TEMP
    int chk = DHT.read22(ET_PIN);
  
  switch (chk)  {
    case DHTLIB_OK:
      #ifdef ERROR_REPORTING
        ET_ERROR = 0;
        send(msgSI.setSensor(ETS_ID).set(ET_ERROR));
      #endif
      
      send(msgETT.set(DHT.temperature, 1));
      send(msgETH.set(DHT.humidity, 1));
      
      #ifdef HEATING_SECTION_SENSOR
        send(msgETT.setDestination(MY_HEATING_CONTROLLER).set(DHT.temperature, 1));
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
      break;
  }
  #endif
}

/*  *******************************************************************************************
 *                                      Universal Input
 *  *******************************************************************************************/
void UIUpdate() {
  
  int FirstSensor;
  int Iterations;
  
  #if defined(SINGLE_RELAY) || defined(DOUBLE_RELAY)
    FirstSensor = RELAY_ID_1;
    Iterations = UI_SENSORS_NUMBER;
  #elif !defined(SINGLE_RELAY) && !defined(DOUBLE_RELAY) && defined(INPUT_1)
    FirstSensor = INPUT_ID_1;
    Iterations = NUMBER_OF_INPUTS;
  #endif

  if(Iterations > 0)  {
    for(int i=FirstSensor; i<FirstSensor+Iterations; i++)  {
      UI[i].CheckInput();
      if(UI[i].NewState != UI[i].OldState)  {
        // Door/window/button/motion sensor
        if(UI[i].SensorType == 0 || UI[i].SensorType == 1)  {
          send(msgUI.setSensor(i).set(UI[i].NewState));
          UI[i].OldState = UI[i].NewState;
        }
        // Relay output
        /*/else if(UI[i].SensorType == 2)  {
          if(!OVERCURRENT_ERROR[i] && !THERMAL_ERROR)  {
            UI[i].SetRelay();
            CheckNow = true;
          }
        }*/
        // Button input + Relay output
        else if(UI[i].SensorType == 3)  {
          if(UI[i].NewState != 2)  {
            if(!OVERCURRENT_ERROR[0] && !THERMAL_ERROR)  {
              send(msgDEBUG.set("Button pressed"));
              UI[i].SetRelay();
              send(msgUI.setSensor(i).set(UI[i].NewState));
              CheckNow = true;
            }
          }
          #ifdef SPECIAL_BUTTON
            else if(UI[i].NewState == 2)  {
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
 *                                      Roller Shutter
 *  *******************************************************************************************/
void RSUpdate() {

#ifdef ROLLER_SHUTTER
  // Handling movement ordered by controller (new position percentage)
  if(NewPosition != RS.Position)  {
    float MovementRange = ((float)NewPosition - (float)RS.Position) / 100;       // Downward => MR > 0; Upward MR < 0
    int MovementDirection = MovementRange > 0 ? 1 : 0;                           // MovementDirection: 1 -> Down; 0 -> Up

    Serial.println("Percentage movement");
    
    int MovementTime = RS.Move(MovementDirection) * (abs(MovementRange) * 1000);
    wait(MovementTime*1000);
    RS.Stop();

    RS.Position = NewPosition;
    EEPROM.put(EEA_RS_POSITION, RS.Position);
    send(msgRS4.set(RS.Position));
  }
  // If no new position set by percentage, check buttons
  else  {
    for(int i=RS_ID; i<RS_ID+2; i++)  {
      UI[i].CheckInput();
      if(UI[i].NewState != UI[i].OldState)  {
        // Handling regular upwards/downwards movement of the roller shutter
        if(UI[i].NewState == 1) {
          int Time = RS.Move(i);
          UI[i].OldState = UI[i].NewState;

          Serial.println("Button movement");
        
          unsigned long TIME_1 = millis();
          unsigned long TIME_2 = 0;
          float TIME_3 = 0;

          while(UI[RS_ID].NewState == UI[RS_ID].OldState && UI[RS_ID+1].NewState == UI[RS_ID+1].OldState) {
            UI[RS_ID].CheckInput();
            UI[RS_ID+1].CheckInput();
            wait(100);

            TIME_2 = millis();
            TIME_3 = ((float) TIME_2 - (float) TIME_1) / 1000;

            if(TIME_3 > Time) {
              RS.Stop();
              RS.Position = (i == 1 ? 100 : 0);
              UI[RS_ID].NewState = 0; UI[RS_ID+1].NewState = 0;
              break;
            }
          }
          if(TIME_3 < Time)  {
            RS.Stop();
            UI[RS_ID].NewState = 0; UI[RS_ID+1].NewState = 0;
            int PositionChange = (float) TIME_3 / (float) Time * 100;
            RS.Position += (i == 1 ? PositionChange : -PositionChange);
            RS.Position = RS.Position > 100 ? 100 : RS.Position;
            RS.Position = RS.Position < 0 ? 0 : RS.Position;

            #ifdef MY_DEBUG
              Serial.print("TIME_3: ");  Serial.println(TIME_3);
              Serial.print("Time: ");  Serial.println(Time);
              Serial.print("Position change: ");  Serial.println(PositionChange);
            #endif
          }
          UI[RS_ID].OldState = UI[RS_ID].NewState;
          UI[RS_ID+1].OldState = UI[RS_ID+1].NewState;
          NewPosition = RS.Position;
          EEPROM.put(EEA_RS_POSITION, RS.Position);
          send(msgRS4.set(RS.Position));
          
          #ifdef MY_DEBUG
            Serial.print("Position: "); Serial.println(RS.Position);
          #endif
        }
        // Procedure to call out calibration process
        else if(UI[i].NewState == 2)  {
          int SecondButton = (i == RS_ID ? RS_ID+1 : RS_ID);

          RS.Move(1);
          wait(2000);
          RS.Stop();
          for(int j=0; j<10; j++) {
            UI[SecondButton].CheckInput();
            wait(200);
            if(UI[SecondButton].NewState == 2)  {
              RS.Calibration();
              break;
            }
          }
          UI[i].OldState = UI[i].NewState;
          UI[SecondButton].OldState = UI[SecondButton].NewState;
        }
      }
    }
  }
#endif
}

/*  *******************************************************************************************
 *                                      Power Sensor
 *  *******************************************************************************************/
void PSUpdate(int Sensor=0)  {

  #if defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    send(msgPS.set(PS.CalculatePower(PS.MeasureAC(), ACVoltage), 0));
  #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
    send(msgPS.setSensor(Sensor).set(PS[Sensor].CalculatePower(PS[Sensor].MeasureAC(), ACVoltage), 0));
  #endif
}

/*  *******************************************************************************************
 *                                   Internal Thermometer
 *  *******************************************************************************************/
void ITUpdate() {

  #ifdef INTERNAL_TEMP
    send(msgIT.set((int)IT.MeasureT()));
  #endif
}

/*  *******************************************************************************************
 *                                      Main Loop
 *  *******************************************************************************************/
void loop() {

  // Safety check
  #if defined(ERROR_REPORTING) && defined(POWER_SENSOR) && !defined(FOUR_RELAY)
    if(!OVERCURRENT_ERROR[0]) {
      OVERCURRENT_ERROR[0] = PS.ElectricalStatus(PS.MeasureAC());
    }
  #elif defined(ERROR_REPORTING) && defined(POWER_SENSOR) && defined(FOUR_RELAY)
    for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++) {
      if(!OVERCURRENT_ERROR[i])  {
        OVERCURRENT_ERROR[i] = PS[i].ElectricalStatus(PS[i].MeasureAC());
      }
    }
  #endif
  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
    THERMAL_ERROR = IT.ThermalStatus(IT.MeasureT());
  #endif
  
  // Regular main loop
  #ifdef ROLLER_SHUTTER
    RSUpdate();
  #endif
  
  if(UI_SENSORS_NUMBER > 0) {
    UIUpdate();
  }
  
  if((millis() > LastUpdate + INTERVAL) || CheckNow == true)  {
    #ifdef INTERNAL_TEMP
      ITUpdate();
    #endif
    #ifdef EXTERNAL_TEMP
      ETUPDATE();
    #endif
    #if defined(POWER_SENSOR) && defined(SINGLE_RELAY)
      if(digitalRead(RELAY_1) == RELAY_ON)  {
        if(CheckNow)  {
          wait(100);
        }
        PSUpdate();
      }
      else  {
        send(msgPS.set(0));
      }
    #elif defined(POWER_SENSOR) && defined(DOUBLE_RELAY)
      if(digitalRead(RELAY_1) == RELAY_ON || digitalRead(RELAY_2) == RELAY_ON)  {
        if(CheckNow)  {
          wait(100);
        }
        PSUpdate();
      }
      else if(digitalRead(RELAY_1) == RELAY_OFF && digitalRead(RELAY_2) == RELAY_OFF) {
        send(msgPS.set(0));
      }
    #elif defined(POWER_SENSOR) && (defined(DIMMER) || defined(RGB) || defined(RGBW))
      if(Dimmer.DimmerStatus)  {
        if(CheckNow)  {
          wait(100);
        }
        PSUpdate();
      }
      else  {
        send(msgPS.set(0));
      }
    #elif defined(POWER_SENSOR) && defined(FOUR_RELAY)
      for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++) {
        if(UI[i].NewState == RELAY_ON)  {
        PSUpdate(PS_ID_1+i);
      }
      else  {
        send(msgPS.setSensor(PS_ID_1+i).set(0));
      }
    }
    #endif
    LastUpdate = millis();
    CheckNow = false;
  }

  // Handling safety procedures
  #if defined(ERROR_REPORTING) && defined(INTERNAL_TEMP)
  if(THERMAL_ERROR == true && InformControllerTS == false) {
    // Board temperature to hot
    // Turn off relays
    #ifdef NUMBER_OF_RELAYS
    for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++)  {
      UI[i].NewState = RELAY_OFF;
      UI[i].SetRelay();
      send(msgUI.setSensor(i).set(UI[i].NewState));
    }
    #endif
    send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
    InformControllerTS = true;
    CheckNow = true;
  }
  else if(THERMAL_ERROR == false && InformControllerTS == true) {
    send(msgSI.setSensor(TS_ID).set(THERMAL_ERROR));
    InformControllerTS = false;
  }
  #endif
  #if defined(ERROR_REPORTING) && defined(POWER_SENSOR)
    // AC current to big
    // Turn off relays
    #ifdef FOUR_RELAY
      for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++)  {
        if(OVERCURRENT_ERROR[i] == true && InformControllerES == false) {
          UI[i].NewState = RELAY_OFF;
          UI[i].SetRelay();
          send(msgUI.setSensor(i).set(UI[i].NewState));
          send(msgSI.setSensor(ES_ID).set(OVERCURRENT_ERROR[i]));
          InformControllerES = true;
          #ifdef MY_DEBUG
            send(msgDEBUG.set("OVERCURRENT RELAY: "));
            send(msgDEBUG.set(i));
          #endif
        }
      }
    #elif defined(SINGLE_RELAY) || defined(DOUBLE_RELAY)
      if(OVERCURRENT_ERROR[0] == true && InformControllerES == false) {
        for(int i=RELAY_ID_1; i<RELAY_ID_1+NUMBER_OF_RELAYS; i++)  {
          UI[i].NewState = RELAY_OFF;
          UI[i].SetRelay();
          send(msgUI.setSensor(i).set(UI[i].NewState));
          send(msgSI.setSensor(ES_ID).set(OVERCURRENT_ERROR[0]));
          InformControllerES = true;
        }
      }
    #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
      //
    #endif
  #endif
  
  wait(LOOP_TIME);
}
/*
 * 
 * EOF
 * 
 */
