#include <Arduino.h>

#define BUZZER_PIN 12
#define SENSOR_PIN A0
int sensorValue;
int minValue = 1000;
int delayBetweenChecks = 5000;
int numTicks = 5;
int probableStart = 0;
int probableEnd = 0;
bool washerStarted = false;
bool washerFinished = false;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(SENSOR_PIN);
  if (!washerStarted) {
    if (sensorValue < minValue) {
      probableStart++;
      tone(BUZZER_PIN, 1000);
      delay(50);
      noTone(BUZZER_PIN);
      delay(delayBetweenChecks);
    }
    if (probableStart > numTicks) {
      washerStarted = true;
      Serial.print(probableStart);
      Serial.println(' ticks registered! Washer start deemed!');
    }
  } else if (washerStarted) {
    Serial.println('Washer already started!');
    delay(1000);
  }
}
