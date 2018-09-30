
#include <ArduinoJson.h>
#include <Http.h>
#include <DHT.h>
#include <Battery.h>
//#include <NewPing.h>

//#define TRIGGER_PIN 5
//#define ECHO_PIN 6
//#define MAX_DISTANCE 100
#define RX_PIN 11
#define TX_PIN 12
#define RST_PIN 10
#define MOISTURE_PIN 7
#define TEMPERATURE_HUMIDITY_PIN 13
#define DHTTYPE DHT11
#define OPEN_VALVE_PIN 5
#define LIPO_BATTERY_PIN 5
#define LITIO_BATTERY_PIN 6
#define ENDPOINT "https://your.api"
#define BODY_FORMAT "{\"w\":{\"m\": %d, \"t\": %d, \"h\": %d, \"mv\": %d, \"sv\": %d}}"
//#define BEARER "gprs-service.com"
//#define BEARER "movistar.es"
#define BEARER "gprs-service.com"
#define DEBUG TRUE

unsigned long waitForRunTime = 1;
const HTTP http(9600, RX_PIN, TX_PIN, RST_PIN, DEBUG);
const DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);
Battery liPoBattery(3300, 3800, LIPO_BATTERY_PIN);
Battery litioBattery(3300, 3800, LITIO_BATTERY_PIN);
//const NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

/*
 * functions
 */

void openValveFor(unsigned long milliseconds){
  Serial.print(F("Open valve for: "));
  Serial.println(milliseconds);
  digitalWrite(OPEN_VALVE_PIN, LOW);
  delay(10000 + milliseconds);
  Serial.println(F("Close Valve"));
  digitalWrite(OPEN_VALVE_PIN, HIGH);
}

unsigned int readMoisture() {
  unsigned long total = 0;
  for (unsigned int i=0; i<100; ++i){
    total += analogRead(MOISTURE_PIN);
  }
  return total/100;
}

void manageGarden(){
  int humidity = dht.readHumidity();
  Serial.print(F("Humidity % "));
  Serial.println(humidity);
  int temperature = dht.readTemperature();
  Serial.print(F("Temperature "));
  Serial.println(temperature);
  unsigned int moisture = readMoisture();
  Serial.print(F("Moisture "));
  Serial.println(moisture);
  unsigned int litioBatteryVoltage = litioBattery.voltage();
  Serial.print(F("Litio voltage "));
  Serial.println(litioBatteryVoltage);
  unsigned int liPoBatteryVoltage = liPoBattery.voltage();
  Serial.print(F("LiPo voltage "));
  Serial.println(liPoBatteryVoltage);
  //Serial.print(F("Water tank distance cm: "));
  //float distance = sonar.ping_cm();
  //Serial.println(distance);
  
  char response[32];
  char body[90];
  Result result;
  sprintf(body, BODY_FORMAT, moisture, temperature, humidity, litioBatteryVoltage, liPoBatteryVoltage);
  Serial.println(body);


  http.configureBearer(BEARER);
  result = http.connect();
  result = http.post(ENDPOINT, body, response);
  http.disconnect();
  
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    
    if (strcmp(root["action"], "open-valve") == 0){
      openValveFor(root["value"]);
      // Delay 1 minute to overpass the irrigation time
      delay(60000);
    }
    else {
      delay(root["value"]);
    }
  }
}

/*
 * the setup routine runs once when you press reset:
 */
void setup() {
  if (DEBUG){
    Serial.begin(9600);
    while(!Serial);
    Serial.println(F("Starting!"));
  }
  
  pinMode(MOISTURE_PIN, INPUT);
  pinMode(OPEN_VALVE_PIN, OUTPUT);
  pinMode(OPEN_VALVE_PIN, HIGH);
  
  openValveFor(1000);
  dht.begin();
  liPoBattery.begin(5000, 1.0);
  litioBattery.begin(5000, 1.0);
}

/*
 * the loop routine runs over and over again forever:
 */
void loop() {
  http.wakeUp();
  manageGarden();
  http.sleep();
}
