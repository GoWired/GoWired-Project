/*
 * Configuration.h file
 * With this file it is possible to configure some advanced features of the software,
 * such as board pinout, backlight parameters, system components.
*/

/* Pin Definitions */
// Digital pins
#define DIGITAL_PIN_1 4
#define DIGITAL_PIN_2 7
#define DIGITAL_PIN_3 8
#define DIGITAL_PIN_4 23

// Builtin LEDs
// LED1
#define LED_PIN_1 3
#define LED_PIN_2 2
#define LED_PIN_3 1

// LED2
#define LED_PIN_4 11
#define LED_PIN_5 9
#define LED_PIN_6 10

// LED3
#define LED_PIN_7 5
#define LED_PIN_8 6
#define LED_PIN_9 0

// Analog pins
#define ANALOG_PIN_1 A0
#define ANALOG_PIN_2 A1
#define ANALOG_PIN_3 A2

// Dip switches
#define DIP_SWITCH_1 A6
#define DIP_SWITCH_2 A7
#define DIP_SWITCH_3 A3
#define DIP_SWITCH_4 A4


/* Quick config */
//#define SINGLE_RELAY
#define DOUBLE_RELAY


/* Basic definitions */
// Relay states
#define RELAY_ON HIGH
#define RELAY_OFF LOW

// Hardware detection
#define HARDWARE_DETECTION_PIN A5           // Auto-detect connected hardware
#define NUMBER_OF_BUTTONS 2                 // Determined by hardware

// Reading inputs
#define TOUCH_THRESHOLD 10                   // A threshold to determine if it was a touch what was sensed (default 5)
#define DEBOUNCE_VALUE 50                   // Debounce time in ms (1 - no debounce, >1 active debounce, default 20, max. 255)
#define LONGPRESS_DURATION 1000             // Duration of longpress in ms (default 1000, max. 65535)

// Builtin LEDs
#define NUMBER_OF_CHANNELS 3                // RGB LEDs
#define DIMMING_STEP 10                     // Size of dimming step, increase for faster, less smooth dimming (default 1)
#define DIMMING_INTERVAL 1                  // Duration of dimming interval, increase for slower dimming (default 10)
#define R_VALUE_OFF 255                     // 255 - OFF, 0 - Full ON
#define G_VALUE_OFF 230
#define B_VALUE_OFF 204
#define R_VALUE_ON 153
#define G_VALUE_ON 204
#define B_VALUE_ON 255
#define BRIGHTNESS_VALUE_OFF 100            // Global brightness value should always be set to 100
#define BRIGHTNESS_VALUE_ON 100
#define INIT_RAINBOW_DURATION 1000          // Duration of initial rainbow effect (0-65535, default 1000)
#define INIT_RAINBOW_RATE 1                 // Rate of initial reainbow effect (0-255, default 1)

// Roller Shutter
#define RS_INTERVAL 80                 // Time in seconds to turn off roller shutter after it was turned on (deafult 80)

// Other
#define LOOP_TIME 100                        // Main loop wait time (default 100)


/* OUTPUT Config */
#define RELAY_PIN_1 DIGITAL_PIN_3
#define RELAY_PIN_2 DIGITAL_PIN_2
#define TOUCH_FIELD_1 ANALOG_PIN_1
#define TOUCH_FIELD_2 ANALOG_PIN_2
#define TOUCH_FIELD_3 ANALOG_PIN_3
#define INPUT_PIN_1 DIGITAL_PIN_1
#define INPUT_PIN_2 DIGITAL_PIN_4
//#define BUTTON_LED_1 LED0
//#define BUTTON_LED_2 LED2
#define NUMBER_OF_RELAYS 2


/* Reliability */
// Watchdog
#define ENABLE_WATCHDOG

/* EEPROM Addresses */
#define SIZE_OF_BYTE 1
#define EEPROM_OFFSET 512                         // First eeprom address to use (prior addresses are taken)
#define EEA_RELAY_1 EEPROM_OFFSET+SIZE_OF_BYTE            // EEPROM addresses to save relay states
#define EEA_RELAY_2 EEA_RELAY_1+SIZE_OF_BYTE

