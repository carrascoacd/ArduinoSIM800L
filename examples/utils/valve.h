#ifndef __VALVE_H__
#define __VALVE_H__

#include <Arduino.h>
#include "logger.h"

#define VALVE_SWITCH_PIN 4
#define VALVE_OPEN_CLOSE_PIN 2

void moveValve(byte value)
{
  // Switch the solar input to power the valve
  pinMode(VALVE_SWITCH_PIN, OUTPUT);
  pinMode(VALVE_OPEN_CLOSE_PIN, OUTPUT);

  digitalWrite(VALVE_SWITCH_PIN, LOW);
  digitalWrite(VALVE_OPEN_CLOSE_PIN, value);

  delay(15000);

  pinMode(VALVE_OPEN_CLOSE_PIN, INPUT);
  pinMode(VALVE_SWITCH_PIN, INPUT);
}

void openValve()
{
  info(F("Open valve"));
  moveValve(HIGH);
}

void closeValve()
{
  info(F("Close Valve"));
  moveValve(LOW);
}

#endif __VALVE_H__