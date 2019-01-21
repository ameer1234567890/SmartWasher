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
  if (washing == false) {
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
      if (probableStart >= numTicks) {
        Serial.print(probableStart);
        Serial.println(" ticks registered! Washer start deemed!");
        washing = true;
        probableStart = 0;
      }
      delay(delayBetweenChecks);
    }
  } else {
    digitalWrite(LED_PIN, 1);
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
      // IFTTT webhook logic goes here...
    }
  }
}
