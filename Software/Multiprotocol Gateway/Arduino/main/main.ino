/**
 * GoWired is an open source project for WIRED home automation. It aims at making wired
 * home automation easy and affordable for every home automation enthusiast. GoWired provides:
 * - hardware (https://www.crowdsupply.com/domatic/getwired),
 * - software (https://github.com/feanor-anglin/GetWired-Project/releases), 
 * - 3D printable enclosures (https://github.com/feanor-anglin/GetWired-Project/tree/master/Enclosures),
 * - instructions (both Crowd Supply campaign page / campaign updates and our GitHub wiki).
 * 
 * GoWired is based on RS485 industrial communication standard. The software is an implementation
 * of MySensors communication protocol (http://www.mysensors.org). 
 *
 * Created by feanor-anglin
 * Copyright (C) 2018-2021 feanor-anglin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * DESCRIPTION
 * GoWired Multiprotocol Gateway translates data received from devices connected through various 
 * communication standards (RS485, CAN, USB) and protocols to the Ethernet link.
 * 
 */

// Includes
#include "Configuration.h"
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <INA219_WE.h>
#include <MCP23S17.h>
#include <WDTZero.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "bitmaps/Bitmaps128x250.h" // 2.13" b/w
#include <MySensors.h>

// Globals
bool ButtonState = false;
bool ButtonHigh = false;
bool CheckControllerUplink = true;
uint32_t TIME_1 = 0;
uint32_t LastUpdate = 0;
uint32_t LEDsLastUpdate = 0;

// Constructors
// WEB Frontend
#ifdef WEBFRONTEND
  EthernetServer server(HTTP_PORT);
#endif

// SPI Expander
MCP23S17 Expander(&SPI2, EXPANDER_CS_PIN, 0);

// Current sensor and shunt used 
INA219_WE ina219 = INA219_WE(INA219_ADDRESS);

// Watchdog
#ifdef ENABLE_WATCHDOG
  WDTZero WDT;
#endif

// Before
void before() {

  // Start USB serial
  Serial2.begin(115200);

  // Set pin modes, set bus relay off
  pinMode(BUS_RELAY, OUTPUT); digitalWrite(BUS_RELAY, 0);
  pinMode(EEPROM_CS_PIN, OUTPUT);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(EXPANDER_CS_PIN, OUTPUT);

  // Obtain MAC address from EEPROM
  // Deselect all other SPI devices
  digitalWrite(SD_CS_PIN, HIGH);
  digitalWrite(EXPANDER_CS_PIN, HIGH);

  // Enable SPI1 (MCP25625)
  SPI1.begin();

  // Start SPI2 (SD card, 25AA02E48, MCP23S17, display)
  SPI2.begin();

  // Start MCP23S17 expander
  //Expander.begin();

  #ifdef EUI48_EEPROM
    // Set EEPROM_CS_PIN to output
    Expander.pinMode(EEPROM_CS_PIN, OUTPUT);

    uint8_t result = 0;
  
    // Take the chip select low to select the device
    Expander.digitalWrite(EEPROM_CS_PIN, LOW);
    // Read status
    SPI2.transfer(READ_STATUS_INSTRUCTION);
    result = SPI2.transfer(0x00);
  
    if(result != 0) {
      SPI2.transfer(READ_INSTRUCTION);
	    // Send the device the register you want to read
	    SPI2.transfer(0xFA);
	    for(uint8_t i = 0; i < 6; i++)  {
        #ifdef MQTT_GATEWAY
          _MQTT_clientMAC[i] = SPI2.transfer(0x00);
        #else
          _ethernetGatewayMAC[i] = SPI2.transfer(0x00);
        #endif
  	  }
    }
  #endif /* EUI48_EEPROM */

  Expander.digitalWrite(EEPROM_CS_PIN, HIGH);

  /*// Print MAC address to USB
  //Serial2.print("MAC Address: ");
  for(int i = 0; i < 6; i++)  {
    if(eui48[i] < 16) Serial2.print("0"); // Padding
    Serial2.print(MY_MAC_ADDRESS[i],HEX);
    if(i < 5) {
      //Serial2.print(":");      
    }
  }
  //Serial2.println();*/
}

void setup()  {

  uint8_t TEMP[5] = {LED1, LED2, LED3, LED4, LED5};

  Expander.pinMode(RELAY_BUTTON, INPUT_PULLUP);

  // Launch LED sequence on enclosure panel
  for(int i=0; i<5; i++)  {
    Expander.pinMode(TEMP[i], OUTPUT); Expander.digitalWrite(TEMP[i], HIGH);  delay(500);
  }

  delay(500);

  for(int i=0; i<5; i++)  {
    Expander.digitalWrite(TEMP[i], LOW); delay(500);
  }

  for(int i=0; i<5; i++)  {
    Expander.digitalWrite(TEMP[i], HIGH);
  }

  delay(500);

  for(int i=0; i<5; i++)  {
    Expander.digitalWrite(TEMP[i], LOW);
  }

  // INA219
  Wire.begin();
  if(!ina219.init())  {
    // Inform the user INA219 is not working
  }
  //ina219.setADCMode(SAMPLE_MODE_128); // choose mode and uncomment for change of default
  //ina219.setMeasureMode(CONTINUOUS); // choose mode and uncomment for change of default
  //ina219.setPGain(PG_320); // choose gain and uncomment for change of default
  //ina219.setBusRange(BRNG_32); // choose range and uncomment for change of default
  //ina219.setCorrectionFactor(0.98); // insert your correction factor if necessary

  // Initialize webserver
  #ifdef WEBFRONTEND
    server.begin();
  #endif

  // Initialize e-ink display

  // Enable SAMD watchdog
  #ifdef ENABLE_WATCHDOG
    WDT.setup(WDT_HARDCYCLE2S);
  #endif

}

void loop() {

  wait(500);

  #ifdef ENABLE_UPLINK_CHECK
    if((millis() > LastUpdate + UPLINK_CHECK_INTERVAL) && CheckControllerUplink) {
      if(Ethernet.linkStatus() != LinkON)  {
        digitalWrite(LAN_RESET, HIGH); wait(100);
        digitalWrite(LAN_RESET, LOW); wait(1000);
        #ifdef ENABLE_WATCHDOG
          WDT.clear();
        #endif
        if(Ethernet.linkStatus() != LinkON)  {
          NVIC_SystemReset();
        }
      }
      LastUpdate = millis();
    }
  #endif

  if(!ButtonHigh)  {
    if(Expander.digitalRead(RELAY_BUTTON))  {
      ButtonHigh = true;
    }
  }

  if(!Expander.digitalRead(RELAY_BUTTON) && ButtonHigh) {
    TIME_1 = millis();
    ButtonHigh = false;
    while(!Expander.digitalRead(RELAY_BUTTON)) {
      wait(251);
      if(millis() - TIME_1 > 500) {
        ButtonState = !ButtonState;
        
        digitalWrite(BUS_RELAY, ButtonState);
        Expander.digitalWrite(RELAY_LED, ButtonState);
        
        CheckControllerUplink = ButtonState == 1 ? false : true;
        
        break;
      }
    }
  }

  // INA219
  /*
    float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0;
  bool ina219_overflow = false;
  
  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
  ina219_overflow = ina219.getOverflow();
  */

  WebFrontend();    

  #ifdef ENABLE_WATCHDOG
    WDT.clear();
  #endif
}

/*
 * HTTP frontend
 */
// Show frontend
void WebFrontend() {
  
  #ifdef WEBFRONTEND
  // listen for incoming clients
  EthernetClient client = server.available();
  if(client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          PrintConfig(client, "<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    wait(10);
    // close the connection:
    client.stop();
    Serial.println("Client disconnected");
  }
  #endif
}

// Print IP Address
void PrintIpAddress(Stream &s, uint8_t a, uint8_t b, uint8_t c, uint8_t d ) {

  uint8_t TEMP[4] = {a, b, c, d};

  for(int i=0; i<4; i++)  {
    s.print(TEMP[i]);
    if(i != 3)  {
      s.print(".");
    }
  }
}

// Print MAC Address
void PrintMacAddress(Stream &s, uint8_t *MACAddr)  {

  for(int i=0; i<6; i++)  {
    s.print(MACAddr[i], HEX);
    if(i != 5)  {
      s.print(":");
    }
  }
}

// Print configuration
void PrintConfig(Stream &s, char *lineBreak)  {

  s.print(F("***********************************************"));
  s.print(lineBreak);
  s.print(SN);
  s.print(lineBreak);
  s.print(F("Version "));  s.print(SV);
  s.print(lineBreak);
  s.print(F("MAC address: "));
  #ifdef MQTT_GATEWAY
    PrintMacAddress(s, _MQTT_clientMAC);
  #else
    PrintMacAddress(s, _ethernetGatewayMAC);
  #endif  
  s.print(lineBreak);
  s.print(F("IP address: "));
  #ifdef MY_IP_ADDRESS
    PrintIpAddress(s, MY_IP_ADDRESS);
  #else
    s.print(F("DHCP"));
  #endif
  s.print(lineBreak);
  s.print(F("*** MYSENSORS ***"));
  #ifdef MQTT_GATEWAY
    s.print(F("Gateway type: MQTT"));
    s.print(lineBreak);
    s.print(F("MQTT controller address: "));
    printIpAddress(s, MY_CONTROLLER_IP_ADDRESS);
    s.print(lineBreak);
    s.print(F("MQTT controller port: "));
    s.print(MY_MQTT_PORT);
    s.print(lineBreak);
    s.print(F("MQTT client id: "));
    s.print(MY_MQTT_CLIENT_ID);
    s.print(lineBreak);
    s.print(F("MQTT publish prefix: "));
    s.print(MY_MQTT_PUBLISH_TOPIC_PREFIX);
    s.print(lineBreak);
    s.print(F("MQTT subscribe prefix: "));
    s.print(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
    s.print(lineBreak);
  #else
    s.print(F("Gateway type: ETHERNET"));
    s.print(lineBreak);
    s.print(F("Network port: "));
    s.print(MY_ETHERNET_PORT);
    s.print(lineBreak);
  #endif
  s.print(F("Communication standard: "));
  #ifdef MY_RS485
    s.print(F("RS485"));
    s.print(lineBreak);
  #else
    s.print(F("Undefined"));
  #endif  
  s.print ( lineBreak );
}

/*
 * LEDs handler
 */
void indication(const indication_t ind)  {

  #if defined(MY_DEFAULT_TX_LED_PIN)
	  if ((INDICATION_TX == ind) || (INDICATION_GW_TX == ind)) {
		  Expander.digitalWrite(MY_DEFAULT_TX_LED_PIN, LOW);
      wait(MY_DEFAULT_LED_BLINK_PERIOD);
      Expander.digitalWrite(MY_DEFAULT_TX_LED_PIN, HIGH);
	  } 
  #endif
  #if defined(MY_DEFAULT_RX_LED_PIN)
		if ((INDICATION_RX == ind) || (INDICATION_GW_RX == ind)) {
			Expander.digitalWrite(MY_DEFAULT_RX_LED_PIN, LOW);
      wait(MY_DEFAULT_LED_BLINK_PERIOD);
      Expander.digitalWrite(MY_DEFAULT_RX_LED_PIN, HIGH);
		}
  #endif
  #if defined(MY_DEFAULT_ERR_LED_PIN)
		if (ind > INDICATION_ERR_START) {
			Expander.digitalWrite(MY_DEFAULT_RX_LED_PIN, LOW);
      wait(MY_DEFAULT_LED_BLINK_PERIOD);
      Expander.digitalWrite(MY_DEFAULT_RX_LED_PIN, HIGH);
		}
  #endif
}

/*
 * End of file
 */
