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
#define MY_NODE_ID 101
#define SN "GetWired HC2"
#define SV "1.0"

// Selecting transmission settings
#define MY_RS485                              // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                     // DE Pin definition
#define MY_RS485_BAUD_RATE 250000              // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial              // Enable for Hardware Serial
#define MY_RS485_SOH_COUNT 3                  // Collision avoidance

// FOTA Feature
#define MY_OTA_FIRMWARE_FEATURE

// Other
#define MY_TRANSPORT_WAIT_READY_MS 60000      // Time to wait for gateway to respond at startup

/*  *******************************************************************************************
 *                                      Various Definitions
 *  *******************************************************************************************/
// Shield - choose only 1
// Define this if your shield utilizes an I2C Expander (Heating Shield, I/O Shield, etc.)
#define EXPANDER_SHIELD

// Define this if your shield controls outputs directly, by uC pins (2SSR Shield - up to 2 sections, 4RelayDin Shield - up to 4 sections)                                 
//#define DIRECT_SHIELD                                 

// Relays
#define RELAY_ON HIGH
#define RELAY_OFF LOW

// Sections and setpoints
#define HEATING_SECTIONS 5                              // Number of heating sections (default 8, available 1-8)
#define DEFAULT_DAY_SP 20                               // Default day setpoint temperature (default 20)
#define DEFAULT_NIGHT_SP 18                           // Default night setpoint (default 18)
#define DEFAULT_HOLIDAY_SP 10                         // Default holiday setpoint (default 10)
#define DEFAULT_HYSTERESIS 0                            // Default hysteresis (default 0)
   
// IDs
#define FIRST_SECTION_ID 0                              // Sensor ID of first section
//#define FIRST_SPD_ID FIRST_SECTION_ID+HEATING_SECTIONS    // Sensor ID of first Thermostat 
//#define FIRST_TSP_ID FIRST_SECTION_ID+HEATING_SECTIONS      // Sensor ID for the first 'Temperature Sensor Passthrough'
//#define MASTER_SWITCH_ID FIRST_TSP_ID+HEATING_SECTIONS  // Sensor ID for Heating Master Switch
#define SELECTOR_SWITCH_ID FIRST_SECTION_ID+HEATING_SECTIONS           // Selector switch ID: ON/OFF / Day/Night/Holiday
#define SPN_ID SELECTOR_SWITCH_ID+1                     // Night set point ID (one for all sections)
#define SPH_ID SPN_ID+1                                 // Holiday set point ID (one for all sections)
#define HYSTERESIS_ID SPH_ID+1                          // Hysteresis ID

// Section temperature sensors IDs
#define T_ID_1 31                     
#define T_ID_2 35
#define T_ID_3 37
#define T_ID_4 38
#define T_ID_5 40
#define T_ID_6 51
#define T_ID_7 52
#define T_ID_8 53

// I2C Sensors
//#define INTERNAL_TEMP
#ifdef INTERNAL_TEMP
  #define ITT_ID HYSTERESIS_ID+1
  #define ITH_ID ITT_ID+1
  #define IT_STATUS_ID ITH_ID+1
#endif

// Main loop definitions
#define INTERVAL 300000                    // Interval value for reporting readings of the sensors: temperature, power usage (default 300000)
#define INIT_DELAY 200                       // A value to be multiplied by node ID value to obtain the time to wait during the initialization process
#define PRESENTATION_DELAY 10       // Time (ms) to wait between subsequent presentation messages (default 10)
#define LOOP_TIME 5000                       // Main loop wait time (default 100)        

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
#define ERROR_REPORTING
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

/*  *******************************************************************************************
 *                                  EEPROM Definitions
 *  *******************************************************************************************/
#define SIZE_OF_BYTE 1
#define SIZE_OF_FLOAT 4
#define EEPROM_OFFSET 512                     // First eeprom address to use (MySensors uses prior addresses)
#define EA_HM EEPROM_OFFSET                   // EEPROM address to save Heating Mode value (byte)
#define EA_SPN EA_HM+SIZE_OF_BYTE             // EEPROM address to save thermostat value for night mode (float)
#define EA_SPH EA_SPN+SIZE_OF_FLOAT           // EEPROM address to save thermostat value for holiday mode (float)
#define EA_HYSTERESIS EA_SPH+SIZE_OF_FLOAT    // EEPROM address to save hysteresis value (float)
#define EA_FIRST_SECTION EA_HYSTERESIS+SIZE_OF_FLOAT  // EEPROM address to save section values (float)

 
#endif
/*
 * 
 * EOF
 * 
 */