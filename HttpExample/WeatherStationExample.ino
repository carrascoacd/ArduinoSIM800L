#include <LowPower.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <Http.h>

#define RX_PIN 11
#define TX_PIN 12
#define RST_PIN 10
#define MOISTURE_PIN 3
#define SERVO_PIN 4

unsigned long lastRunTime = 0;
unsigned long waitForRunTime = 0;
unsigned long millisOffset = 0;
const bool DEBUG = false;
const HTTP http(9600, RX_PIN, TX_PIN, RST_PIN, DEBUG);
const Servo servo;

/*
 * functions
 */

void openValve(){
  servo.attach(SERVO_PIN);
  servo.write(14);
  delay(1000);
  servo.detach();
}

void closeValve(){
  servo.attach(SERVO_PIN);
  servo.write(90);
  delay(1000);
  servo.detach();
}
 
void print(const __FlashStringHelper *message, int code = -1){
  if (DEBUG){
    if (code != -1){
      Serial.print(message);
      Serial.println(code);
    }
    else {
      Serial.println(message);
    }
  }
}

void print(const char *message, int code = -1){
  if (DEBUG){
    if (code != -1){
      Serial.print(message);
      Serial.println(code);
    }
    else {
      Serial.println(message);
    }
  }
}

unsigned long currentMillis(){
  return millis() + millisOffset;
}

void sleepEightSeconds(){
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  /* The 8000 is 8 seconds, the time the clock has been sleeping */
  millisOffset += 8000;
}

bool shouldTrackWeatherEntry(){
  /*
   * This calculation uses the max value the unsigned long can store as key. Remember when a negative number 
   * is assigned or the maximun is exceeded, then the module is applied to that value.
   */ 
  unsigned long elapsedTime = currentMillis() - lastRunTime;
  print(F("Elapsed time: "), elapsedTime);
  return elapsedTime >= waitForRunTime;
}

unsigned int readMoisture() {
  unsigned long total = 0;
  for (unsigned int i=0; i<100; ++i){
    total += analogRead(MOISTURE_PIN);
  }
  return total/100;
}

void manageGarden(){
  
  char response[32];
  char body[90];
  Result result;

  print(F("Cofigure bearer: "), http.configureBearer("bearer"));
  result = http.connect();
  print(F("HTTP connect: "), result);

  // char voltage[6];
  // http.readVoltage(voltage);

  sprintf(body, "{\"w\":{\"m\": %d}}", readMoisture());
  print(body);
  
  result = http.post("your.endpoint", body, response);
  print(F("HTTP POST: "), result);
  print(F("HTTP disconnect: "), http.disconnect());
  
  if (result == SUCCESS) {
    print(response);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    
    if (strcmp(root["action"], "open-valve") == 0){
      print(F("Opening valve for: "), root["value"]);
      openValve();
      delay(root["value"]);
      closeValve();
    }
    else {
      lastRunTime = currentMillis();
      print(F("Wait for run time: "), root["value"]);
      waitForRunTime = root["value"];
    }
  }
}

/*
 * the setup routine runs once when you press reset:
 */
void setup() {
  pinMode(MOISTURE_PIN, INPUT);
  Serial.begin(9600);
  while(!Serial);
  print("Starting!");
  openValve();
  closeValve();
}

/*
 * the loop routine runs over and over again forever:
 */
void loop() {
  if (shouldTrackWeatherEntry()){
    http.wakeUp();
    manageGarden();
  }
  else {
    http.sleep();
    sleepEightSeconds();
  }
}
