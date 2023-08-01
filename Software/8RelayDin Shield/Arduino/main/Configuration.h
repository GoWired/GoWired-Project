/*
 * 
 * All definitions in one file
 * 
 */


#ifndef Configuration_h
#define Configuration_h

/*  *******************************************************************************************
 *                            MySensors Definitions
 *  *******************************************************************************************/
// Identification
#define MY_NODE_ID AUTO
#define MN "GW-8RD"
#define FV "2.1"

// Selecting transmission settings
#define MY_RS485                              // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                     // DE Pin definition
#define MY_RS485_BAUD_RATE 57600              // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial              // Enable for Hardware Serial
#define MY_RS485_SOH_COUNT 3                  // Collision avoidance

// FOTA Feature
#define MY_OTA_FIRMWARE_FEATURE

// Other
#define MY_TRANSPORT_WAIT_READY_MS 60000      // Time to wait for gateway to respond at startup

/*  *******************************************************************************************
 *                                   General Definitions
 *  *******************************************************************************************/
// Relay states
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

#define ENABLE_WATCHDOG

#define INTERVAL 300000                       // Interval value (ms) for reporting readings of the sensors: temperature, power usage (default 300000)
#define INIT_DELAY 200                        // A value (ms) to be multiplied by node ID value to obtain the time to wait during the initialization process
#define PRESENTATION_DELAY 10                 // Time (ms) to wait between subsequent presentation messages (default 10)
#define LOOP_TIME 100                         // Main loop wait time (ms); (default 100)

/*  *******************************************************************************************
 *                                   IO Config
 *  *******************************************************************************************/
#define FIRST_OUTPUT_ID 0                     // default 0; should not be altered (expander pins for outputs: 0-7)
#define TOTAL_NUMBER_OF_OUTPUTS 8             // Total number of outputs; value from 0-8 (default for 8RelayDin Shield 8; do not change it with this shield)

// Inputs not bound to any outputs (expander pins for inputs: 8-15)
#define INDEPENDENT_IO 4                      // Number of independent inputs and outputs; value from 0 to 8 (default 0)
#define INPUT_TYPE 0                          // Define input type for independent inputs: 0 - INPUT_PULLUP, 1 - INPUT, 3 - Button

#define NUMBER_OF_OUTPUTS TOTAL_NUMBER_OF_OUTPUTS-INDEPENDENT_IO

#define SPECIAL_BUTTON                        // Enables long press functionality for all buttons

#define INVERT_BUTTON_LOGIC false             // Invert logic of relay-related inputs 
#define INVERT_INPUT_LOGIC true               // Invert logic of independend inputs

/*  *******************************************************************************************
 *                                   MCU Pin Definitions
 *  *******************************************************************************************/
// OUTPUT [RELAY / RGBW]
#define OUTPUT_PIN_1 5
#define OUTPUT_PIN_2 9
#define OUTPUT_PIN_3 6
#define OUTPUT_PIN_4 10

// INPUT [BUTTON / SENSOR]
// General input
#define INPUT_PIN_1 2
#define INPUT_PIN_2 3
#define INPUT_PIN_3 4
#define INPUT_PIN_4 A3

// Analog input
#define INPUT_PIN_5 A1
#define INPUT_PIN_6 A2
#define INPUT_PIN_7 A6
#define INPUT_PIN_8 A7

// Protocols
// 1-wire
#define ONE_WIRE_PIN A0

// I2C
#define I2C_PIN_1 A4
#define I2C_PIN_2 A5

/*  *******************************************************************************************
 *                                   ERROR REPORTING
 *  *******************************************************************************************/
//#define ERROR_REPORTING
#ifdef ERROR_REPORTING
  #ifdef POWER_SENSOR
    #define ES_ID HYSTERESIS_ID+1
  #endif
  #ifdef INTERNAL_TEMP
    #define TS_ID ES_ID+1
  #endif
  #ifdef EXTERNAL_TEMP
    #define ETS_ID TS_ID+1
  #endif
#endif

//#define RS485_DEBUG
#ifdef RS485_DEBUG
  #define DEBUG_ID ETS_ID+1
#endif
 
#endif
/*
 * 
 * EOF
 * 
 */
