[![Build Status](https://travis-ci.org/ameer1234567890/SmartWasher.svg?branch=master)](https://travis-ci.org/ameer1234567890/SmartWasher)

# SmartWasher
A smart washer using Arduino

#### How it works
* Arduino waits for vibrations on the sensor, called __ticks__.
* When a tick is detected, a variable named `probableStart` is incremented.
* Simultaneously, a counter is started, counting down to 45 seconds.
* When `probableStart` reaches 5 (5 tickds), washer is deemed to have started.
* If the aforementioned countdown finishes before reaching 5 ticks, `probableStart` is reset to zero.

* While washing is started, Arduino waits for the ticks to continue.
* If ticks do not continue for a period of 45 seconds, it is deemed that washer is finished.
* For this to work, a variable named `probableFinish` is incremented every time a tick is detected.
* `probableFinish` variable is decremented every time a tick is not detected in a 5 second time period.

* When washer is finished, a notification is sent via IFTTT webhook.
* Additionally, the cycle is started from the begining, where Auduino waits for ticks to start washing.

#### Components Used
* Raspberry Pi 3B+
* DHT22
* Soil Moisture Sensor KY70
* Analog to Digital Converter (ADC) YL-40
* 15x2 I2C LCD with PCF8574 as driver

#### Pinout for DHT22
```
Wemos D1  -  Vibration Sensor
-----------------------------
5V        -  VCC
GND       -  GND
A0        -  SIG
```
