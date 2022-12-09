#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <settings.h>

#define SSID "Sam <3 Pea"

const int lampNumber = 0;
const int minPressInterval = 1000; // Minimum time that must pass between button presses, in milliseconds
const int buttonPin = D0; // Pin where the button is connected
const char* host = "http://192.168.20.10:8000"; // HTTP + host + port

int buttonState = 0; // Variable to store the button state
int changeColour = 0; // Variable to store whether simulateButtonPress needs to trigger
unsigned long lastPressTime = 0; // Variable to store the time of the last button press

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

// Function to simulate a button press on the specified pin
void simulateButtonPress() {
  pinMode(buttonPin, OUTPUT); // Set the button pin as an output
  digitalWrite(buttonPin, HIGH);
  delay(100);
  digitalWrite(buttonPin, LOW);
  pinMode(buttonPin, INPUT); // Set button back to input
}

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

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect(SSID);
 
  Serial.println("WiFi Connected!"); // If you get here you have connected to the WiFi
  pinMode(buttonPin, INPUT); // Set the button pin as an input
}

void loop() {
  buttonState = digitalRead(buttonPin); // Read the button state
  unsigned long currentTime = millis(); // Keep track of time

  if (buttonState == HIGH) { // Check if the button is pressed
    if (currentTime - lastPressTime >= minPressInterval) { // Prevent quick taps
      HTTPClient http;
      http.begin(wifiClient, String(host) + "/lamp/" + String((lampNumber ^ 1))); // toggle 1/0
      http.addHeader("x-api-key",  APIKEY);
      int httpCode = http.POST("");

      if (httpCode == HTTP_CODE_OK) { // Check for successful response
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