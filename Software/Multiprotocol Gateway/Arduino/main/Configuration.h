/*
 * Configuration file for Multiprotocol Gateway
 */

/*
 * Basics
 */
// Software name & version
#define SN "GoWired Multiprotocol Gateway"
#define SV "0.1"

// Web frontend
#define WEBFRONTEND                                       // provide a webfrontend that shows gateway config
#define HTTP_PORT 80                                      // Port for http frontend

/*
 * MySensors definitions
 */
// RS485 interface
#define MY_RS485                                          // Enable RS485 transport layer
#define MY_RS485_DE_PIN 33                                // Define this to enables DE-pin management on defined pin
#define MY_RS485_BAUD_RATE 57600                          // Set RS485 baud rate to use
#define MY_RS485_HWSERIAL Serial                          // Select which serial to use (Serial/Serial1) 
#define MY_RS485_SOH_COUNT 3                              // Use this in case of collisions on the bus

// Gateway parameters
//#define MY_IP_ADDRESS 192,168,8,70                        // Gateway IP address; comment out for DHCP
#define MY_ETHERNET_PORT 5003                             // The port to keep open on node server mode / or port to contact in client mode
#define SOFT_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB // Software defined Gateway MAC address

// MQTT
//#define MQTT_GATEWAY                                      // Enable for the Gateway to work as MQTT
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "gw-out"             // Prefix of publish topic
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gw-in"            // Prefix of subscribe topic
#define MY_MQTT_CLIENT_ID "GW-1"                          // MQTT client ID 
#define MY_MQTT_PORT 1883                                 // MQTT port

// Controller ip address. Enables client mode (default is "server" mode). 
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere. 
#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 254   

#ifdef MQTT_GATEWAY
  #define MY_GATEWAY_MQTT_CLIENT                            
  #define MY_PORT MY_MQTT_PORT
#else
  #define MY_GATEWAY_ENC28J60                               // Enable gateway ethernet module type 
  #define MY_PORT MY_ETHERNET_PORT
#endif

// LEDs
#define MY_INDICATION_HANDLER                             // LEDs are connected through an expander, so they cannot be handled by functions implementes in MySensors library
//#define MY_WITH_LEDS_BLINKING_INVERSE                     // Define to inverse LED behaviour
#define MY_DEFAULT_LED_BLINK_PERIOD 10                    // Set blinking period
//#define MY_DEFAULT_ERR_LED_PIN 4                          // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  5                          // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  6                          // Transmit led pin

// Debug prints
//#define MY_DEBUG                                        // Enable debug prints to serial monitor
#define MY_DEBUGDEVICE Serial2                            // Redirects debug prints to Serial2 (USB)

/* 
 * MODBUS
 */

/*
 * CAN
 */

/*
 * Other peripherals
 */
// 25AA02E48
#define EUI48_EEPROM
#define EEPROM_CS_PIN 15                // Adjust correct pin number (expander)
#define READ_INSTRUCTION 0b00000011     // 25AA02A's read command
#define WRITE_INSTRUCTION 0b00000010    // 25AA02A's write command
#define READ_STATUS_INSTRUCTION  0b00000101   
#define WRITE_STATUS_INSTRUCTION 0b00000001
#define WREN 6

// INA219
#define INA219_ADDRESS 0x40
#define R_SHUNT 0.1
#define V_SHUNT_MAX 0.2
#define V_BUS_MAX 26
#define I_MAX_EXPECTED 3

// SD CARD
#define SD_CS_PIN SDCARD_CS

// Enclosure panel
#define EXPANDER_CS_PIN CONNECTOR_CS2

// Other
#define BUTTON_1 0              // MCP23S17 pin
#define BUTTON_2 1              // MCP23S17 pin
#define LED1 2                  // MCP23S17 pin
#define LED2 3                  // MCP23S17 pin
#define LED3 4                  // MCP23S17 pin
#define LED4 5                  // MCP23S17 pin
#define LED5 6                  // MCP23S17 pin
#define BUS_RELAY 8             // ATSAMD21G18 pin

#define RELAY_BUTTON BUTTON_1
#define RELAY_LED LED4

#define MY_DEFAULT_ERR_LED_PIN LED1                       // Error led pin
#define MY_DEFAULT_RX_LED_PIN  LED2                       // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  LED3                       // Transmit led pin

/* 
 * WATCHDOG & CONTROLLER UPLINK CHECK
 * Watchdog resets the gateway in case of any software hang. Enabling it should result in
 * more robustness and long term reliability.
 * Uplink Check tests the connection between the Gateway and the Controller.
 * In case of connection loss, gateway will be reset. 
 * Time interval between tests can be customized.
 */
//#define ENABLE_WATCHDOG                                   // Resets the Gateway in case of any software hang
#define ENABLE_UPLINK_CHECK                               // Resets the Gateway in case of connection loss with the controller
#define UPLINK_CHECK_INTERVAL 60000                       // Time interval for the uplink check (default 60000)
















