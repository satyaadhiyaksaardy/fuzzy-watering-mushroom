#include "ActuatorControl.h"
#include <Arduino.h>

void ActuatorControl::init()
{
    pinMode(pumpPin, OUTPUT); // Assuming pump is connected to GPIO 5
    pinMode(fanPin, OUTPUT);  // Assuming fan is connected to GPIO 4

    digitalWrite(pumpPin, HIGH);
    digitalWrite(fanPin, HIGH);
}

void ActuatorControl::controlPump(bool on)
{
    digitalWrite(pumpPin, on ? LOW : HIGH);
    digitalWrite(fanPin, on ? LOW : HIGH);
}
