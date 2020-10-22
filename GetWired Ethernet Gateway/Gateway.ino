/**
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
 * version 3 as published by the Free Software Foundation. *
 *
 * DESCRIPTION
 * The Ethernet Gateway sends data received from sensors to the ethernet link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the RS485 network.
 * Hardware serial is used with baud rate of 57600 by default.
 *
 * The code is designed for GetWired Ethernet Gateway
 *
 * LED purposes:
 * - RX - blink fast on radio message received
 * - TX - blink fast on radio message transmitted
 * - ERR - fast blink on error during transmission error or receive crc error
 * - CONF - turns on after pressing CONF button
 * 
 * Buttons:
 * - CONF - turns of bus power supply, cuts the power to any modules connected to the gateway.
 *
 */


#define MY_RS485                                          // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                                 // Define this to enables DE-pin management on defined pin
#define MY_RS485_BAUD_RATE 57600                          // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial                          // Enable for Hardware Serial
#define MY_RS485_SOH_COUNT 3                              // Use this in case of collisions on the bus

#define MY_GATEWAY_ENC28J60                               // Enable gateway ethernet module type 
#define MY_IP_ADDRESS 192,168,8,7                        // Gateway IP address
#define MY_PORT 5003                                      // The port to keep open on node server mode / or port to contact in client mode
#define MY_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB // Gateway MAC address

// Controller ip address. Enables client mode (default is "server" mode). 
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere. 
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 254   

//#define MY_LEDS_BLINKING_FEATURE                        // Flash leds on rx/tx/err
#define MY_WITH_LEDS_BLINKING_INVERSE                     // Define to inverse LED behaviour
#define MY_DEFAULT_LED_BLINK_PERIOD 10                    // Set blinking period
#define MY_DEFAULT_ERR_LED_PIN 4                          // Error led pin
#define MY_DEFAULT_RX_LED_PIN  5                          // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  6                          // Transmit led pin

//#define MY_DEBUG                                        // Enable debug prints to serial monitor

// Includes
#include <UIPEthernet.h>
#include <MySensors.h>

// Definitions
#define CONF_BUTTON A0
#define CONF_LED 9
#define BUS_RELAY 8

// Globals
bool ButtonState = false;
bool ButtonHigh = false;
unsigned long TIME_1;

void before() {

  pinMode(CONF_BUTTON, INPUT_PULLUP);
  pinMode(BUS_RELAY, OUTPUT);
  pinMode(CONF_LED, OUTPUT);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  digitalWrite(BUS_RELAY, 0);
  digitalWrite(CONF_LED, 0);
  ButtonState = 0;

  digitalWrite(4, 1);
  digitalWrite(5, 1);
  digitalWrite(6, 1);
  digitalWrite(CONF_LED, 1);
  delay(500);
  digitalWrite(4, 0);
  delay(500);
  digitalWrite(5, 0);
  delay(500);
  digitalWrite(6, 0);
  delay(500);
  digitalWrite(CONF_LED, 0);
  delay(500);
  digitalWrite(4, 1);
  digitalWrite(5, 1);
  digitalWrite(6, 1);
  digitalWrite(CONF_LED, 1);
  delay(500);
  digitalWrite(4, 0);
  digitalWrite(5, 0);
  digitalWrite(6, 0);
  digitalWrite(CONF_LED, 0);
}

void setup()  {

}

void loop() {

  if(!ButtonHigh)  {
    if(digitalRead(CONF_BUTTON))  {
      ButtonHigh = true;
    }
  }

  if(!digitalRead(CONF_BUTTON) && ButtonHigh) {
    TIME_1 = millis();
    ButtonHigh = false;
    while(!digitalRead(CONF_BUTTON)) {
      wait(251);
      if(millis() - TIME_1 > 500) {
        ButtonState = !ButtonState;
        digitalWrite(BUS_RELAY, ButtonState);
        digitalWrite(CONF_LED, ButtonState);
        break;
      }
    }
  }
  
  wait(100);
}
/*
 * End of file
 */
