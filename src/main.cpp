#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <CapacitiveSensor.h>
#include <settings.h>

#define DEBUG true // Increases serial output
#define SSID "Sam <3 Pea"
#define MEASURE_SENSOR false // Use to find value for CAPACITIVE_THRESHOLD
#define CAPACITIVE_THRESHOLD 25

#define SENSOR_WIRE D6 // Pin for the capacitive sensor
#define RESISTOR    D7 // Pin for other end of the 100k resistor
#define OUTPUT_WIRE D5 // Pin for the output "touch" wire to main board

const int lampNumber = 0;
const char* host = "http://192.168.20.10:8000"; // HTTP + host + port

const int minPressInterval = 1000; // Minimum time that must pass between button presses, in milliseconds
int buttonState = 0; // Variable to store the button state
int changeColour = 0; // Variable to store whether simulateButtonPress needs to trigger
unsigned long lastPressTime = 0; // Variable to store the time of the last button press
CapacitiveSensor buttonSensor = CapacitiveSensor(RESISTOR, SENSOR_WIRE); // 100k resistor between GPIO12 & 13

const int minRequestInterval = 2000; // Minimum time that must pass between API requests, in milliseconds
unsigned long lastRequestTime = 0; // Variable to store the time of the last API request

WiFiUDP ntpUDP; // Define NTP Client
NTPClient timeClient(ntpUDP, "pool.ntp.org");
WiFiClient wifiClient; // Needed for HTTP requests

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

// Function to simulate a button press to the main board
void simulateButtonPress() {
  pinMode(OUTPUT_WIRE, OUTPUT); // Set the button pin as an output
  digitalWrite(OUTPUT_WIRE, HIGH);
  delay(100);
  digitalWrite(OUTPUT_WIRE, LOW);
  if (DEBUG) {
    Serial.println("Tapped the internal button!");
  }
  pinMode(OUTPUT_WIRE, INPUT); // Set the button pin as an output
}

// Function to notify the server we changed colours
void notifyChange() {
  HTTPClient http;
  http.begin(wifiClient, String(host) + "/lamp/" + String(lampNumber));
  http.addHeader("x-api-key",  APIKEY);
  int httpCode = http.POST("");

  if (httpCode == HTTP_CODE_OK) { // Check for successful response
    Serial.println("Told the server we changed colours.");
  } else {
    Serial.println("Error telling server we changed colours.");
  }
}

// Function to test the sensor in order to set CAPACITIVE_THRESHOLD
void measureCapacitive() {
  long start = millis();
  long total1 =  buttonSensor.capacitiveSensor(30);

  Serial.print(millis() - start);
  Serial.print("\t");
  Serial.print(total1);
  Serial.print("\n");

  delay(100);
}

// Function to read the value of the capacitive sensor
int readButton() {
  long value = buttonSensor.capacitiveSensor(30);
  if (value >= CAPACITIVE_THRESHOLD) {
    return 1;
  } else {
    return 0;
  }
}

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect(SSID);
 
  Serial.println("WiFi Connected!"); // If you get here you have connected to the WiFi
}

void loop() {
  buttonState = readButton(); // Read the button state
  unsigned long currentTime = millis(); // Keep track of time

  if (MEASURE_SENSOR) {
    measureCapacitive();
  }

  if (buttonState == HIGH) { // Check if the button is pressed
    if (currentTime - lastPressTime >= minPressInterval) { // Prevent quick taps
      HTTPClient http;
      http.begin(wifiClient, String(host) + "/lamp/" + String((lampNumber ^ 1))); // toggle 1/0
      http.addHeader("x-api-key",  APIKEY);
      int httpCode = http.POST("");

      if (httpCode == HTTP_CODE_OK) { // Check for successful response
        simulateButtonPress(); // we do this here so it's obvious to the user if the lamp isn't working!
        Serial.println("Told the server we're changing colours.");
      } else {
        Serial.println("Error telling server we're changing colours.");
      }
      lastPressTime = currentTime; // Update the time of the last button press
    }
  }

  if (currentTime - lastRequestTime >= minRequestInterval) { // Ask server if I need to change
    HTTPClient http;
    http.begin(wifiClient, String(host) + "/lamp/" + String(lampNumber) + "?" + String(getTime()));
    http.addHeader("x-api-key",  APIKEY);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) { // Check for successful response
      String payload = http.getString();

      if (payload == "1") { // I need to change colour!
        Serial.println("Changing colour.");
        simulateButtonPress(); // press button
        notifyChange(); // notify server of colour change
      } else {
        Serial.println("Checked with server, no change needed.");
      }
    } else {
      Serial.println("Error checking status with server");
    }
    lastRequestTime = currentTime; // Update the time of the last API request
  } 
}