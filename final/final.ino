#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// WiFi credentials
const char* ssid = "POCO F4";
const char* password = "sayaadalahkamu";

// ThingSpeak API
const char* server = "http://api.thingspeak.com";
const char* apiKey = "2WEDLR29SB1UFFFI";

// Define pins
#define DHT_PIN 26
#define MQ135_PIN 34
#define BUZZER_PIN 5
#define BUTTON_PIN 19

// Initialize DHT sensor
DHT dht(DHT_PIN, DHT11);

// Number of readings to average (1 per second for 30 seconds)
const int numReadings = 30;
float readings[numReadings]; // The readings from the analog input
float temperatureReadings[numReadings]; // The readings from the temperature sensor
int readIndex = 0; // The index of the current reading
float totalCO2 = 0.0; // The running total for CO2 concentration
float averageCO2 = 0.0; // The average CO2 concentration
float totalTemperature = 0.0; // The running total for temperature
float averageTemperature = 0.0; // The average temperature

// Threshold for CO2 concentration in ppm
const float CO2Threshold = 1000.0; // Set the threshold value to 1000 ppm

bool collectingData = false; // Flag to indicate if we are collecting data
unsigned long startTime = 0; // Start time for the data collection period
unsigned long lastButtonPress = 0; // Last button press time
const unsigned long debounceDelay = 50; // Debounce time

void setup() {
  pinMode(MQ135_PIN, INPUT);  // Set the MQ-135 pin as input
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set the button pin as input with internal pull-up resistor

  Serial.begin(9600); // Initialize the serial monitor at a baud rate of 115200
  Serial.println("Starting serial communication...");

  dht.begin(); // Initialize the DHT sensor

  // Initialize all the readings to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0.0;
    temperatureReadings[thisReading] = 0.0;
  }

  WiFi.begin(ssid, password); // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  static bool lastButtonState = HIGH; // Last state of the button
  bool buttonState = digitalRead(BUTTON_PIN); // Current state of the button

  // Handle button debounce
  if ((millis() - lastButtonPress) > debounceDelay) {
    if (lastButtonState == HIGH && buttonState == LOW) {
      collectingData = !collectingData; // Toggle data collection state
      if (collectingData) {
        Serial.println("Data collection started.");
        startTime = millis(); // Start the timer for data collection
      } else {
        Serial.println("Data collection stopped.");
      }
      lastButtonPress = millis(); // Update last button press time
    }
  }

  lastButtonState = buttonState; // Update the last button state

  if (collectingData) {
    // Request temperature and humidity readings from DHT11
    float temperatureC = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(temperatureC) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature (C): ");
      Serial.println(temperatureC);
      Serial.print("Humidity (%): ");
      Serial.println(humidity);

      // Subtract the last temperature reading
      totalTemperature = totalTemperature - temperatureReadings[readIndex];
      // Read the new temperature
      temperatureReadings[readIndex] = temperatureC;
      // Add the reading to the total
      totalTemperature = totalTemperature + temperatureReadings[readIndex];

      // Calculate the average temperature
      averageTemperature = totalTemperature / float(numReadings);
    }

    // Read the MQ-135 sensor
    int analogValue = analogRead(MQ135_PIN); // Read the analog value from MQ-135 sensor

    // Convert the analog value to a meaningful CO2 concentration
    float ppm = (analogValue / 1024.0) * 5000;

    // Subtract the last CO2 reading
    totalCO2 = totalCO2 - readings[readIndex];
    // Read from the sensor
    readings[readIndex] = ppm;
    // Add the reading to the total
    totalCO2 = totalCO2 + readings[readIndex];

    // Calculate the average CO2 concentration
    averageCO2 = totalCO2 / float(numReadings);

    Serial.print("CO2 Concentration (ppm): ");
    Serial.println(ppm);

    // Advance to the next position in the array
    readIndex = (readIndex + 1) % numReadings;

    // Check if 30 seconds have passed
    if (millis() - startTime >= 30000) {
      collectingData = false;
      Serial.print("Average CO2 Concentration (ppm) over 30 seconds: ");
      Serial.println(averageCO2);
      Serial.print("Average Temperature (C) over 30 seconds: ");
      Serial.println(averageTemperature);

      // Check if the average ppm exceeds the threshold and turn on buzzer if it does
      if (averageCO2 >= CO2Threshold) {
        digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
        Serial.println("Buzzer ON: CO2 concentration exceeds threshold");
        delay(5000); // Keep buzzer on for 5 seconds
        digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
        Serial.println("Buzzer OFF");
      }

      // Send data to ThingSpeak
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(server) + "/update?api_key=" + apiKey + "&field1=" + String(averageTemperature) + "&field2=" + String(averageCO2);
        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
          Serial.println("Data berhasil diunggah ke ThingSpeak.");
        } else {
          Serial.print("Error on sending GET: ");
          Serial.println(httpResponseCode);
          Serial.println("Data gagal diunggah ke ThingSpeak.");
        }
        http.end();
      } else {
        Serial.println("Tidak terhubung ke WiFi.");
      }

      // Reset the readings for the next cycle
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = 0.0;
        temperatureReadings[thisReading] = 0.0;
      }
      totalCO2 = 0.0;
      totalTemperature = 0.0;
      readIndex = 0;
      startTime = millis(); // Reset the timer
    }
  }

  delay(1000);  // Wait for 1 second before reading the sensor again
}
