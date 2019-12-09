#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <SD.h>
#include <Arduino.h>

void info(const __FlashStringHelper *message, bool newLine = TRUE)
{
  File file = SD.open("l", FILE_WRITE);
  if (file)
  {
    newLine ? file.println(message) : file.print(message);
    file.close();
  }
  newLine ? Serial.println(message) : Serial.print(message);
}

void info(long message, bool newLine = TRUE)
{
  File file = SD.open("l", FILE_WRITE);
  if (file)
  {
    newLine ? file.println(message) : file.print(message);
    file.close();
  }
  newLine ? Serial.println(message) : Serial.print(message);
}

void info(char message)
{
  File file = SD.open("l", FILE_WRITE);
  if (file)
  {
    file.print(message);
    file.close();
  }
  Serial.print(message);
}

void info(const char *message, bool newLine = TRUE)
{
  File file = SD.open("l", FILE_WRITE);
  if (file)
  {
    newLine ? file.println(message) : file.print(message);
    file.close();
  }
  newLine ? Serial.println(message) : Serial.print(message);
}

#endif __LOGGER_H__