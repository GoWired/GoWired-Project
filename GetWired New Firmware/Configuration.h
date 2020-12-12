/*
 * Configuration.h file
 * Use this file for quick and easy configuration of your GetWired software.
 * For the whole description have a look at .ino file.
 * 
 */

#ifndef Configuration_h
#define Configuration_h

/*  *******************************************************************************************
                    MySensors Definitions
 *  *******************************************************************************************/
// Identification
#define MY_NODE_ID 1                            // Set node ID
#define SN "GetWired Module"               // Set node name to present to a controller
#define SV "1.0"                                // Set sensor version

// Selecting transmission settings
#define MY_RS485                              // Enable RS485 transport layer
#define MY_RS485_DE_PIN 7                     // DE Pin definition
#define MY_RS485_BAUD_RATE 57600              // Set RS485 baud rate
#define MY_RS485_HWSERIAL Serial              // Enable Hardware Serial
#define MY_RS485_SOH_COUNT 3                  // Collision avoidance

// FOTA Feature
#define MY_OTA_FIRMWARE_FEATURE                 // Enable OTA feature

// Other
#define MY_TRANSPORT_WAIT_READY_MS 60000        // Time to wait for gateway to respond at startup (default 60000)

/*  *******************************************************************************************
                    Quick config
 *  *******************************************************************************************/
// Output Config - one of these has to be defined
#define DOUBLE_RELAY              // Define this node as a double relay node, setting below
//#define ROLLER_SHUTTER            // Define this node as a roller shutter node, setting below
//#define FOUR_RELAY                // Define this node as a four relay node, setting below
//#define DIMMER                    // Define this node as a 1-channel dimmer node, setting below
//#define RGB                       // Define this node as a RGB dimmer node, setting below
//#define RGBW                      // Define this node as a RGBW dimmer node, setting below

// Input Config - define according to your needs
// Digital Inputs
#define INPUT_1
#define INPUT_2
#define INPUT_3
#define INPUT_4

// Board dependent
#define POWER_SENSOR
#define INTERNAL_TEMP

// External temperature sensor - define exactly one from: DHT22, SHT30 or DS18B20
//#define EXTERNAL_TEMP
#ifdef EXTERNAL_TEMP
    //#define DHT22
    #define SHT30
    //#define DS18B20    
#endif

/*  *******************************************************************************************
                    MCU Pin Definitions
 *  *******************************************************************************************/
// OUTPUT [RELAY / RGBW]
#define OUTPUT_PIN_1 5
#define OUTPUT_PIN_2 9
#define OUTPUT_PIN_3 6
#define OUTPUT_PIN_4 10

// INPUT [BUTTON / SENSOR]
// General input
#define INPUT_PIN_1 2             // default 2
#define INPUT_PIN_2 3             // default 3
#define INPUT_PIN_3 4             // default 4
#define INPUT_PIN_4 A3              // default A3

// Analog input
#define INPUT_PIN_5 A1              // default A1
#define INPUT_PIN_6 A2              // default A2
#define INPUT_PIN_7 A6              // default A6
#define INPUT_PIN_8 A7              // default A7

// Protocols
// 1-wire
#define ONE_WIRE_PIN A0             // default A0

// I2C
#define I2C_PIN_1 A4              // default A4
#define I2C_PIN_2 A5              // default A5

/*  *******************************************************************************************
                    Various Definitions
 *  *******************************************************************************************/
// General
#define RELAY_ON HIGH                       // Pin state to turn the relays on (default HIGH)
#define RELAY_OFF LOW                       // Pin state to turn the relays off (default LOW)

// Internal temperature sensor
#define MVPERC 10                         // V per 1 degree celsius (default 10)
#define ZEROVOLTAGE 500                     // Voltage output of temperature sensor (default 500)
#define MAX_TEMPERATURE 85                  // Maximum temperature the module can have before reporting error (default 85)

// Power Sensor
#define MAX_CURRENT 3                      // Maximum current the module can handle before reporting error (2SSR - 3; 4RelayDin - 10A or 16)
#define POWER_MEASURING_TIME 20             // Current measuring takes this long (default 20)
#define MVPERAMP 185                       // mV per 1A (default: 2SSR 185 mV/A; 4RelayDin 73.3 mV/A, RGBW 100 mV/A)
#define RECEIVER_VOLTAGE 230                // 230V, 24V, 12V - values for power usage calculation, depends on the receiver
#define COSFI 1                             // cos(fi) value for a given load: resistive load - 1, LED - 0.4 < cos(fi) < 0.99, fluorescent - 

// Dimmer
#define DIMMING_STEP 1                      // Size of dimming step, increase for faster, less smooth dimming (default 1)
#define DIMMING_INTERVAL 1                  // Duration of dimming interval, increase for slower dimming (default 10)
#define DIMMING_TOGGLE_STEP 20              // Value to increase dimming percentage when using wall switch

// Roller Shutter
#define RS_AUTO_CALIBRATION
#ifdef RS_AUTO_CALIBRATION
  #define PS_OFFSET 0.2                     // Power sensor offset for roller shutter calibration (default 0.2)
  #define CALIBRATION_SAMPLES 2             // Number of calibration samples for roller shutter calibration (default 2)
#else
  #define UP_TIME 21                        // Define roller shutter movement durations manually
  #define DOWN_TIME 20
#endif

// Heating system section thermometer
//#define HEATING_SECTION_SENSOR            // Define if this module if going to be a temperature sensor for a heating controller
#ifdef HEATING_SECTION_SENSOR
  #define MY_HEATING_CONTROLLER 1           // Node ID to which this module should report external temperature to
#endif

// Other
#define INTERVAL 300000                    // Interval value for reporting readings of the sensors: temperature, power usage (default 300000)
#define INIT_DELAY 200                       // A value to be multiplied by node ID value to obtain the time to wait during the initialization process
#define PRESENTATION_DELAY 10       // Time (ms) to wait between subsequent presentation messages (default 10)
#define LOOP_TIME 100                       // Main loop wait time (default 100)        

/*  *******************************************************************************************
                    OUTPUT Config
 *  *******************************************************************************************/
// 2SSR DOUBLE_RELAY
#ifdef DOUBLE_RELAY
  #define RELAY_ID_1 0
  #define RELAY_ID_2 1
  #define RELAY_1 OUTPUT_PIN_1
  #define RELAY_2 OUTPUT_PIN_2
  #define BUTTON_1 INPUT_PIN_1
  #define BUTTON_2 INPUT_PIN_2
  #define NUMBER_OF_RELAYS 2
#endif

// Roller Shutter
#ifdef ROLLER_SHUTTER
  #define RS_ID 0
  #define RELAY_ID_1 0
  #define RELAY_1 OUTPUT_PIN_1
  #define RELAY_2 OUTPUT_PIN_2
  #define BUTTON_1 INPUT_PIN_1
  #define BUTTON_2 INPUT_PIN_2
  #define NUMBER_OF_RELAYS 2
#endif

// 4RelayDin 4 Relay Output
#ifdef FOUR_RELAY
  #define RELAY_ID_1 0
  #define RELAY_ID_2 1
  #define RELAY_ID_3 2
  #define RELAY_ID_4 3
  #define RELAY_1 OUTPUT_PIN_3
  #define RELAY_2 OUTPUT_PIN_2
  #define RELAY_3 OUTPUT_PIN_1
  #define RELAY_4 OUTPUT_PIN_4
  #define NUMBER_OF_RELAYS 4
#endif

// Dimmer / RGB / RGBW
#ifdef DIMMER
  #define DIMMER_ID 0
  #define LED_PIN_1 OUTPUT_PIN_1
  #define LED_PIN_2 OUTPUT_PIN_2
  #define LED_PIN_3 OUTPUT_PIN_3
  #define LED_PIN_4 OUTPUT_PIN_4
  #define BUTTON_1 INPUT_PIN_1
  #define BUTTON_2 INPUT_PIN_2
  #define NUMBER_OF_CHANNELS 4
#endif

#ifdef RGB
  #define DIMMER_ID 0
  #define LED_PIN_1 OUTPUT_PIN_4
  #define LED_PIN_2 OUTPUT_PIN_1
  #define LED_PIN_3 OUTPUT_PIN_2
  #define BUTTON_1 INPUT_PIN_1
  #define BUTTON_2 INPUT_PIN_2
  #define NUMBER_OF_CHANNELS 3
#endif

#ifdef RGBW
  #define DIMMER_ID 0
  #define LED_PIN_1 OUTPUT_PIN_4
  #define LED_PIN_2 OUTPUT_PIN_1
  #define LED_PIN_3 OUTPUT_PIN_2
  #define LED_PIN_4 OUTPUT_PIN_3
  #define BUTTON_1 INPUT_PIN_1
  #define BUTTON_2 INPUT_PIN_2
  #define NUMBER_OF_CHANNELS 4
#endif

#ifdef NUMBER_OF_RELAYS
  #define FIRST_INPUT_ID NUMBER_OF_RELAYS
#elif defined(NUMBER_OF_CHANNELS)
  #define FIRST_INPUT_ID 2
  #define NUMBER_OF_RELAYS 2
#else
  #define NUMBER_OF_RELAYS 0
  #define FIRST_INPUT_ID 0
#endif

/*  *******************************************************************************************
                    INPUT Config
 *  *******************************************************************************************/
// Digital input - define what inputs to use
#ifdef INPUT_1
  #define INPUT_ID_1 FIRST_INPUT_ID
  #define PIN_1 INPUT_PIN_3
  #define PULLUP_1
  #define NUMBER_OF_INPUTS 1
#endif

#ifdef INPUT_2
  #define INPUT_ID_2 INPUT_ID_1+1
  #define PIN_2 INPUT_PIN_4
  #define PULLUP_2
  #define NUMBER_OF_INPUTS 2
#endif

#ifdef INPUT_3
  #define INPUT_ID_3 INPUT_ID_2+1
  #define PIN_3 INPUT_PIN_5
  #define PULLUP_3
  #define NUMBER_OF_INPUTS 3
#endif

#ifdef INPUT_4
  #define INPUT_ID_4 INPUT_ID_3+1
  #define PIN_4 INPUT_PIN_6
  #define PULLUP_4
  #define NUMBER_OF_INPUTS 4
#endif

#ifndef NUMBER_OF_INPUTS
  #define NUMBER_OF_INPUTS 0
#endif

// Special Button
#ifdef BUTTON_1
  #define SPECIAL_BUTTON
  #define SPECIAL_BUTTON_ID 8
#endif

// ACS712 Power Sensor
#ifdef POWER_SENSOR
  #if defined(DOUBLE_RELAY) || defined(ROLLER_SHUTTER)
    #define PS_ID 9
    #define PS_PIN INPUT_PIN_7
  #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
    #define PS_ID 9
    #define PS_PIN INPUT_PIN_8
  #elif defined(FOUR_RELAY)
    #define PS_ID_1 4
    #define PS_ID_2 5
    #define PS_ID_3 6
    #define PS_ID_4 7
    #define PS_PIN_1 INPUT_PIN_7
    #define PS_PIN_2 I2C_PIN_2
    #define PS_PIN_3 I2C_PIN_1
    #define PS_PIN_4 INPUT_PIN_8
  #endif
#endif

// Analog Internal Thermometer (Disable for 4RelayDin)
#ifdef INTERNAL_TEMP
  #define IT_ID 10
  #if defined(DOUBLE_RELAY) || defined(ROLLER_SHUTTER)
    #define IT_PIN INPUT_PIN_8
  #elif defined(DIMMER) || defined(RGB) || defined(RGBW)
    #define IT_PIN INPUT_PIN_7
  #endif
#endif

// 1wire external thermometer (e.g. DHT22)
#ifdef EXTERNAL_TEMP
  #define ETH_ID 11
  #define ETT_ID 12
  #if defined(DHT22) || defined(DS18B20)
    #define ET_PIN ONE_WIRE_PIN
  #endif
#endif

/*  *******************************************************************************************
                      ERROR REPORTING
 *  *******************************************************************************************/
#define ERROR_REPORTING
#ifdef ERROR_REPORTING
  #ifdef POWER_SENSOR
    #define ES_ID 23
  #endif
  #ifdef INTERNAL_TEMP
    #define TS_ID 24
  #endif
  #ifdef EXTERNAL_TEMP
    #define ETS_ID 25
  #endif
#endif

//#define RS485_DEBUG
#ifdef RS485_DEBUG
  #define DEBUG_ID 26
#endif

/*  *******************************************************************************************
                    EEPROM Definitions
 *  *******************************************************************************************/
#define SIZE_OF_INT 2
#define EEPROM_OFFSET 512                         // First eeprom address to use (prior addresses are taken)
#define EEA_RS_TIME_DOWN EEPROM_OFFSET            // EEPROM address to save RShutter travel down time
#define EEA_RS_TIME_UP EEA_RS_TIME_DOWN+SIZE_OF_INT     // EEPROM address to save RShutter travel up time
#define EEA_RS_POSITION EEA_RS_TIME_UP+SIZE_OF_INT      // EEPROM address to save RShutter last known position

#endif
/*
   EOF
*/
