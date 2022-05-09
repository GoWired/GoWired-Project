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
 * This is source code for GoWired Touch MCU Raw boards (no communication 
 * variant of GoWired Touch).
 * 
 */

/***** INCLUDES *****/
#include <avr/wdt.h>
#include <EEPROM.h>
#include "Configuration.h"
#include <GoWired.h>


/***** Globals *****/
// 1 - 1 output; 2 - 2 outputs
uint8_t HardwareVariant;                    // Variant of connected hardware: 0 - Power Board AC, 1 - Power Board DC
uint8_t LoadVariant;                        // Variant of connected load: 1 - One input/output; 2 - Two inputs/outputs
bool Shutter = false;                 // false - lighting; true - roller shutter
bool Monostable = false;
bool RememberStates = false;

uint32_t LastCheck = 0;

uint32_t RSTimer;
bool RSReset = false;

uint16_t EPPROM_Address[2] = {EEA_RELAY_1, EEA_RELAY_2};

/***** Constructors *****/
CommonIO IO[NUMBER_OF_BUTTONS];

Dimmer D[NUMBER_OF_BUTTONS];

/**
 * @brief Setups software components: wdt, check & initialize hardware, initialize LEDs
 * 
 */
void setup()  {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif

  delay(2000);

  // Resistive hardware detection

  uint16_t ReadHardware = analogRead(HARDWARE_DETECTION_PIN);

  if(ReadHardware < 20) {
    // Hardware variant: Power Board AC
    HardwareVariant = 0;
  }
  else if(ReadHardware < 50)  {
    // Hardware variant: Power Board DC
    HardwareVariant = 1;
  }
  else  {/* Handling error */}

  // Reading dip switches
  pinMode(DIP_SWITCH_1, INPUT_PULLUP);
  pinMode(DIP_SWITCH_2, INPUT_PULLUP);
  pinMode(DIP_SWITCH_3, INPUT_PULLUP);
  pinMode(DIP_SWITCH_4, INPUT_PULLUP);

  delay(100);

  // Reading dip switch 1
  if(!digitalRead(DIP_SWITCH_1)) {
    LoadVariant = 1;
  }
  else  {
    LoadVariant = 2;
  }

  // Reading dip switch 2
  if(!digitalRead(DIP_SWITCH_2)) {
    Monostable = true;
  }

  // Reading dip switch 3
  if(!digitalRead(DIP_SWITCH_3)) {
    if(HardwareVariant == 0)  {
      Shutter = true;
    }
  }

  // Reading dip switch 4
  if(!digitalRead(DIP_SWITCH_4)) {
    if(!Monostable && !Shutter) {
      RememberStates = true;
      uint8_t RecoveredState;
      for(int i=0; i<HardwareVariant; i++)  {
        EEPROM.get(EPPROM_Address[i], RecoveredState);
        if(RecoveredState < 2)  {
          IO[i].SetState(RecoveredState);
        }
      }      
    }
  }

  // One button variant
  if(LoadVariant == 1)  {
    // Initialize LEDs
    D[0].SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_7, LED_PIN_8, LED_PIN_9);

    // Show initialization
    RainbowLED(INIT_RAINBOW_DURATION, INIT_RAINBOW_RATE);

    // Initializing and calibrating button
    if(HardwareVariant == 0) {
      IO[0].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_3, INPUT_PIN_1, RELAY_PIN_1);
    }
    else if(HardwareVariant == 1) {
      IO[0].SetValues(RELAY_OFF, false, 7, TOUCH_FIELD_3, RELAY_PIN_1);
    }

    // Restore saved state
    if(RememberStates)  {
      IO[0].SetRelay();
      AdjustLEDs(IO[0].NewState, 0);
    }
    else  {
      // Turn on LED
      AdjustLEDs(false, 0);
    }
  }
  // Two buttons variant
  else if(LoadVariant == 2) {
    // Initialize LEDs
    D[0].SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_1, LED_PIN_2, LED_PIN_3);
    D[1].SetValues(NUMBER_OF_CHANNELS, DIMMING_STEP, DIMMING_INTERVAL, LED_PIN_4, LED_PIN_5, LED_PIN_6);

    // Show initialization
    RainbowLED(INIT_RAINBOW_DURATION, INIT_RAINBOW_RATE);
    
    // Initializing and calibrating buttons
    if(HardwareVariant == 0) {
      IO[0].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_1, INPUT_PIN_1, RELAY_PIN_1);
      IO[1].SetValues(RELAY_OFF, false, 6, TOUCH_FIELD_2, INPUT_PIN_2, RELAY_PIN_2);
    }
    else if(HardwareVariant == 1) {
      IO[0].SetValues(RELAY_OFF, false, 7, TOUCH_FIELD_1, RELAY_PIN_1);
      IO[1].SetValues(RELAY_OFF, false, 7, TOUCH_FIELD_2, RELAY_PIN_2);
    }

    // Restore saved states
    if(RememberStates)  {
      for(int i=0; i<2; i++)  {
        IO[i].SetRelay();
        AdjustLEDs(IO[i].NewState, i);
      }
    }
    else  {
      // Turning on LEDs
      for(int i=0; i<2; i++)  {
        AdjustLEDs(false, i);
      }
    }
  }

  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_4S);
  #endif
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

    for(int i=1; i<=LoadVariant; i++)  {
      D[i-1].UpdateLEDs(BRIGHTNESS_VALUE_ON, RValue, GValue, BValue);
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
    delay(Rate);
  }
}

/**
 * @brief Adjusts built-in LEDs to current state of buttons 
 * 
 * @param State state of button
 * @param Dimmer number of dimmer
 */
void AdjustLEDs(bool State, uint8_t Dimmer) {

  if(State != 1) {
    D[Dimmer].UpdateLEDs(BRIGHTNESS_VALUE_OFF, R_VALUE_OFF, G_VALUE_OFF, B_VALUE_OFF);
  }
  else  {
    D[Dimmer].UpdateLEDs(BRIGHTNESS_VALUE_ON, R_VALUE_ON, G_VALUE_ON, B_VALUE_ON);
  }
}

/**
 * @brief Updates CommonIO class objects; reads inputs & set outputs
 * 
 */
void UpdateIO() {

  for(int i=0; i<LoadVariant; i++)  {
    IO[i].CheckInput3(TOUCH_THRESHOLD, DEBOUNCE_VALUE, Monostable);

    if(IO[i].NewState == IO[i].State) {
      continue;
    }
    
    if(Shutter) {
      if(IO[i].State)  {
        // Stop
        for(int j=0; j<LoadVariant; j++)  {
          IO[j].SetState(0);            
          IO[j].SetRelay();
          AdjustLEDs(false, j);
        }
      }
      else  {
        IO[i].SetRelay();
        AdjustLEDs(IO[i].NewState, i);
        if(IO[i].NewState)  {
          RSTimer = millis();
          RSReset = true;
        }
      }
    }
    else  {
      if(!Monostable) {
        IO[i].SetRelay();
        AdjustLEDs(IO[i].NewState, i);
      }
      // Saving state to eeprom
      //EEPROM.put(EPPROM_Address[i], IO[i].NewState);
    }
  }
}

/**
 * @brief main loop: resets wdt, updates inputs & outputs
 * 
 */
void loop() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
  #endif

  // Check inputs & adjust outputs
  if(millis() > LastCheck + LOOP_TIME)  {
    LastCheck = millis();
    UpdateIO();
  }
  else if(millis() < LastCheck) {
    LastCheck = millis();
  }

  // Roller shutter timer  
  if(Shutter == true) {
    if((millis() > RSTimer + RS_INTERVAL) && RSReset)  {
      for(int i=0; i<2; i++)  {
        IO[i].SetState(0);
        IO[i].SetRelay();
        AdjustLEDs(false, i);
      }
      RSReset = false;
    }
  }
}
