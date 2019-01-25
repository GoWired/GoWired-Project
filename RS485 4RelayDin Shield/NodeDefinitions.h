/*
 * 
 * All definitions in one place
 * 
 */


#ifndef NodeDefinitions_h
#define NodeDefinitions_h

/*  *******************************************************************************************
 *                            MySensors Definitions
 *  *******************************************************************************************/
// Identification
#define MY_NODE_ID AUTO
#define SN "RS485 4RelayDIN Module"
#define SV "1.0"

// Selecting radio type and transmission settings
#define MY_RS485                              // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                    // DE Pin definition
#define MY_RS485_BAUD_RATE 57600              // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial              // Enable for Hardware Serial
#define MY_RS485_SOH_COUNT 3                  // Collision avoidance

// FOTA Feature
#define MY_OTA_FIRMWARE_FEATURE
//#define OTA_WAIT_PERIOD 300

// Other
#define MY_TRANSPORT_WAIT_READY_MS 60000      // Time to wait for gateway to respond at startup
//#define MY_DEBUG                              // Enable debug prints to serial monitor

/*  *******************************************************************************************
 *                           Custom Libraries Definitions
 *  *******************************************************************************************/
#define UI_SENSORS_NUMBER 4                   // UniversalInput Default 5
#define MAX_CURRENT 10                       // PowerSensor Default 3
//#define RELAY_ON HIGH
//#define RELAY_OFF LOW
//#define POWER_MEASURING_TIME 100            // PowerSensor Default 100

/*  *******************************************************************************************
 *                                  EEPROM Definitions
 *  *******************************************************************************************/
//#define EEPROM_OFFSET 512                   // First eeprom address to use (MySensors uses prior addresses)
//#define EEA_RS_TIME_DOWN EEPROM_OFFSET      // EEPROM address to save RShutter travel down time
//#define EEA_RS_TIME_UP EEA_RS_TIME_DOWN+1   // EEPROM address to save RShutter travel up time
//#define EEA_RS_POSITION EEA_RS_TIME_UP+1    // EEPROM address to save RShutter last known position

/*  *******************************************************************************************
 *                                   Pin&ID Definitions
 *  *******************************************************************************************/
// Universal Input
#define FIRST_RELAY_ID 0                    // Sensor ID of first relay
#define RELAY_1 5                            // Pin of first relay
#define RELAY_2 6
#define RELAY_3 9
#define RELAY_4 10
#define NUMBER_OF_RELAYS 4                  // Board dependable

#define RELAY_ON HIGH
#define RELAY_OFF LOW

// Power Sensors
#define PS1_ID 4                             // Power Sensor ID
#define PS1_PIN A0                           // Power Sensor pin
#define PS2_ID 5                             // Power Sensor ID
#define PS2_PIN A4                           // Power Sensor pin
#define PS3_ID 6                             // Power Sensor ID
#define PS3_PIN A5                           // Power Sensor pin
#define PS4_ID 7                             // Power Sensor ID
#define PS4_PIN A7                           // Power Sensor pin

// Module Safety and Faultlessness Indicators
#define ES_ID 9                            // Electrical Status sensor ID

/*  *******************************************************************************************
 *                                  Other Definitions
 *  *******************************************************************************************/
#define INTERVAL 60000                     // Interval value for reporting certain sensors
#define LOOP_TIME 100                       // Main loop wait time
 
#endif
/*
 * 
 * EOF
 * 
 */
