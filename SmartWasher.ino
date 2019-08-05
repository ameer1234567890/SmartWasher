#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Wire.h>
#include "Secrets.h"

/* Secrets.h file should contain data as below: */
#ifndef WIFI_SSID
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define IFTTT_URL "http://maker.ifttt.com/trigger/washer_finished/with/key/xxxxxxxxxxxxxxxxxxxxxx"
#endif

/* Configurable variables */
#define LED_PIN D5
#define SERVER_PORT 80
#define OTA_HOSTNAME "SmartWasher"
const int DST = 0; // Daylight Saving Time
const int MPU = 0x68; // i2c address for the sensor
const int TIMEZONE = 5; // your timezone (UTC+5)

/* Do not change unless you know what you are doing */
String logMsg;
String logTime;
int sensorValue;
int tickCount = 0;
bool washing = false;
unsigned long lastTickTime;
const int threshold = 16000;
const int resetInterval = 5000; // 5 seconds
const int numTicksRequired = 10;
bool sensorErrorReported = false;
const int delayBetweenTicks = 1000; // 1 second

WiFiClient wClient;
ESP8266WebServer server(SERVER_PORT);


void setup() {
  log("I/system: startup");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(115200);
  setupWifi();
  setupTime();
  server.on("/", []() {
    server.send(200, "text/html", "\
      <a href=\"/log\">/log</a><br>\
      <a href=\"/reboot\">/reboot</a><br>\
    ");
  });
  server.on("/log", []() {
    server.send(200, "text/plain", logMsg);
  });
  server.on("/reboot", []() {
    server.send(200, "text/plain", "rebooting");
    delay(1000);
    ESP.restart();
  });
  server.begin();
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();
}


void loop() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, 1);

  sensorValue = Wire.read()<<8|Wire.read();
  if (sensorValue < 2) {
    if (!sensorErrorReported) {
      sensorErrorReported = true;
      log("E/ticker: Sensor error! " + String(sensorValue));
    }
  } else {
    sensorErrorReported = false;
    if (!washing) {
      if (sensorValue < threshold && millis() > (lastTickTime + delayBetweenTicks)) {
        tickCount++;
        lastTickTime = millis();
        log("I/ticker: vibration detected " + String(sensorValue) + ". tickCount => " + String(tickCount));
      }
      if (tickCount > numTicksRequired) {
        washing = true;
        log("I/ticker: washing started!");
      }
    } else {
      if (sensorValue < threshold) {
        if (millis() > (lastTickTime + delayBetweenTicks)) {
          lastTickTime = millis();
          tickCount++;
          log("I/ticker: vibration detected " + String(sensorValue) + ". tickCount => " + String(tickCount));
        }
        if (tickCount > numTicksRequired) {
          tickCount = numTicksRequired;
          log("I/ticker: ticker going too high! reset! tickCount => " + String(tickCount));
        }
      }
      if (tickCount == 0) {
        washing = false;
        log("I/ticker: washing finished!");
        if(WiFi.status() == WL_CONNECTED) {
          log("I/ticker: wifi was connected!");
          notify();
        } else {
          log("I/ticker: wifi was disconnected!");
          setupWifi();
          notify();
        }
      }
    }

    if (!washing && (millis() - lastTickTime) > resetInterval && tickCount > 0) {
      tickCount--;
      log("I/ticker: ticker reduced due to inactivity! tickCount => " + String(tickCount));
      lastTickTime = millis();
    }

    if (washing && (millis() - lastTickTime) > resetInterval) {
      tickCount--;
      log("I/ticker: ticker reduced due to inactivity! tickCount => " + String(tickCount));
      lastTickTime = millis();
    }
  }

  server.handleClient();
  ArduinoOTA.handle();
}


void notify() {
  if (postToIfttt()) {
    digitalWrite(LED_PIN, HIGH);
    log("I/notify: notified via IFTTT!");
    delay(2000);
    digitalWrite(LED_PIN, LOW);
  } else {
    log("E/notify: IFTTT did not work!");
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
    log("I/notify: IFTTT response => " + String(httpCode));
    return true;
  } else {
    log("E/notify: IFTTT response => " + String(httpCode));
    return false;
  }
}


void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);
    delay(10);
    digitalWrite(LED_PIN, LOW);
    delay(10);
  }
  log("I/wifi  : wifi connected");  
  log("I/wifi  : IP address => " + WiFi.localIP().toString());
  delay(700);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}


void log(String msg) {
  time_t now = time(0);
  logTime = ctime(&now);
  logTime.trim();
  logMsg = logMsg + "[" + logTime + "] ";
  logMsg = logMsg + msg + "\n";
  Serial.println(msg);
}


void setupTime() {
  configTime(TIMEZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");
  log("I/time  : waiting for time");
  while (!time(nullptr)) {
    delay(100);
  }
  delay(100);
  time_t now = time(0);
  logTime = ctime(&now);
  logTime.trim();
  log("I/time  : time obtained via NTP => " + logTime);
}
