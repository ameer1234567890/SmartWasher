#include <Arduino.h>

#define BUZZER_PIN 12
#define SENSOR_PIN A0
int sensorValue;
int minSense = 950;
int delayBetweenChecks = 5000; // 5 seconds
int numTicks = 5;
int probableStart = 0;
int probableEnd = 0;
unsigned long timeTillDetection = 45000; // 45 seconds
unsigned long startMillis;
unsigned long currentMillis;
bool washerStarted = false;
bool washerFinished = false;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Start up");
}

void loop() {
  sensorValue = analogRead(SENSOR_PIN);
  //Serial.println(sensorValue);
  if (!washerStarted) {
    if (sensorValue < minSense) {
      Serial.print("sensorValue: ");
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
      washerStarted = true;
      Serial.print(probableStart);
      Serial.println(" ticks registered! Washer start deemed!");
    }
  } else if (washerStarted) {
    Serial.println("Washer already started!");
    // Finish detection algorithm goes here
    delay(1000);
  }
}
