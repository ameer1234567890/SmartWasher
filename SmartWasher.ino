#include <Arduino.h>

#define BUZZER_PIN 12
#define SENSOR_PIN A0
#define LED_PIN 13
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

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Start up");
}

void loop() {
  sensorValue = analogRead(SENSOR_PIN);
  //Serial.println(sensorValue);
  if (!washing) {
    digitalWrite(LED_PIN, 0);
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
      delay(delayBetweenChecks);
    }
    if (probableStart >= numTicks) {
      washing = true;
      Serial.print(probableStart);
      Serial.println(" ticks registered! Washer start deemed!");
    }
  } else {
    digitalWrite(LED_PIN, 1);
    if (sensorValue > minSense) {
      Serial.print("Started - sensorValue: ");
      Serial.println(sensorValue);
      if (probableFinish == 0) {
        startMillis = millis();
      }
      probableFinish++;
      currentMillis = millis();
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
    if (probableFinish >= numTicks) {
      washing = false;
      Serial.print(probableFinish);
      Serial.println(" ticks registered! Washer start deemed!");
    }
  }
}
