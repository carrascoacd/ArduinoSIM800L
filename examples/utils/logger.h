#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <SD.h>
#include <Arduino.h>

#define LOG_FILE "l"

const char LOG_INT[] PROGMEM = "%d";

void info(const char *message, bool newLine = true)
{
  File file = SD.open(LOG_FILE, FILE_WRITE);
  if (file)
  {
    newLine ? file.println(message) : file.print(message);
    file.close();
  }
  newLine ? Serial.println(message) : Serial.print(message);
}

void info(const __FlashStringHelper *message, bool newLine = true)
{
  File file = SD.open(LOG_FILE, FILE_WRITE);
  if (file)
  {
    newLine ? file.println(message) : file.print(message);
    file.close();
  }
  newLine ? Serial.println(message) : Serial.print(message);
}

void info(long message, bool newLine = true)
{
  char buffer[10];
  // Use sprintf instead of overrided Serial.print in order to
  // save space
  sprintf_P(buffer, LOG_INT, message);
  return info(buffer, newLine);
}

#endif __LOGGER_H__