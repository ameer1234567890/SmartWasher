#define BUZZER_PIN 12
#define SENSOR_PIN A0
int sensorValue;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(SENSOR_PIN);
  //Serial.println(sensorValue);
  if (sensorValue < 1000) {
    Serial.println(sensorValue);
    tone(BUZZER_PIN, 1000);
    delay(50);
    noTone(BUZZER_PIN);
  }
}
