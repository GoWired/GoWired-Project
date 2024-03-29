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
 * This is source code for GoWired Touch MCU boards working with 2Relay or RGBW shields.
 * 
 */

/***** INCLUDES *****/
#include "Configuration.h"
#include <GoWired.h>
#include <LP50XX.h>
#ifdef SHT30
  #include <SHTSensor.h>
#endif

/***** Globals *****/
// General
// 0 - 2Relay; 1 - RGBW
uint8_t HardwareVariant;
// HardwareVariant = 0: LoadVariant=> 0 - 1 relay; 1 - 2 relays; 2 - roller shutter
// HardwareVariant = 1: LoadVariant=> 0 - rgb; 1 - rgbw; 2 - 1-channel dimmer
uint8_t LoadVariant;
uint8_t Iterations;

// Shutter
uint32_t MovementTime;
uint32_t StartTime;

// Timer
uint32_t LastUpdate = 0;               // Time of last update of interval sensors
bool CheckNow = true;

// Module Safety Indicators
bool OVERCURRENT_ERROR = false;            // Overcurrent error status
bool InformControllerES = false;            // Was controller informed about error?
bool ET_ERROR = 0;                       // External thermometer status (0 - ok, 1 - error)

// Initialization
bool InitConfirm = false;

// Touch Diagnosis
uint8_t LimitTransgressions = 0;
uint8_t LongpressDetection = 0;

/***** Constructors *****/
// LP5009 - onboard RGB LED controller
LP50XX LP5009(BGR, LP5009_ENABLE_PIN);

// CommonIO constructor
CommonIO CommonIO[NUMBER_OF_RELAYS+NUMBER_OF_INPUTS];

// Shutter Constructor
Shutters Shutter(EEA_SHUTTER_TIME_DOWN, EEA_SHUTTER_TIME_UP, EEA_SHUTTER_POSITION);

// Dimmer class constructor
Dimmer Dimmer;

// Power sensor class constructor
#ifdef POWER_SENSOR
  PowerSensor PS;
  MyMessage MsgWATT(0, V_WATT);
#endif

// SHTSensor class constructor
#ifdef SHT30
  SHTSensor sht;
  MyMessage MsgTEMP(0, V_TEMP);
  MyMessage MsgHUM(0, V_HUM);
#endif

// Protocol messages constructors
MyMessage MsgSTATUS(0, V_STATUS);
MyMessage MsgUP(0, V_UP);
MyMessage MsgDOWN(0, V_DOWN);
MyMessage MsgSTOP(0, V_STOP);
MyMessage MsgPERCENTAGE(0, V_PERCENTAGE);
MyMessage MsgRGB(0, V_RGB);
MyMessage MsgRGBW(0, V_RGBW);
MyMessage MsgTEXT(0, V_TEXT);

// Debug
#ifdef RS485_DEBUG
  MyMessage MsgCUSTOM(0, V_CUSTOM);
#endif

/**
 * @brief Setups software components: hardware version, configuration, LED controller, wdt reset
 * 
 */
void before() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif

  // Resistive hardware detection
  uint16_t ReadHardware = analogRead(HARDWARE_DETECTION_PIN);

  if(ReadHardware < 20) {
    // Hardware variant: 2Relay Board
    HardwareVariant = 0;
  }
  else if(ReadHardware < 40)  {
    // Hardware variant: RGBW Board
    HardwareVariant = 1;
  }
  else  {/* Handling error */}

  // Reading dip switches, determining load variant
  pinMode(DIP_SWITCH_1, INPUT_PULLUP);
  pinMode(DIP_SWITCH_2, INPUT_PULLUP);
  pinMode(DIP_SWITCH_3, INPUT_PULLUP);

  delay(100);

  if(!digitalRead(DIP_SWITCH_1)) {
    // DIP SWITCH 1 ON
    // HardwareVariant 0: roller shutter; HardwareVariant 1: Dimmer 
    LoadVariant = 2;
  }
  else  {
    if(!digitalRead(DIP_SWITCH_2))  {
      // DIP SWITCH 2 ON
      // HardwareVariant 0: single switch; HardwareVariant 1: RGB
      LoadVariant = 0;
    }
    else  {
      // Default option - DIP SWITCH 1 & DIP SWITCH 2 OFF
      // HardwareVariant 0: double switch; HardwareVariant 1: RGBW
      LoadVariant = 1;
    }
  }

  if(!digitalRead(DIP_SWITCH_3)) {
    // Saving States to EEPROM
    // TO BE DEVELOPED
  }

  // Calculating number of iterations depending from IO instances
  Iterations = (HardwareVariant == 0 && LoadVariant == 0) ? 1 : 2;

  // Detecting I2C peripherals
  Wire.begin();
  // TO BE DEVELOPED

  // Support for 400kHz available
  //Wire.setClock(400000UL);

  // Initializing LP5009/LP5012
  LP5009.Begin();
}

/**
 * @brief Setups software components: objects, initialize LEDs, enable wdt
 * 
 */
void setup() {

  // LED: 0 || 1 || 2; R,G,B: 0-255, brightness: 0-255
  // LED0 - D1 (Touch Field A0), LED1 - D3 (Touch Field A2), LED2 - D2 (Touch Field A1)
  // LP5009.SetLEDColor(LED, R, G, B)
  // LP5009.SetLEDBrightness(LED, brightness)

  // LED initialization visual effect
  RainbowLED(RAINBOW_DURATION, RAINBOW_RATE);

  float Vcc = ReadVcc();  // mV

  // Initializing POWER SENSOR
  #if defined(POWER_SENSOR)
    if(HardwareVariant == 0)  {
      PS.SetValues(PS_PIN, MVPERAMP, 230, MAX_CURRENT, POWER_MEASURING_TIME, Vcc);
    }
    else if(HardwareVariant == 1) {
      PS.SetValues(PS_PIN, MVPERAMP, 24, 6, POWER_MEASURING_TIME, Vcc);
    }
  #endif

  // Initializing inputs & outputs
  if(HardwareVariant == 0)  {
    if(LoadVariant == 0)  {
      // Lighting: One button, single output
      CommonIO[RELAY_ID_1].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_3, INPUT_PIN_1, RELAY_PIN_1);
      // Not used touch field for calibration purposes
      CommonIO[UNUSED_TF_ID].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_1);
    }
    else if(LoadVariant == 1) {
      // Lighting: two buttons, double output
      CommonIO[RELAY_ID_1].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_1, INPUT_PIN_1, RELAY_PIN_1);
      CommonIO[RELAY_ID_2].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_2, INPUT_PIN_2, RELAY_PIN_2);
      // Not used touch field for calibration purposes
      CommonIO[UNUSED_TF_ID].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_3);
    }
    else if(LoadVariant == 2) {
      // Shutter
      CommonIO[SHUTTER_ID].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_1);
      CommonIO[SHUTTER_ID + 1].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_2);
      // Not used touch field for calibration purposes
      CommonIO[UNUSED_TF_ID].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_3);
      Shutter.SetOutputs(RELAY_OFF, RELAY_PIN_1, RELAY_PIN_2);
      // Temporary calibration (first launch only)
      if(!Shutter.Calibrated) {
        Shutter.Calibration(UP_TIME, DOWN_TIME);
      }
    }
  }
  else if(HardwareVariant == 1) {
    if(LoadVariant == 2)  {
      // 1-channel dimmer
      Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_W);
    }
    else if(LoadVariant == 0) {
      // RGB dimmer
      Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_R, LED_PIN_G, LED_PIN_B);
    }
    else if(LoadVariant == 1) {
      // RGBW dimmer
      Dimmer.SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_PIN_W);
    }
    // Every dimmer has two buttons
    CommonIO[0].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_1);
    CommonIO[1].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_2);
    // Not used touch field for calibration purposes
    CommonIO[UNUSED_TF_ID].SetValues(RELAY_OFF, false, 5, TOUCH_FIELD_3);
  }

  // Indicate inactivity with LEDs
  for(int i=0; i<Iterations; i++) {
    AdjustLEDs(3, i);
  }

  // ONBOARD THERMOMETER
  #ifdef SHT30
    sht.init();
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
  #endif

  // Enable AVR Watchdog
  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_8S);
  #endif

}

/**
 * @brief Presents module to the controller, send name, software version, info about sensors
 * 
 */
void presentation() {

  // OUTPUT
  if(HardwareVariant == 0)  {
    if(LoadVariant == 0)  {
      sendSketchInfo("GWT-2R-1c", SV);
      present(RELAY_ID_1, S_BINARY, "Relay 1");   wait(PRESENTATION_DELAY);
    }
    else if(LoadVariant == 1) {
      sendSketchInfo("GWT-2R-2c", SV);
      present(RELAY_ID_1, S_BINARY, "Relay 1");   wait(PRESENTATION_DELAY);
      present(RELAY_ID_2, S_BINARY, "Relay 2");   wait(PRESENTATION_DELAY);
    }
    else if(LoadVariant == 2) {
      sendSketchInfo("GWT-2R-Shutter", SV);
      present(SHUTTER_ID, S_COVER, "Shutter");  wait(PRESENTATION_DELAY);     
    }
  }
  else if(HardwareVariant == 1) {
    if(LoadVariant == 2)  {
      sendSketchInfo("GWT-D-1c", SV);
      present(DIMMER_ID, S_DIMMER, "Dimmer"); wait(PRESENTATION_DELAY);
    }
    else if(LoadVariant == 0) {
      sendSketchInfo("GWT-D-3c", SV);
      present(DIMMER_ID, S_RGB_LIGHT, "RGB"); wait(PRESENTATION_DELAY);
    }
    else if(LoadVariant == 1) {
      sendSketchInfo("GWT-D-4c", SV);
      present(DIMMER_ID, S_RGBW_LIGHT, "RGBW");   wait(PRESENTATION_DELAY);
    }
  }

  #ifdef SPECIAL_BUTTON
    present(SPECIAL_BUTTON_ID, S_BINARY, "Longpress-1"); wait(PRESENTATION_DELAY);
    present(SPECIAL_BUTTON_ID+1, S_BINARY, "Longpress-2"); wait(PRESENTATION_DELAY);
  #endif

  // POWER SENSOR
  #if defined(POWER_SENSOR)
    present(PS_ID, S_POWER, "Power Sensor");    wait(PRESENTATION_DELAY);
  #endif

  // Onboard Thermometer
  #ifdef SHT30
    present(ETT_ID, S_TEMP, "External Thermometer"); wait(PRESENTATION_DELAY);
    present(ETH_ID, S_HUM, "External Hygrometer");  wait(PRESENTATION_DELAY);
  #endif

  // Electronic fuse
  #ifdef ELECTRONIC_FUSE 
      present(ES_ID, S_BINARY, "OVERCURRENT ERROR");    wait(PRESENTATION_DELAY);
  #endif

  #ifdef SHT30
    present(ETS_ID, S_BINARY, "ET STATUS");   wait(PRESENTATION_DELAY);
  #endif

  #ifdef RS485_DEBUG
    present(DEBUG_ID, S_INFO, "DEBUG INFO");
    present(TOUCH_DIAGNOSTIC_ID, S_CUSTOM, "Touch Diagnostic");
  #endif

  // Configuration sensor
  present(CONFIGURATION_SENSOR_ID, S_INFO, "TEXT MSG");

}

/**
 * @brief Sends initial value of sensors as required by Home Assistant
 * 
 */
void InitConfirmation() {

  // OUTPUT
  // 2Relay
  if(HardwareVariant == 0)  {
    // Single output
    if(LoadVariant == 0)  {
      send(MsgSTATUS.setSensor(RELAY_ID_1).set(CommonIO[RELAY_ID_1].NewState));
      request(RELAY_ID_1, V_STATUS);
      wait(2000, C_SET, V_STATUS);
    }
    // Double output
    else if(LoadVariant == 1) {
      send(MsgSTATUS.setSensor(RELAY_ID_1).set(CommonIO[RELAY_ID_1].NewState));
      request(RELAY_ID_1, V_STATUS);
      wait(2000, C_SET, V_STATUS);

      send(MsgSTATUS.setSensor(RELAY_ID_2).set(CommonIO[RELAY_ID_2].NewState));
      request(RELAY_ID_2, V_STATUS);
      wait(2000, C_SET, V_STATUS);
    }
    // Roller shutter
    else if(LoadVariant == 2) {
      send(MsgUP.setSensor(SHUTTER_ID).set(0));
      request(SHUTTER_ID, V_UP);
      wait(2000, C_SET, V_UP);

      send(MsgDOWN.setSensor(SHUTTER_ID).set(0));
      request(SHUTTER_ID, V_DOWN);
      wait(2000, C_SET, V_DOWN);

      send(MsgSTOP.setSensor(SHUTTER_ID).set(0));
      request(SHUTTER_ID, V_STOP);
      wait(2000, C_SET, V_STOP);

      send(MsgPERCENTAGE.setSensor(SHUTTER_ID).set(Shutter.Position));
      request(SHUTTER_ID, V_PERCENTAGE);
      wait(2000, C_SET, V_PERCENTAGE);
    }
  }
  // RGBW
  else if(HardwareVariant == 1) {
    send(MsgSTATUS.setSensor(DIMMER_ID).set(false));
    request(DIMMER_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);
    
    send(MsgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
    request(DIMMER_ID, V_PERCENTAGE);
    wait(2000, C_SET, V_PERCENTAGE);

    // RGB dimmer
    if(LoadVariant == 0)  {
      send(MsgRGB.setSensor(DIMMER_ID).set("ffffff"));
      request(DIMMER_ID, V_RGB);
      wait(2000, C_SET, V_RGB);
    }
    // RGBW dimmer
    else if(LoadVariant == 1) {
      send(MsgRGBW.setSensor(DIMMER_ID).set("ffffffff"));
      request(DIMMER_ID, V_RGBW);
      wait(2000, C_SET, V_RGBW);
    }
    // 1-channel dimmer
    else if(LoadVariant == 2) {}
  }

  #ifdef SPECIAL_BUTTON
    send(MsgSTATUS.setSensor(SPECIAL_BUTTON_ID).set(0));
    send(MsgSTATUS.setSensor(SPECIAL_BUTTON_ID+1).set(0));
  #endif

  // Built-in sensors
  #ifdef POWER_SENSOR
    send(MsgWATT.set("0"));
  #endif

  // External sensors
  #ifdef SHT30
    ETUpdate();
    send(MsgSTATUS.setSensor(ETS_ID).set(0));
  #endif

  //
  #ifdef ELECTRONIC FUSE
    send(MsgSTATUS.setSensor(ES_ID).set(0));
  #endif

  #ifdef RS485_DEBUG
    send(MsgTEXT.setSensor(DEBUG_ID).set("DEBUG MESSAGE"));
    send(MsgCUSTOM.setSensor(TOUCH_DIAGNOSTIC_ID).set(0));
  #endif

  send(MsgTEXT.setSensor(CONFIGURATION_SENSOR_ID).set("CONFIG INIT"));

  SetLEDs();

  InitConfirm = true;

}

/**
 * @brief performs a rainbow effect on active LEDs
 * 
 * @param Duration duration of effect
 * @param Rate rate of effect
 */
void RainbowLED(uint16_t Duration, uint8_t Rate)	{
	
  int RValue = 254;
  int GValue = 127;
  int BValue = 1;
  int RDirection = -1;
  int GDirection = -1;
  int BDirection = 1;
  uint32_t StartTime = millis();
	
  while(millis() < StartTime + Duration)	{

    if(Iterations == 1)  {
      AdjustLEDs2(BUILTIN_LED3, BRIGHTNESS_VALUE_ON, RValue, GValue, BValue);
    }
    else {
      uint8_t LEDs[2] = {BUILTIN_LED1, BUILTIN_LED2};
      for(int i=0; i<Iterations; i++)  {
        AdjustLEDs2(LEDs[i], BRIGHTNESS_VALUE_ON, RValue, GValue, BValue);
      }
    }
	
    RValue += RDirection;
    GValue += GDirection;
    BValue += BDirection;
	
    if(RValue >= 255 || RValue <= 0)	{
      RDirection = -RDirection;
    }
	
    if(GValue >= 255 || GValue <= 0)	{
      GDirection = -GDirection;
    }
	
    if(BValue >= 255 || BValue <= 0)	{
      BDirection = -BDirection;
    }

    if(millis() < StartTime)  {
      StartTime = millis();
    }

    wait(Rate);
  }
}

/**
 * @brief Adjusts built-in LEDs to current state of buttons 
 * 
 * @param State state of button
 * @param Button number of button (ID)
 */
void AdjustLEDs(uint8_t State, uint8_t Button) {

  uint8_t LED;

  if(HardwareVariant == 0 && LoadVariant == 0)  {
    LED = BUILTIN_LED3;
  }
  else  {
    LED = Button == 0 ? BUILTIN_LED1 : BUILTIN_LED2;
  }

  switch(State) {
    case 0:
      // Output is in OFF state
      AdjustLEDs2(LED, BRIGHTNESS_VALUE_OFF,R_VALUE_OFF, G_VALUE_OFF, B_VALUE_OFF);
      break;
    case 1:
      // Output is in ON state
      AdjustLEDs2(LED, BRIGHTNESS_VALUE_ON,R_VALUE_ON, G_VALUE_ON, B_VALUE_ON);
      break;
    case 2:
      // Long press
      RainbowLED(RAINBOW_DURATION, RAINBOW_RATE);
      break;
    case 3:
      // Indication of button inactivity
      AdjustLEDs2(LED, BRIGHTNESS_VALUE_ON, R_VALUE_INACTIVE, G_VALUE_INACTIVE, G_VALUE_INACTIVE);
      break;
    case 4:
      // Turn all channels OFF
      AdjustLEDs2(LED, 0, 0, 0, 0);
      break;
    default:
      break;
  }
}

/**
 * @brief Sets built-in LEDs to given color and brightness
 * 
 * @param LED number of LED
 * @param Brightness brightness value to be set
 * @param R red value to be set
 * @param G green value to be set
 * @param B blue value to be set
 */
void AdjustLEDs2(uint8_t LED, uint8_t Brightness, uint8_t R, uint8_t G, uint8_t B) {

  // LED: BUILTIN_LED1 / BUILTIN_LED2 / BUILTIN_LED3
  LP5009.SetLEDColor(LED, R, G, B);
  LP5009.SetLEDBrightness(LED, Brightness);
}

/**
 * @brief Sets LEDs according to current output state
 */
void SetLEDs()  {

  // Adjust LEDs back to indicate the states of buttons
  if(HardwareVariant == 0)  {
    if(LoadVariant != 2)  {
      for(int i=0; i<Iterations; i++) {
        AdjustLEDs(CommonIO[i].State, i);
      }
    }
    else {
      if(Shutter.State == 2) {
        AdjustLEDs(0, 0); AdjustLEDs(0, 1);
      }
      else if(Shutter.State == 1)  {
        AdjustLEDs(0, 0); AdjustLEDs(1, 1);
      }
      else if(Shutter.State == 0)  {
        AdjustLEDs(1, 0); AdjustLEDs(0, 1);
      }
    }
  }
  else if(HardwareVariant == 1) {
    if(Dimmer.CurrentState) {
      AdjustLEDs(1, 0); AdjustLEDs(0, 1);
    }
    else  {
      AdjustLEDs(0, 0); AdjustLEDs(0, 1);
    }
  }
}

/**
 * @brief Blinks built-in LEDs with externally controlled period
 * 
 * @param InitialState initial state of function
 */
void BlinkLEDs(uint8_t InitialState=0) {
  
  static uint8_t Blink = 3;

  if(InitialState == 3 || InitialState == 4) {
    Blink = InitialState;
  }

  for(int i=0; i<Iterations; i++) {
    AdjustLEDs(Blink, i);
  }

  Blink = Blink == 3 ? 4 : 3;
}

/**
 * @brief Diagnoses the operation of touch buttons
 * 
 * @param Threshold touch treshold value from Configuration.h
 */
void TouchDiagnosis(uint16_t Threshold) {

  for(int i=0; i<Iterations; i++)  {
    if(CommonIO[i].TouchDiagnosisValue > (int)(Threshold / 2) || CommonIO[i].TouchDiagnosisValue < -(int)(Threshold / 2))  {
      
      #ifdef RS485_DEBUG
        // Send message with TouchValue
        send(MsgCUSTOM.setSensor(TOUCH_DIAGNOSTIC_ID).set(IO[i].TouchDiagnosisValue));
      #endif

      if(CommonIO[i].TouchDiagnosisValue < Threshold) {
        LimitTransgressions++;
      }
    }
  }
}

/**
 * @brief Diagnoses the operation of touch buttons
 * 
 */
void TouchDiagnosis2() {

  for(int i=0; i<2; i++)  {
    CommonIO[UNUSED_TF_ID].CheckInput2(TOUCH_THRESHOLD, LONGPRESS_DURATION, DEBOUNCE_VALUE);
    if(CommonIO[UNUSED_TF_ID].NewState < 2) return;
  }
  
  CommonIO[UNUSED_TF_ID].NewState = 0;
  ReadNewReference();
}

/**
 * @brief Reads new reference value for touch buttons
 * 
 */
void ReadNewReference() {

  if(HardwareVariant == 0 && LoadVariant == 2)  {
    Shutter.NewState = 2;
    Shutter.Movement();
  }

  // Blink LEDs to indicate inactivity and take a break from normal operation
  for(int i=0; i<5; i++)  {
    BlinkLEDs();
    delay(1000);
  }

  // Read new reference
  for(int i=0; i<Iterations+1; i++)  {
    CommonIO[i].ReadReference();
  }

  // Rainbow LED visual effect - indicate calibration
  RainbowLED(RAINBOW_DURATION, RAINBOW_RATE);

  // Adjust LEDs to indicate button states
  SetLEDs();
}

/**
 * @brief Handles incoming messages
 * 
 * @param message incoming message data
 */
void receive(const MyMessage &message)  {
  
  // Binary messages
  if (message.type == V_STATUS) {
    #ifdef SPECIAL_BUTTON
      if (message.sensor == SPECIAL_BUTTON_ID || message.sensor == SPECIAL_BUTTON_ID+1)  {
        // Ignore this message
      }
    #endif
    if(HardwareVariant == 1)  {
      if (message.sensor == DIMMER_ID) {
        Dimmer.ChangeState(message.getBool());
        SetLEDs();
      }
    }
    if(HardwareVariant == 0 && LoadVariant != 2)  {
      if (message.sensor == RELAY_ID_1 || message.sensor == RELAY_ID_2)  {
        if (!OVERCURRENT_ERROR) {
          CommonIO[message.sensor].SetState(message.getBool());
          CommonIO[message.sensor].SetRelay();
          AdjustLEDs(CommonIO[message.sensor].State, message.sensor);
          #ifdef RS485_DEBUG
            send(MsgCUSTOM.setSensor(TOUCH_DIAGNOSTIC_ID).set(CommonIO[message.sensor].DebugValue));
          #endif
        }
      }
    }
  }
  // Percentage messages
  else if (message.type == V_PERCENTAGE) {
    if(HardwareVariant == 0 && LoadVariant == 2)  {
      // Roller shutter position
      if(message.sensor == SHUTTER_ID) {
        int NewPosition = atoi(message.data);
        NewPosition = NewPosition > 100 ? 100 : NewPosition;
        NewPosition = NewPosition < 0 ? 0 : NewPosition;
        Shutter.NewState = 2;
        ShutterUpdate(0);
        MovementTime = Shutter.ReadNewPosition(NewPosition) * 10;
      }
    }
    else if(HardwareVariant == 1)  {
      if(message.sensor == DIMMER_ID) {
        // Dimming value for dimmers (all variants)
        Dimmer.NewDimmingLevel = atoi(message.data);
        Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel > 100 ? 100 : Dimmer.NewDimmingLevel;
        Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel < 0 ? 0 : Dimmer.NewDimmingLevel;
      }
    }
  }
  // RGB/RGBW messages
  else if (message.type == V_RGB || message.type == V_RGBW) {
    if(HardwareVariant == 1 && LoadVariant != 2)  {
      if(message.sensor == DIMMER_ID) {
        const char *rgbvalues = message.getString();

        Dimmer.NewColorValues(rgbvalues);
      }
    }
  }
  // Roller shutter control messages (UP, DOWN, STOP)
  else if(message.type == V_UP) {
    if(HardwareVariant == 0 && LoadVariant == 2 && message.sensor == SHUTTER_ID)  {
      MovementTime = Shutter.ReadMessage(0) * 1000;
    }
  }
  else if(message.type == V_DOWN) {
    if(HardwareVariant == 0 && LoadVariant == 2 && message.sensor == SHUTTER_ID)  {
      MovementTime = Shutter.ReadMessage(1) * 1000;
    }
  }
  else if(message.type == V_STOP) {
    if(HardwareVariant == 0 && LoadVariant == 2 && message.sensor == SHUTTER_ID)  {
      MovementTime = Shutter.ReadMessage(2);
    }
  }
  // Text messages
  else if(message.type == V_TEXT) {
    // Configuration by message
    if(message.sensor == CONFIGURATION_SENSOR_ID)  {
      
      // Initialize strings and pointers
      char ReceivedPayload[10];
      char *RPaddr = ReceivedPayload;
      String RPstr = String(message.getString());

      // Turn String payload to char array and send back to the controller
      RPstr.toCharArray(ReceivedPayload, 10);
      send(MsgTEXT.setSensor(CONFIGURATION_SENSOR_ID).set(RPaddr));

      if(RPstr.equals(CONF_MSG_1)) {
        if(HardwareVariant == 0 && LoadVariant == 2)  {
          // Roller shutter: calibration
          float Vcc = ReadVcc();
          ShutterCalibration(Vcc);
        }
      }
      else if(RPstr.equals(CONF_MSG_2)) {
        // Touch fields calibration
        ReadNewReference();
      }
      else if(RPstr.equals(CONF_MSG_3)) {
        // Watchdog test procedure / module restart
        delay(10000);
      }
      else if(RPstr.equals(CONF_MSG_4)) {
        // Clear EEPROM and restart
        for (int i=0;i<1024;i++) {
          EEPROM.write(i,0xFF);
        }
        delay(10000);
      }
    }
  }
}

/**
 * @brief Reads temperature & humidity from an optional, external thermometer 
 * 
 */
void ETUpdate()  {

  #ifdef SHT30
    if(sht.readSample())  {
      ET_ERROR = 0;
      send(MsgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
      send(MsgTEMP.setSensor(ETT_ID).setDestination(0).set(sht.getTemperature(), 1));
      send(MsgHUM.setSensor(ETH_ID).set(sht.getHumidity(), 1));
      #ifdef HEATING_SECTION_SENSOR
        send(MsgTEMP.setSensor(ETT_ID).setDestination(MY_HEATING_CONTROLLER).set(sht.getTemperature(), 1));
      #endif
    }
    else  {
      ET_ERROR = 1;
      send(MsgSTATUS.setSensor(ETS_ID).set(ET_ERROR));
    }
  #endif
}

/**
 * @brief Updates CommonIO class objects; reads inputs & set outputs
 * 
 */
void UpdateIO() {

  for(int i=0; i<Iterations; i++) {
    CommonIO[i].CheckInput2(TOUCH_THRESHOLD, LONGPRESS_DURATION, DEBOUNCE_VALUE);
    
    if(CommonIO[i].NewState == CommonIO[i].State)  {
      continue;
    }
    
    // Binary states
    if(CommonIO[i].NewState != 2)  {
      // 2Relay Board
      if(HardwareVariant == 0)  {
        // Load: lighting
        if(LoadVariant != 2)  {
          CommonIO[i].SetRelay();
          AdjustLEDs(CommonIO[i].NewState, i);
          send(MsgSTATUS.setSensor(i).set(CommonIO[i].NewState));
        }
        // Load: roller shutter
        else  {
          MovementTime = Shutter.ReadButtons(i) * 1000;
          CommonIO[i].State = CommonIO[i].NewState;
        }
      }
      // RGBW Board
      else if(HardwareVariant == 1) {
        // Changing dimmer state (ON/OFF)
        if(i == 0 || (i == 1 && !Dimmer.CurrentState))  {          
          Dimmer.ChangeState(!Dimmer.CurrentState);
          AdjustLEDs(Dimmer.CurrentState, i);
          send(MsgSTATUS.setSensor(DIMMER_ID).set(Dimmer.CurrentState));
          CommonIO[i].State = CommonIO[i].NewState;
        }
        else if(i == 1 && Dimmer.CurrentState) {
          // Toggle dimming level by DIMMING_TOGGLE_STEP
          Dimmer.NewDimmingLevel += DIMMING_TOGGLE_STEP;

          Dimmer.NewDimmingLevel = Dimmer.NewDimmingLevel > 100 ? DIMMING_TOGGLE_STEP : Dimmer.NewDimmingLevel;
          send(MsgPERCENTAGE.setSensor(DIMMER_ID).set(Dimmer.NewDimmingLevel));
          CommonIO[i].NewState = CommonIO[i].State;
        }
      }
    }
    // Longpress
    else  {
      LongpressDetection++;
      
      #ifdef SPECIAL_BUTTON
        uint8_t SensorID = i == 0 ? SPECIAL_BUTTON_ID : SPECIAL_BUTTON_ID+1;
        send(MsgSTATUS.setSensor(SensorID).set(true));
      #endif

      CommonIO[i].NewState = CommonIO[i].State;
    }
    #ifdef RS485_DEBUG
      send(MsgCUSTOM.setSensor(TOUCH_DIAGNOSTIC_ID).set(IO[i].DebugValue));
    #endif
  }
}

/**
 * @brief Measures shutter movement duration; calls class Calibration() function to save measured durations
 * 
 * @param Vcc current uC voltage
 */
void ShutterCalibration(float Vcc)  {

  float Current = 0;
  uint16_t DownTimeCumulated = 0;
  uint16_t UpTimeCumulated = 0;
  uint32_t StartTime = 0;
  uint32_t StopTime = 0;
  uint32_t MeasuredTime = 0;

  // Indicate inactivity of buttons with builtin LEDs
  BlinkLEDs(3);

  // Open the shutter  
  Shutter.NewState = 0;
  Shutter.Movement();

  do  {
    delay(500);
    BlinkLEDs();
    wdt_reset();
    Current = PS.MeasureAC(Vcc);
  } while(Current > PS_OFFSET);

  Shutter.NewState = 2;
  Shutter.Movement();

  delay(1000);

  // Calibrating
  for(int i=0; i<CALIBRATION_SAMPLES; i++) {
    for(int j=1; j>=0; j--)  {
      Shutter.NewState = j;
      Shutter.Movement();
      StartTime = millis();

      do  {
        delay(250);
        BlinkLEDs();
        Current = PS.MeasureAC(Vcc);
        StopTime = millis();
        wdt_reset();
      } while(Current > PS_OFFSET);

      Shutter.NewState = 2;
      Shutter.Movement();

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

  Shutter.Position = 0;

  // Calculate movement durations
  uint8_t DownTime = (int)(DownTimeCumulated / CALIBRATION_SAMPLES);
  uint8_t UpTime = (int)(UpTimeCumulated / CALIBRATION_SAMPLES);

  // Pass movement durations to the object (add 1 s to each duration)
  Shutter.Calibration(UpTime+1, DownTime+1);

  EEPROM.put(EEA_SHUTTER_TIME_DOWN, DownTime);
  EEPROM.put(EEA_SHUTTER_TIME_UP, UpTime);
  EEPROM.put(EEA_SHUTTER_POSITION, Shutter.Position);

  // Inform Controller about the current state of roller shutter
  send(MsgSTOP.setSensor(SHUTTER_ID));
  send(MsgPERCENTAGE.setSensor(SHUTTER_ID).set(Shutter.Position));

  // Change LED indication to normal again
  SetLEDs();
}

/**
 * @brief Updates shutter condition, informs controller about shutter condition and position
 * 
 */
void ShutterUpdate(float Current) {

  uint32_t StopTime = 0;
  uint32_t MeasuredTime;
  uint8_t TempState = 2;
  bool Direction;

  if(Shutter.State != 2) {
    if((millis() >= StartTime + MovementTime) || (Current < PS_OFFSET)) {
      StopTime = millis();
    }
    else if(millis() < StartTime)  {
      uint32_t Temp = 4294967295 - StartTime + millis();
      wait(MovementTime - Temp);
      StartTime = 0;
      StopTime = MovementTime;
    }
  }

  if(Shutter.State != Shutter.NewState) {
    if(Shutter.NewState != 2)  {
      if(Shutter.State == 2)  {
        ShutterStart();
      }
      else  {
        TempState = Shutter.NewState;
        StopTime = millis();
      }
    }
    else  {
      StopTime = millis();
    }
  }

  if(StopTime > 0)  {
    Direction = Shutter.State;
    Shutter.NewState = 2;
    Shutter.Movement();
    SetLEDs();
    send(MsgSTOP.setSensor(SHUTTER_ID));

    MeasuredTime = StopTime - StartTime;
    Shutter.CalculatePosition(Direction, MeasuredTime);
    EEPROM.put(EEA_SHUTTER_POSITION, Shutter.Position);
  
    send(MsgPERCENTAGE.setSensor(SHUTTER_ID).set(Shutter.Position));
  
    if(TempState != 2)  {
      wait(500);
      Shutter.NewState = TempState;
      ShutterStart();
    }
  }
}

void ShutterStart() {

  Shutter.Movement();
  StartTime = millis();

  Shutter.NewState == 0 ? send(MsgUP.setSensor(SHUTTER_ID)) : send(MsgDOWN.setSensor(SHUTTER_ID));

  SetLEDs();

  wait(500);
}

/**
 * @brief Informs controller about power sensor readings
 * 
 * @param Current current measured by sensor 
 * @param Sensor sensor ID if more than one sensor is attached
 */
void PSUpdate(float Current, uint8_t Sensor = 0)  {

  if(Current == 0 && PS.OldValue == 0)  return;
  else if(Current < 1 && (abs(PS.OldValue - Current) < 0.1)) return;
  else if(Current >= 1 && (abs(PS.OldValue - Current) < (0.1 * PS.OldValue))) return;
  
  send(MsgWATT.setSensor(PS_ID).set(PS.CalculatePower(Current, COSFI), 0));
  PS.OldValue = Current;
}

/**
 * @brief Measures uC supply voltage
 * 
 * @return long measured voltage in mV
 */
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

/**
 * @brief main loop: calls all 'Update' functions, runs all measurements, checks if safety parameters are within limits
 * 
 */
void loop() {

  float Vcc = ReadVcc(); // mV
  float Current = 0;

  // Sending out states for the first time (as required by Home Assistant)
  if (!InitConfirm)  {
    InitConfirmation();
  }

  // Reading power sensor
  #ifdef POWER_SENSOR
    // 2Relay Board
    if(HardwareVariant == 0)  {
      if (digitalRead(RELAY_PIN_1) == RELAY_ON || digitalRead(RELAY_PIN_2) == RELAY_ON)  {
        Current = PS.MeasureAC(Vcc);
      }
    }
    // RGBW Board
    if(HardwareVariant == 1)  {
      if (Dimmer.CurrentState)  {
        Current = PS.MeasureDC(Vcc);
      }
    }

    PSUpdate(Current);
      
    #ifdef ERROR_REPORTING
      OVERCURRENT_ERROR = PS.ElectricalStatus(Current);
    #endif
  #endif

  // Current safety
  #if defined(ELECTRONIC_FUSE) && defined(POWER_SENSOR)
    if(OVERCURRENT_ERROR)  {
      // Current to high
      // Board: 2Relay 
      if(HardwareVariant == 0)  {
        // Load: Lighting
        if(LoadVariant < 2) {
          for (int i = RELAY_ID_1; i < RELAY_ID_1 + NUMBER_OF_RELAYS; i++)  {
            CommonIO[i].SetState(RELAY_OFF);
            CommonIO[i].SetRelay();
            send(MsgSTATUS.setSensor(i).set(RELAY_OFF));
          }
        }
        // Load: Roller shutter
        else if(LoadVariant == 2) {
          Shutter.NewState = 2;
          ShutterUpdate(0);
        }
      }
      // Board: RGBW
      else if(HardwareVariant == 1) {
        Dimmer.ChangeState(false);
        send(MsgSTATUS.setSensor(DIMMER_ID).set(Dimmer.CurrentState));
      }
      send(MsgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR));
      InformControllerES = true;
    }
    else if(!OVERCURRENT_ERROR && InformControllerES)  {
      // Current normal (only after reporting error)
      send(MsgSTATUS.setSensor(ES_ID).set(OVERCURRENT_ERROR));
      InformControllerES = false;
    }
  #endif

    // Reading inputs & adjusting outputs
  if(Iterations > 0)  {
    UpdateIO();
    if(LongpressDetection > 0)  {
      
      // Launch Longpress LED sequence
      AdjustLEDs(2, 0);

      // Adjust LEDs back to indicate the states of outputs
      SetLEDs();

      LongpressDetection = 0;
      
      #ifdef SPECIAL_BUTTON
        for(int i=SPECIAL_BUTTON_ID; i<SPECIAL_BUTTON_ID+2; i++)  {
          send(MsgSTATUS.setSensor(i).set(false));
        }
      #endif  
    }
    if(HardwareVariant == 0 && LoadVariant == 2)  {
      ShutterUpdate(Current);
    }
    else if(HardwareVariant == 1)  {
      Dimmer.UpdateDimmer();
    }
  }

  // Reset LastUpdate if millis() has overflowed
  if(LastUpdate > millis()) {
    LastUpdate = millis();
  }

  #ifdef TOUCH_AUTO_DIAGNOSTICS
    // Checking if touch feature works correctly
    TouchDiagnosis2();
  #endif
  
  // Checking out sensors which report at a defined interval
  if ((millis() > LastUpdate + INTERVAL) || CheckNow == true)  {
    #ifdef SHT30
      ETUpdate();
    #elif defined(POWER_SENSOR)
      PSUpdate(Current);
    #endif

    LimitTransgressions = 0;
    LastUpdate = millis();
    CheckNow = false;
  }

  wait(LOOP_TIME);
}
/*

   EOF

*/
