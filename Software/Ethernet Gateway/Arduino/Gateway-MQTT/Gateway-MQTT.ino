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
 * This is source code for GoWired Ethernet Gateway.
 *
 */


#define MY_RS485                                          // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                                 // Define this to enables DE-pin management on defined pin
#define MY_RS485_BAUD_RATE 57600                          // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial                          // Enable for Hardware Serial
#define MY_RS485_SOH_COUNT 3                              // Use this in case of collisions on the bus

#define MY_GATEWAY_ENC28J60                               // Enable gateway ethernet module type 
#define MY_IP_ADDRESS 192,168,8,7                        // Gateway IP address
#define MY_PORT 1883                                      // The port to keep open on node server mode / or port to contact in client mode
#define MY_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB // Gateway MAC address

#define MY_GATEWAY_MQTT_CLIENT
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "gwo"
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gwi"
#define MY_MQTT_CLIENT_ID "G1"

// Controller ip address. Enables client mode (default is "server" mode). 
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_URL_ADDRESS "your-controller-url"
#define MY_CONTROLLER_IP_ADDRESS 192,168,8,8  

#define MY_WITH_LEDS_BLINKING_INVERSE                     // Define to inverse LED behaviour
#define MY_DEFAULT_LED_BLINK_PERIOD 10                    // Set blinking period
#define MY_DEFAULT_ERR_LED_PIN 4                          // Error led pin
#define MY_DEFAULT_RX_LED_PIN  5                          // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  6                          // Transmit led pin

//#define MY_DEBUG                                        // Enable debug prints to serial monitor

/* 
 * WATCHDOG & CONTROLLER UPLINK CHECK
 * Watchdog resets the gateway in case of any software hang. Enabling it should result in
 * more robustness and long term reliability.
 * Uplink Check tests the connection between the Gateway and the Controller.
 * In case of connection loss, gateway will be reset be watchdog. 
 * Time interval between tests can be customized.
 * Use CONTROLLER UPLINK CHECK only together with WATCHDOG.
 */
#define ENABLE_WATCHDOG                                   // Resets the Gateway in case of any software hang
#define ENABLE_UPLINK_CHECK                               // Resets the Gateway in case of connection loss with the controller
#define UPLINK_CHECK_INTERVAL 60000                       // Time interval for the uplink check (default 60000)

// Includes
#include <UIPEthernet.h>
#include <MySensors.h>
#include <avr/wdt.h>

// Definitions
#define CONF_BUTTON A0
#define CONF_LED 9
#define BUS_RELAY 8

// Globals
bool ButtonState = false;
bool ButtonHigh = false;
bool CheckControllerUplink = true;
uint32_t TIME_1 = 0;
uint32_t LastUpdate = 0;

void before() {

  #ifdef ENABLE_WATCHDOG
    wdt_reset();
    MCUSR = 0;
    wdt_disable();
  #endif
  
  pinMode(CONF_BUTTON, INPUT_PULLUP); 
  pinMode(BUS_RELAY, OUTPUT); digitalWrite(BUS_RELAY, 0);

  uint8_t TEMP[4] = {MY_DEFAULT_ERR_LED_PIN, MY_DEFAULT_RX_LED_PIN, MY_DEFAULT_TX_LED_PIN, CONF_LED};

  for(int i=0; i<4; i++)  {
    pinMode(TEMP[i], OUTPUT); digitalWrite(TEMP[i], HIGH);
  }

  delay(500);

  for(int i=0; i<4; i++)  {
    digitalWrite(TEMP[i], LOW); delay(500);
  }

  for(int i=0; i<4; i++)  {
    digitalWrite(TEMP[i], HIGH);
  }

  delay(500);

  for(int i=0; i<4; i++)  {
    digitalWrite(TEMP[i], LOW);
  }
}

void setup()  {

  #ifdef ENABLE_WATCHDOG
    wdt_enable(WDTO_4S);
  #endif
  
}

void loop() {

  wait(500);

  #ifdef ENABLE_UPLINK_CHECK
    if((millis() > LastUpdate + UPLINK_CHECK_INTERVAL) && CheckControllerUplink) {
      if(!requestTime())  {
        delay(10000);
      }
      LastUpdate = millis();
    }
  #endif

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
        
        CheckControllerUplink = ButtonState == 1 ? false : true;
        
        break;
      }
    }
  }
}
/*
 * End of file
 */
