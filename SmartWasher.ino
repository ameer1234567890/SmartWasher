#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <time.h>
#include "Secrets.h"

/* Secrets.h file should contain data as below: */
#ifndef WIFI_SSID
#define WIFI_SSID "xxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"
#define IFTTT_URL "http://maker.ifttt.com/trigger/washer_finished/with/key/xxxxxxxxxxxxxxxxxxxxxx"
#endif

#define LED_PIN D5
#define SENSOR_PIN D3
#define BUZZER_PIN D4
#define SERVER_PORT 80
#define OTA_HOSTNAME "SmartWasher"

String logMsg;
String logTime;
long lastTickTime;
const int DST = 0;
int tickCount = 0;
bool washing = false;
int sensorValue = HIGH;
const int TIMEZONE = 5;
int numTicksRequired = 10;
int delayBetweenTicks = 1000; // 1 second
int resetInterval = 10000; // 10 seconds

WiFiClient wClient;
ESP8266WebServer server(SERVER_PORT);


void setup() {
  log("I/system: startup");
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  setupWifi();
  setupTime();
  server.on("/", []() {
    server.send(200, "text/html", "\
      <a href=\"/log\">/log</a><br>\
      <a href=\"/reboot\">/reboot</a><br>\
    ");
    log("I/server: served / to " + server.client().remoteIP().toString());
  });
  server.on("/log", []() {
    server.send(200, "text/plain", logMsg);
    log("I/server: served /log to " + server.client().remoteIP().toString());
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
  sensorValue = digitalRead(SENSOR_PIN);
  if (!washing) {
    if (sensorValue == LOW && millis() > (lastTickTime + delayBetweenTicks)) {
      tickCount++;
      lastTickTime = millis();
      log("I/ticker: vibration detected. tickCount => " + String(tickCount));
    }
    if (tickCount > numTicksRequired) {
      washing = true;
      log("I/ticker: washing started!");
    }
  } else {
    if (sensorValue == LOW) {
      if (millis() > (lastTickTime + delayBetweenTicks)) {
        lastTickTime = millis();
        tickCount++;
        log("I/ticker: vibration detected. tickCount => " + String(tickCount));
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

  if (!washing && (millis() - lastTickTime) > resetInterval) {
    if (tickCount > 0) {
      tickCount = 0;
      log("I/ticker: ticker reset due to inactivity! tickCount => " + String(tickCount));
    }
  }

  if (washing && (millis() - lastTickTime) > resetInterval) {
    tickCount--;
    log("I/ticker: ticker reduced due to inactivity! tickCount => " + String(tickCount));
    lastTickTime = millis();
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
