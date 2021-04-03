#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <DHT.h>
#include "HTTP.h"

#define TEMPERATURE_HUMIDITY_PIN 6
#define MOISTURE_PIN 7
#define DHTTYPE DHT11

DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);

void initializeSensors(){
  dht.begin();
  pinMode(MOISTURE_PIN, INPUT);
}

unsigned int readHumidity()
{
  unsigned int h = dht.readHumidity();
  byte retry = 0;
  while (isnan(h) && retry < 5) 
  {
    delay(250);
    h = dht.readHumidity();
    retry ++;
  } 
  return h;
}

unsigned int readTemperature()
{
  unsigned int t = dht.readTemperature();
  byte retry = 0;
  while (isnan(t) && retry < 5) 
  {
    delay(250);
    t = dht.readTemperature();
    retry ++;
  } 
  return t;
}

unsigned int readMoisture()
{
  /*
    Dry 676 - Wet 376
  */
  int moisture = 0;
  for (uint8_t i = 0; i <5; ++i){
    int current = analogRead(MOISTURE_PIN);
    if (current > moisture) moisture = current;
    delay(250);
  }
  return (unsigned int)moisture;
}

unsigned int readLitioVoltage()
{
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
     ADMUX = _BV(MUX5) | _BV(MUX0) ;
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  // Taking 1.1 as Vref: Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  // In this example I used the Vref of 1.08
  result = 1104840 / result; 
  return result; // Vcc in millivolts
}

#endif __SENSOR_H__
