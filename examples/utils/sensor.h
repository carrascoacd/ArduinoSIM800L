#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <DHT.h>
#include "HTTP.h"

#define TEMPERATURE_HUMIDITY_PIN 6
#define MOISTURE_PIN 1
#define DHTTYPE DHT11

DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);

unsigned int readHumidity()
{
  dht.begin();
  return dht.readHumidity();
}

unsigned int readTemperature()
{
  dht.begin();
  return dht.readTemperature();
}

unsigned int readMoisture()
{
  return 0;
}

unsigned int readLitioVoltage()
{
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);            // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC))
    ;
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

#endif __SENSOR_H__