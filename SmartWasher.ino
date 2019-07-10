#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Secrets.h"

/*
Secrets.h file should contain data as below:
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define IFTTT_URL "http://maker.ifttt.com/trigger/washer_finished/with/key/xxxxxxxxxxxxxxxxxxxxxx"
*/

#define BUZZER_PIN D5
#define SENSOR_PIN A0
#define LED_PIN D4

void setupWifi(void);
bool postToIfttt(void);

int sensorValue;
int minSense = 950;
int delayBetweenChecks = 5000; // 5 seconds
int numTicks = 5;
int probableStart = 0;
int probableFinish = 0;
unsigned long timeTillDetection = 45000; // 45 seconds
unsigned long startMillis;
unsigned long currentMillis;
bool washing = false;
bool startUpSignalled = false;
WiFiClient wClient;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("OS Start up");
  setupWifi();
}

void loop() {
  if (!startUpSignalled) {
    Serial.println("Program startup");
    startUpSignalled = true;
  }
  sensorValue = analogRead(SENSOR_PIN);
  Serial.println(sensorValue);
  delay(1000);
  if (washing == false) {
    digitalWrite(LED_PIN, LOW);
    if (sensorValue < minSense) {
      Serial.print("Not Started - sensorValue: ");
      Serial.println(sensorValue);
      if (probableStart == 0) {
        startMillis = millis();
      }
      probableStart++;
      currentMillis = millis();
      if (currentMillis > (startMillis + timeTillDetection)) {
        probableStart = 0;
      }
      tone(BUZZER_PIN, 1000);
      delay(50);
      noTone(BUZZER_PIN);
      Serial.print("probableStart: ");
      Serial.println(probableStart);
      if (probableStart >= numTicks) {
        Serial.print(probableStart);
        Serial.println(" ticks registered! Washer start deemed!");
        washing = true;
        probableStart = 0;
        digitalWrite(LED_PIN, HIGH);
      }
      delay(delayBetweenChecks);
    }
  } else {
    currentMillis = millis();
    if (sensorValue < minSense) {
      Serial.print("Running - sensorValue: ");
      Serial.println(sensorValue);
      if (probableFinish == 0) {
        startMillis = millis();
      }
      probableFinish++;
      if (currentMillis > (startMillis + timeTillDetection)) {
        probableFinish = 0;
      }
      tone(BUZZER_PIN, 1000);
      delay(50);
      noTone(BUZZER_PIN);
      Serial.print("probableFinish: ");
      Serial.println(probableFinish);
      delay(delayBetweenChecks);
    }
    if (currentMillis > (startMillis + timeTillDetection + 10000)) {
      Serial.print("No ticks registered for ");
      Serial.print((timeTillDetection + 10000) / 1000);
      Serial.println(" seconds. Washer finish deemed!");
      washing = false;
      probableFinish = 0;
      digitalWrite(LED_PIN, LOW);
      if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Wifi was connected!");
        notify();
      } else {
        Serial.println("Wifi was disconnected!");
        setupWifi();
        notify();
      }
    }
  }
}


void notify() {
  if (postToIfttt()) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Done!");
    delay(2000);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("IFTTT did not work!");
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
  }
}


bool postToIfttt() {
  HTTPClient http;
  uint httpCode;
  http.begin(wClient, IFTTT_URL);
  httpCode = http.GET();
  http.end();
  if (httpCode == 200) {
    Serial.println(httpCode);
    return true;
  } else {
    Serial.println(httpCode);
    return false;
  }
}


void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(700);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}
