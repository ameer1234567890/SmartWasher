## SmartWasher
A smart washer using Arduino

**This is a WIP (Work in progress)**

#### How it works
* Arduino waits for vibrations on the sensor, called __ticks__.
* When a tick is detected, a variable named `probableStart` is incremented.
* Simultaneously, a counter is started, counting down to 45 seconds.
* When `probableStart` reaches 5 (5 tickds), the washer is deemed to have started.
* If the aforementioned countdown finishes before reaching 5 ticks, `probableStart` is reset to zero.
