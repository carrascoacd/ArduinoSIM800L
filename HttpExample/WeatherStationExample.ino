#include <LowPower.h>
#include <ArduinoJson.h>
#include <Http.h>

unsigned long lastRunTime = 0;
unsigned long waitForRunTime = 0;
unsigned long millisOffset = 0;
unsigned int RX_PIN = 11;
unsigned int TX_PIN = 12;
unsigned int RST_PIN = 10;
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN, true);

/*
 * the setup routine runs once when you press reset:
 */
void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Starting!");
}

/*
 * the loop routine runs over and over again forever:
 */
void loop() {
  if (shouldTrackTimeEntry()){
    http.wakeUp();
    trackTimeEntry();
  }
  else {
    http.sleep();
  }
}

/*
 * functions
 */
void print(const __FlashStringHelper *message, int code = -1){
  if (code != -1){
    Serial.print(message);
    Serial.println(code);
  }
  else {
    Serial.println(message);
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

bool shouldTrackTimeEntry(){
  /*
   * This calculation uses the max value the unsigned long can store as key. Remember when a negative number 
   * is assigned or the maximun is exceeded, then the module is applied to that value.
   */ 
  sleepEightSeconds();
  unsigned long elapsedTime = currentMillis() - lastRunTime;
  print(F("Elapsed time: "), elapsedTime);
  return elapsedTime >= waitForRunTime;
}

void trackTimeEntry(){
  
  char response[32];
  char body[90];
  Result result;

  print(F("Cofigure bearer: "), http.configureBearer("your.apn"));
  result = http.connect();
  print(F("HTTP connect: "), result);

  unsigned int moisture = random(1023);
  char voltage[6];
  http.readVoltage(voltage);

  sprintf(body, "[{\"weatherEntry\":[{\"m\": %d, \"cv\": %s}], \"n\": \"Arduino\"}]", moisture, voltage);
  Serial.println(body);
  
  result = http.post("your.api", body, response);
  print(F("HTTP POST: "), result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    lastRunTime = currentMillis();
    waitForRunTime = root["waitForRunTime"];
    
    print(F("Last run time: "), lastRunTime);
    print(F("Next post in: "), waitForRunTime);
  }
  
  print(F("HTTP disconnect: "), http.disconnect());
}