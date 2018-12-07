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
#define SN "RS485"
#define SV "1.0"

// Selecting radio type and transmission settings
#define MY_RS485                              // Enable RS485 transport layer
#define MY_RS485_DE_PIN A6                    // DE Pin definition
#define MY_RS485_BAUD_RATE 57600              // Set RS485 baud rate to use
//#define MY_RS485_HWSERIAL Serial1             // For RS485 connected to Hardware Serial port (0, 1)

// FOTA Feature
#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_SS 10
//#define OTA_WAIT_PERIOD 300

// Other
#define MY_TRANSPORT_WAIT_READY_MS 15000      // Time to wait for gateway to respond at startup
//#define MY_DEBUG                              // Enable debug prints to serial monitor
//#define MY_REPEATER_FEATURE                 // Enable repeater functionality for this node (wireless only)

/*  *******************************************************************************************
 *                           Custom Libraries Definitions
 *  *******************************************************************************************/
#define UI_SENSORS_NUMBER 2                   // UniversalInput Default 5  
//#define UI_PIN_SHIFT 2                        // UniversalInput Default 2
//#define RELAY_ON HIGH
//#define RELAY_OFF LOW
//#define MAX_TEMPERATURE 60                    // InternalTemp Default 60
//#define MAX_CURRENT 3                         // PowerSensor Default 3
//#define POWER_MEASURING_TIME 100              // PowerSensor Default 100
//#define TEMPRERATURE_SENSOR_TYPE MCP9700A
//#define POWER_SENSOR_TYPE ACS7125A

/*  *******************************************************************************************
 *                                  EEPROM Definitions
 *  *******************************************************************************************/
#define SIZE_OF_INT 2
#define EEPROM_OFFSET 512                   // First eeprom address to use (MySensors uses prior addresses)
#define EEA_RS_TIME_DOWN EEPROM_OFFSET      // EEPROM address to save RShutter travel down time
#define EEA_RS_TIME_UP EEA_RS_TIME_DOWN+SIZE_OF_INT   // EEPROM address to save RShutter travel up time
#define EEA_RS_POSITION EEA_RS_TIME_UP+SIZE_OF_INT    // EEPROM address to save RShutter last known position

/*  *******************************************************************************************
 *                                   Pin&ID Definitions
 *  *******************************************************************************************/
// Universal Input
#define RS_ID 0                             // RShutter ID
#define FIRST_RELAY_ID 0                    
#define RELAY_1 6                           // Pin of first relay (UPWARD DIRECTION!)
#define RELAY_2 7                           // Pin of DOWNWARD DIRECTION
#define NUMBER_OF_RELAYS 2                  // Board dependable

#define RELAY_ON HIGH
#define RELAY_OFF LOW

#define BUTTON_1 2                          // Usually 1 or 2 buttons, ready for 4 buttons, including 2 virtual buttons
#define BUTTON_2 3        
//#define V_BUTTON_1 ?                      // Additional/virtual buttons
//#define V_BUTTON_2 ?
//#define CHILD_ID_VB_1 2
//#define CHILD_ID_VB_2 3
//#define KONT_ID_1 4
//#define KONT_ID_2 5
#define SPECIAL_BUTTON_ID 6                 // Special button/buttons
//#define SPECIAL_BUTTON_ID_2 7

// Power Sensor
#define PS_ID 8                             // Power Sensor ID
#define PS_PIN A0                           // Power Sensor pin

// External Thermometer
//#define ETT_ID 9                          // External Thermometer temperature sensor ID
//#define ETH_ID 10                         // External Thermometer humidity sensor ID
//#define ET_PIN 10                         // External Thermometer pin
//#define ET_TIMER 3000                     // External Thermometer timer 

// Internal Thermometer
#define IT_ID 11                            // Internal Thermometer sensor ID
#define IT_PIN A7                           // Internal Thermometer pin
#define IT_TIMER 3000                       // Internal Thermometer timer

// Module Safety and Faultlessness Indicators
#define TS_ID 12                            // Thermal Status sensor ID
#define ES_ID 13                            // Electrical Status sensor ID
//#define ETS_ID 14                           // External Thermometer Status sensor ID

/*  *******************************************************************************************
 *                                  Other Definitions
 *  *******************************************************************************************/
#define INTERVAL 300000                     // Interval value for reporting certain sensors
#define LOOP_TIME 100                       // Main loop wait time
 
#endif
/*
 * 
 * EOF
 * 
 */
