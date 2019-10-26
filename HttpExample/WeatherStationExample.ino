#include <ArduinoJson.h>
#include <Http.h>
#include <Ftp.h>
#include <DHT.h>
#include <LowPower.h>
#include <SD.h>
//#include <Battery.h>

#define MOISTURE_PIN 1
#define TEMPERATURE_HUMIDITY_PIN 6
#define OPEN_VALVE_PIN 4
#define CLOSE_VALVE_PIN 5
#define LITIO_BATTERY_PIN 0
#define SD_CS_PIN 3
#define RST_PIN 8
#define RX_PIN 10
#define TX_PIN 9

#define MAX_VOLTAGE 4200
#define MIN_VOLTAGE 3500
#define BAUD_RATE 19200
#define MAX_RETRIES 10

#define DEBUG TRUE
#define DHTTYPE DHT11

//#define BEARER "movistar.es"
#define BEARER "gprs-service.com"
#define FTP_SERVER "34.255.8.59"
//#define FTP_SERVER "ftp.drivehq.com"
#define FTP_USER "user"
#define FTP_PASS "password"

#define CLOSE_VALVE_STATE 0
#define OPEN_VALVE_STATE 1

//const PROGMEM Battery litio(MIN_VOLTAGE, MAX_VOLTAGE, LITIO_BATTERY_PIN);

unsigned long timeToSleep = 0;
unsigned long elapsedTime = 0;
unsigned int retries = 0;
bool currentState = CLOSE_VALVE_STATE;

/*
 * functions
 */

void(*resetArudino) (void) = 0;
 
void openValve(){
  Serial.print(F("Open valve. "));
  
  pinMode(OPEN_VALVE_PIN, OUTPUT);
  digitalWrite(OPEN_VALVE_PIN, LOW);
  delay(15000);
  pinMode(OPEN_VALVE_PIN, INPUT);

  //valveIsOpen() ? Serial.println(F("Open")) : Serial.println(F("Closed"));
}

void closeValve(){
  Serial.print(F("Close Valve. "));
  
  pinMode(CLOSE_VALVE_PIN, OUTPUT);
  digitalWrite(CLOSE_VALVE_PIN, LOW);
  delay(15000);
  pinMode(CLOSE_VALVE_PIN, INPUT);

  //valveIsOpen() ? Serial.println(F("Open")) : Serial.println(F("Closed"));
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

unsigned int readHumidity(){
  DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);
  dht.begin();
  return dht.readHumidity();
}

unsigned int readTemperature(){
  DHT dht(TEMPERATURE_HUMIDITY_PIN, DHTTYPE);
  dht.begin();
  return dht.readTemperature();
}

unsigned int readLitioVoltage() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

//unsigned int readLitioVoltage(){
//  unsigned int voltage = 0;
//  for (unsigned int i=0; i<100; ++i){
//    unsigned int cv = litio.voltage();
//    if (cv > voltage){
//      voltage = cv;
//    }
//  }
//  return voltage;
//}

unsigned int readLipoVoltage(){
  HTTP http(BAUD_RATE, RX_PIN, TX_PIN, RST_PIN, DEBUG);
  unsigned int voltage = 0;
  for (unsigned int i=0; i<10; ++i){
    unsigned int cv = http.readVoltage();
    if (cv > voltage){
      voltage = cv;
    }
  }
  return voltage;
}

void sleep(){
  while (elapsedTime <= timeToSleep){ 
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    elapsedTime += 7000;
  }
}

unsigned int availableMemory(){
  int size = 1024;
  byte *buf;
 
  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);

  return size;
}

void uploadFile(File dataFile){
  if (dataFile){
    FTP ftp(BAUD_RATE, RX_PIN, TX_PIN, RST_PIN, DEBUG);
    ftp.configureBearer(BEARER);

    while (ftp.putBegin("image.jpg", FTP_SERVER, FTP_USER, FTP_PASS, "/") != SUCCESS);
    
    unsigned int i;
    unsigned int chunkSize = availableMemory() - 10;
    unsigned int writes = ceil(dataFile.size() / chunkSize);
    char buff[chunkSize];
    
    while(dataFile.available()){
      i = 0;
      while (i < chunkSize){
        buff[i] = dataFile.read();
        ++i;
      }
      
      ftp.putWrite(buff, i);
      writes --;
      
      Serial.print(F("Pending: "));
      Serial.print(writes);
      Serial.print(F("/"));
      Serial.println(ceil(dataFile.size() / chunkSize));
    }

    ftp .putEnd();
  }
  else {
    Serial.println(F("Error opening the file"));
  }
}

void postEntry(){
  unsigned int humidity = readHumidity();
  Serial.print(F("Humidity % "));
  Serial.println(humidity);
  
  unsigned int temperature = readTemperature();
  Serial.print(F("Temperature Cº "));
  Serial.println(temperature);
  
  unsigned int moisture = readMoisture();
  Serial.print(F("Moisture "));
  Serial.println(moisture);
 
  unsigned int litioVoltage = readLitioVoltage();
  Serial.print(F("Litio voltage "));
  Serial.println(litioVoltage);

  unsigned int liPoVoltage = 10;//readLipoVoltage();
  Serial.print(F("LiPo voltage "));
  Serial.println(liPoVoltage);

  /*
  const char ENDPOINT[] = {"https://api/something"};
  const char BODY_FORMAT[] PROGMEM = {"{\"w\":{\"m\":%d,\"t\":%d,\"h\":%d,\"mv\":%d,\"sv\":%d,\"st\":%d}}"};

  http.wakeUp();
  
  char response[32];
  char body[70];
  Result result;
  sprintf_P(body, BODY_FORMAT, 
    moisture,
    temperature,
    humidity,
    litioVoltage,
    liPoVoltage,
    currentState
  );
  Serial.println(body);

  http.configureBearer(BEARER);
  http.connect();
  result = http.post(ENDPOINT, body, response);
  
  http.disconnect();*/
}

void manageGarden(){
  File image = SD.open("image.jpg");
  uploadFile(image);
  image.close();
  while(1);
  
  /*if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    
    if (strcmp(root["action"], "open-valve") == 0){
      openValve();
    }
    else if (strcmp(root["action"], "close-valve") == 0){
      closeValve();
    }
    else if (strcmp(root["action"], "reset") == 0) {
      resetArudino();
    }

    currentState = valveIsOpen() ? OPEN_VALVE_STATE : CLOSE_VALVE_STATE;
    timeToSleep = root["value"];
    // 51848 is the offset the arduino millis have per hour
    // timeToSleep -= 26848 * timeToSleep/(60*60*1000);
    elapsedTime = 0;
    retries = 0;
  }
  else {
    Serial.print(F("Error: "));
    Serial.println(result);
    retries += 1;
    if (retries == MAX_RETRIES) {
      retries = 0;
      elapsedTime = 0;
    }
  }*/
}

void initialize(){
  //if (DEBUG){
    Serial.begin(BAUD_RATE);
    while(!Serial);
    Serial.println(F("Starting!"));
  //}
  
  //litio.begin(4200, 1.0);
  //openValveFor(0);

  if (!SD.begin(SD_CS_PIN)){
    Serial.println(F("Error starting SD"));
    return;
  }
  currentState = CLOSE_VALVE_STATE;
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
  manageGarden();
  sleep();

  // If the module is not working properly twice the sleep time, restart it
  if (elapsedTime >= timeToSleep*2 && timeToSleep > 0){
    Serial.println(F("Reset!"));
    resetArudino();
  }
}
