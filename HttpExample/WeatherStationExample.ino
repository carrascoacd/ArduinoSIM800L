#include <ArduinoJson.h>
#include <Http.h>

unsigned long lastRunTime = 0;
unsigned long waitForRunTime = 0;

unsigned int RX_PIN = 7;
unsigned int TX_PIN = 8;
unsigned int RST_PIN = 12;
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
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

bool shouldTrackTimeEntry(){
  /*
   * This calculation uses the max value the unsigned long can store as key. Remember when a negative number 
   * is assigned or the maximun is exceeded, then the module is applied to that value.
   */
  unsigned long elapsedTime = millis() - lastRunTime;
  print(F("Elapsed time: "), elapsedTime);
  return elapsedTime >= waitForRunTime;
}

void readVoltage(char *voltageBuffer){
   /* 
   * We are using a voltage divider, so the voltage in each resistence is calculated by:
   * Vn = (Rn / R1 + .. + Rn) * Vt. In our example: V = (270/270 + 270) * 5V = 2.5v
   */
  int input = analogRead(0);    // Read the input pin. Possible values are betwen 0 (0v) to 1023 (5v)
  float maximumBatterVoltage = 5.0;
  float maximumAnalogicIput = 1023.0;
  float dividedVoltage = maximumBatterVoltage * input / maximumAnalogicIput;
  
  float voltage = dividedVoltage * 2; // We use 2 because of our two resistences are equal
   /*
   * As the Arduino implementaion of sprintf does not support float values we need to convert it to char
   */
  dtostrf(voltage, 4, 2, voltageBuffer);
}

void trackTimeEntry(){
  
  char response[32];
  char body[90];
  Result result;

  print(F("Cofigure bearer: "), http.configureBearer("movistar.es"));
  result = http.connect();
  print(F("HTTP connect: "), result);

  unsigned int moisture = random(1023);
  char voltage[6];
  readVoltage(voltage);
  
  sprintf(body, "[{\"weatherEntries\":[{\"moisture\": %d, \"currentVoltage\": %s}], \"name\": \"Arduino\"}]", moisture, voltage);
  Serial.println(body);
  result = http.post("your.domain/api/devices", body, response);
  print(F("HTTP POST: "), result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    lastRunTime = millis();
    waitForRunTime = root["waitForRunTime"];
    
    print(F("Last run time: "), lastRunTime);
    print(F("Next post in: "), waitForRunTime);
  }
  
  print(F("HTTP disconnect: "), http.disconnect());
}