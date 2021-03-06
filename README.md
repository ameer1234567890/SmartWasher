[![Build Status](https://travis-ci.org/ameer1234567890/SmartWasher.svg?branch=master)](https://travis-ci.org/ameer1234567890/SmartWasher)

# SmartWasher
A smart washer using Arduino

__This is a work in progress (WIP)__

__I am using an Arduino Uno for the initial development, and shall move to a Wemos D1 when IFTTT is required__

#### How it works
* Arduino waits for vibrations on the sensor, called __ticks__.
* When a tick is detected, a variable named `probableStart` is incremented.
* Simultaneously, a counter is started, counting down to 45 seconds.
* When `probableStart` reaches 5 (5 tickds), washer is deemed to have started.
* If the aforementioned countdown finishes before reaching 5 ticks, `probableStart` is reset to zero.
* While washing is started, Arduino waits for the ticks to continue.
* If ticks do not continue for a period of 55 seconds, it is deemed that washer is finished.
* For this to work, a variable named `probableFinish` is incremented every time a tick is detected.
* `probableFinish` and `startMillis` variables are reset when it reaches 45 seconds.
* If `currentMillis` variable reaches 55 seconds, washer is deemed to have finished.
* When washer is finished, a notification is sent via IFTTT webhook.
* Additionally, the cycle is started from the begining, where Auduino waits for ticks to start washing.

#### Components Used
* Wemos D1 / Any ESP8266 based microcontroller
* A vibration sensor with three pins (VCC, GND, SIG)

Pinout for Wemos D1
```
Wemos D1  -  Vibration Sensor
-----------------------------
5V        -  VCC
GND       -  GND
A0        -  SIG
```
