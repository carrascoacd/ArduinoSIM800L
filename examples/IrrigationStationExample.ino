#include <LowPower.h>
#include <DHT.h>

#define BATTERY_PIN 2
#define PROBE_PIN 6
#define PUMP_MOFSET_PIN 15 // Used to switch the power of the pump with the MOFSET
#define LED_PIN 17
#define TEMP_HUM_PIN 18
#define VIN_PROBE_PIN 19

DHT dht(TEMP_HUM_PIN, DHT11);

unsigned long oneMinuteMs = 60000;
unsigned long oneHourMs = 3600000;

unsigned long irrigationTimeMs = 10000;
unsigned long sleepTimeSeconds = 24 * oneHourMs / 1000;
unsigned int irrigationLoops = 6;
const int dryThreshold = 942;
/*
 * >= 13 volts = 100%
 * 12.75 volts 90%
 * 12.5 volts = 80%
 * 12.3 volts = 70%
 * Don't go bellow 70%
*/
float minBatteryVoltage = 12.5;

// Set to true to test the moisture
bool test = false;
bool noWater = false;

void setCPUAt8Mhz(){
  noInterrupts();
  CLKPR = 1<<CLKPCE;
  CLKPR = 1;
  interrupts();
}
  
void setup() {
  setCPUAt8Mhz();
  Serial.begin(19200);
  Serial.println("Starting!");
  pinMode(PROBE_PIN, INPUT);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(PUMP_MOFSET_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(VIN_PROBE_PIN, OUTPUT);
  pinMode(TEMP_HUM_PIN, INPUT);
  dht.begin();
}

int readMoistureDiscarding(){
  int moisture;
  int times = 3;
  
  for (int i = 0; i < times; ++i){
      moisture = readMoisture();  
      delay(1000);
  }
  
  return moisture;
}

int readMoisture(){
  digitalWrite(VIN_PROBE_PIN, HIGH);
  delay(10);
  
  int times = 100;
  long total = 0;
  for (int i = 0; i < times; ++i){
    total += analogRead(PROBE_PIN);
    delay(10);
  }
  digitalWrite(VIN_PROBE_PIN, LOW);
  return total/times;
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

void openPump(){
  Serial.println("Open pump");

  digitalWrite(PUMP_MOFSET_PIN, HIGH);
}

void closePump(){
  Serial.println("Close pump");

  digitalWrite(PUMP_MOFSET_PIN, LOW);
}

float readBatteryVoltage(){
  float batteryReferenceV = 12.37;
  float batteryReferenceInput= 819;
  float batteryInput = analogRead(BATTERY_PIN);
  
  return batteryInput * batteryReferenceV / batteryReferenceInput;
}

void maybeReportLowBattery(float voltage){
  if (voltage < minBatteryVoltage)  {
    Serial.println("Low battery!");
  
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(150);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}

int calculateIrrigationLoops(){
  const float maxTemperature = 30.0;
  float temperature = min(maxTemperature, readTemperature());
  
  float seasonFactor = temperature / maxTemperature;
  
  return round(irrigationLoops * seasonFactor);
}

void loop() {
  while (test){    
    Serial.print("Moisture: ");
    Serial.println(readMoisture());
    
    Serial.print("D. Moisture: ");
    Serial.println(readMoistureDiscarding());
    
    Serial.print("Battery V: ");
    Serial.println(readBatteryVoltage());
    
    Serial.print("Battery bytes: ");
    Serial.println(analogRead(BATTERY_PIN));

    Serial.print("Temperature: ");
    Serial.println(readTemperature());

    Serial.print("Humidity: ");
    Serial.println(readHumidity());

    maybeReportLowBattery(0);
    
    delay(1000);
  }
  Serial.flush();
  while (noWater) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  
  int loops = calculateIrrigationLoops(); 
  Serial.println(loops);
  
  for (int i = 0; i < loops; ++i){
    openPump();
    delay(irrigationTimeMs);
    closePump();
    
    // Let the soil to absorb the water so no drops fall when it is too wet.
    delay(oneMinuteMs * 5);
  }

  closePump();

  unsigned long times = sleepTimeSeconds/8;
  Serial.print("Sleep: ");
  Serial.println(times);
  
  for (unsigned long i = 0; i < times; ++i) {
    Serial.flush();
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    Serial.flush();
    
    // Report it only once every minute more or less
    if (times % 10 == 0) { 
      float voltage = readBatteryVoltage();
      Serial.print("Voltage: ");
      Serial.println(voltage);
      
      maybeReportLowBattery(voltage);
    }
  }
}