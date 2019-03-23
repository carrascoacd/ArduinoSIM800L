
#include <ArduinoJson.h>
#include <Http.h>
#include <DHT.h>
#include <Battery.h>
#include <LowPower.h>
#include <NewPing.h>

#define TRIGGER_PIN 7
#define ECHO_PIN 8
#define MAX_DISTANCE 100
#define RST_PIN 11
#define RX_PIN 10
#define TX_PIN 9
#define MOISTURE_PIN 1
#define TEMPERATURE_HUMIDITY_PIN 6
#define DHTTYPE DHT11
#define OPEN_VALVE_PIN 5
#define OPEN_VALVE_VIN_PIN 4
#define LITIO_BATTERY_PIN 0
//#define BEARER "movistar.es"
#define BEARER "gprs-service.com"
#define MAX_VOLTAGE 4200
#define MIN_VOLTAGE 3500
#define DEBUG TRUE

const char ENDPOINT[] = {"https://your.api"};
const char BODY_FORMAT[] PROGMEM = {"{\"w\":{\"m\": %d, \"t\": %d, \"h\": %d, \"mv\": %d, \"sv\": %d, \"v\": %d}}"};

const PROGMEM HTTP http(9600, RX_PIN, TX_PIN, RST_PIN, DEBUG);
const PROGMEM DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);
const PROGMEM Battery litioBattery(MIN_VOLTAGE, MAX_VOLTAGE, LITIO_BATTERY_PIN);
const PROGMEM NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

unsigned long timeToSleep = 0;
unsigned long starToSleepTime = 0;

/*
 * functions
 */
void openValve(){
  Serial.print(F("Open valve"));
  
  pinMode(OPEN_VALVE_VIN_PIN, OUTPUT);
  pinMode(OPEN_VALVE_PIN, OUTPUT);
  digitalWrite(OPEN_VALVE_PIN, HIGH);
  delay(15000);
  pinMode(OPEN_VALVE_VIN_PIN, INPUT);
  pinMode(OPEN_VALVE_PIN, INPUT);
}

void closeValve(){
  Serial.println(F("Close Valve"));
  
  pinMode(OPEN_VALVE_VIN_PIN, OUTPUT);
  pinMode(OPEN_VALVE_PIN, OUTPUT);
  digitalWrite(OPEN_VALVE_PIN, LOW);
  delay(15000);
  pinMode(OPEN_VALVE_VIN_PIN, INPUT);
  pinMode(OPEN_VALVE_PIN, INPUT);
}

void openValveFor(unsigned long milliseconds){
  openValve();
  delay(milliseconds);
  closeValve();
}

unsigned int readMoisture() {
  unsigned long total = 0;
  for (unsigned int i=0; i<100; ++i){
    total += analogRead(MOISTURE_PIN);
  }
  return total/100;
}

bool rightVoltage(unsigned int voltage){
  return voltage >= MIN_VOLTAGE;
}

bool shouldSleep(){
  return millis() - starToSleepTime <= timeToSleep;
}

//int availableMemory() 
//{
// int size = 1024;
// byte *buf;
//
// while ((buf = (byte *) malloc(--size)) == NULL)
//   ;
//
// free(buf);
//
// return size;
//}

void manageGarden(){ 
  unsigned int humidity = dht.readHumidity();
  Serial.print(F("Humidity % "));
  Serial.println(humidity);
  
  unsigned int temperature = dht.readTemperature();
  Serial.print(F("Temperature Cº "));
  Serial.println(temperature);
  
  unsigned int moisture = readMoisture();
  Serial.print(F("Moisture "));
  Serial.println(moisture);
 
  unsigned int litioBatteryVoltage = litioBattery.voltage();
  Serial.print(F("Litio voltage "));
  Serial.println(litioBatteryVoltage);
  
  Serial.print(F("Water tank distance cm: "));
  unsigned int distance = sonar.ping_cm();
  Serial.println(distance);

  http.wakeUp();
  unsigned int liPoBatteryVoltage = http.readVoltage();
  Serial.print(F("LiPo voltage "));
  Serial.println(liPoBatteryVoltage);

  if (rightVoltage(liPoBatteryVoltage)){
    char response[32];
    char body[70];
    Result result;
    sprintf_P(body, BODY_FORMAT, moisture, temperature, humidity, litioBatteryVoltage, liPoBatteryVoltage, distance);
    Serial.println(body);

    http.configureBearer(BEARER);
    result = http.connect();
    result = http.post(ENDPOINT, body, response);
    http.disconnect();
    http.sleep();
    
    if (result == SUCCESS) {
      Serial.println(response);
      StaticJsonBuffer<32> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(response);
      
      if (strcmp(root["action"], "open-valve") == 0){
        // Delay 1 minute to overpass the irrigation time
        unsigned long sleepTime = root["value"];
        openValveFor(sleepTime + 60000);
      }
      else {
        timeToSleep = root["value"];
        // 26848 is the offset the arduino millis have per hour
        timeToSleep -= 26848 * timeToSleep/(60*60*1000);
        starToSleepTime = millis();
      }
    }
  }
  else {
    http.sleep();
    Serial.println(F("Low voltage detected. I can not work property, charge me!"));
    delay(10000);
  }
}

void initialize(){
  if (DEBUG){
    Serial.begin(9600);
    while(!Serial);
    Serial.println(F("Starting!"));
  }
  
  delay(1000);
  
  openValveFor(0);
  
  dht.begin();
  litioBattery.begin(5000, 1.02);
}

/*
 * the setup routine runs once when you press reset:
 */
void setup() {
  initialize();
}

/*
 * the loop routine runs over and over again forever:
 */
void loop(){
  if (!shouldSleep()){
    manageGarden();
  }  
}
