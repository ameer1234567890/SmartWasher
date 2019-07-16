#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Secrets.h"

/*
Secrets.h file should contain data as below:
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define IFTTT_URL "http://maker.ifttt.com/trigger/washer_finished/with/key/xxxxxxxxxxxxxxxxxxxxxx"
*/

#define SENSOR_PIN D3
#define BUZZER_PIN D4
#define LED_PIN D5

long lastTickTime;
//long lastTime;
bool washing = false;
int sensorValue = HIGH;
int tickCount = 0;
int delayBetweenTicks = 1000; // 1 second
int resetInterval = 10000; // 10 seconds
int numTicksRequired = 10;
WiFiClient wClient;


void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);  
  Serial.println("\nStartup");
  setupWifi();
}


void loop() {
  sensorValue = digitalRead(SENSOR_PIN);
  if (!washing) {
    if (sensorValue == LOW && millis() > (lastTickTime + delayBetweenTicks)) {
      tickCount++;
      lastTickTime = millis();
      Serial.print("Vibration detected. tickCount: ");
      Serial.println(tickCount);
    }
    if (tickCount > numTicksRequired) {
      washing = true;
      Serial.println("Washing started!");
    }
  } else {
    if (sensorValue == LOW) {
      if (millis() > (lastTickTime + delayBetweenTicks)) {
        lastTickTime = millis();
        tickCount++;
        Serial.print("Vibration detected! tickCount: ");
        Serial.println(tickCount);
      }
      if (tickCount > numTicksRequired) {
        tickCount = numTicksRequired;
        Serial.print("Ticker going too high! Reset! tickCount: ");
        Serial.println(tickCount);
      }
    }
    if (tickCount == 0) {
      washing = false;
      Serial.println("Washing finished!");
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

  if (!washing && (millis() - lastTickTime) > resetInterval) {
    if (tickCount > 0) {
      tickCount = 0;
      Serial.print("Ticker reset due to inactivity! tickCount: ");
      Serial.println(tickCount);
    }
  }

  if (washing && (millis() - lastTickTime) > resetInterval) {
    tickCount--;
    Serial.print("Ticker reduced due to inactivity! tickCount: ");
    Serial.println(tickCount);
    lastTickTime = millis();
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
