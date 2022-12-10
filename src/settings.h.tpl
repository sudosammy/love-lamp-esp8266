const int lampNumber = 0;               // Lamp 1 or 0
const char* host = "http://myserver";   // HTTP + host + port

#define APIKEY "changeme"               // API key to use with the Lamp API server
#define SSID "Sam <3 Pea"               // WiFi name when in AP mode
#define PRODUCTION true                 // Reduces frequency of API requests
#define DEBUG false                     // Increases serial output
#define MEASURE_SENSOR false            // Use to find value for CAPACITIVE_THRESHOLD
#define CAPACITIVE_THRESHOLD 25

#define SENSOR_WIRE D6                  // Pin for the capacitive sensor
#define RESISTOR    D7                  // Pin for other end of the 100k resistor
#define OUTPUT_WIRE D5                  // Pin for the output "touch" wire to main board

#define NTP_SERVER "pool.ntp.org"       // Network Time Server