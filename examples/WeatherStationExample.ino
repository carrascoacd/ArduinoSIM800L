#include <ArduinoJson.h>
#include <LowPower.h>
#include <SD.h>

#include <Http.h>
#include <Ftp.h>

#include "../examples/utils/logger.h"
#include "../examples/utils/valve.h"
#include "../examples/utils/sensor.h"
#include "../examples/utils/arducam.h"

#define SD_CS_PIN 3
#define RST_PIN 8
#define RX_PIN 10
#define TX_PIN 9

#define BAUD_RATE 19200
#define MAX_RETRIES 10

#define CLOSE_VALVE_STATE 0
#define OPEN_VALVE_STATE 1

const char BEARER[] PROGMEM = "your.bearer";
const char FTP_SERVER[] PROGMEM = "ftp.server";
const char FTP_USER[] PROGMEM = "user";
const char FTP_PASS[] PROGMEM = "pass";
const char BODY_FORMAT[] PROGMEM = "{\"w\":{\"m\":%d,\"t\":%d,\"h\":%d,\"mv\":%d,\"sv\":%d,\"st\":%d}}";
const char ENDPOINT[] PROGMEM = "https://your.api.com";
const char OPEN_VALVE[] PROGMEM = "open-valve";
const char CLOSE_VALVE[] PROGMEM = "close-valve";
const char RESET[] PROGMEM = "reset";

unsigned long timeToSleep = 0;
unsigned long elapsedTime = 0;
uint8_t retries = 0;
uint8_t currentImage = 0;
bool currentState = CLOSE_VALVE_STATE;

/*
   functions
*/

void(*reset) (void) = 0;

void resetArudino(){
  info(F("Reset!"));
  delay(1000);
  reset();
}

void sleep() {
  while (elapsedTime <= timeToSleep) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    elapsedTime += 7000;
  }
}

unsigned int readLipoVoltage() {
  HTTP http(BAUD_RATE, RX_PIN, TX_PIN, RST_PIN);
  http.wakeUp();
  unsigned int voltage = 0;
  for (unsigned int i = 0; i < 10; ++i) {
    unsigned int cv = http.readVoltage();
    if (cv > voltage) {
      voltage = cv;
    }
  }
  http.sleep();
  return voltage;
}

unsigned int availableMemory() {
  int size = 1024;
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);

  return size;
}

void uploadFile(const char *filename) {
  File dataFile = SD.open(filename);

  if (dataFile) {
    FTP ftp(BAUD_RATE, RX_PIN, TX_PIN, RST_PIN);
    ftp.wakeUp();
    ftp.putBegin(BEARER, filename, FTP_SERVER, FTP_USER, FTP_PASS);

    unsigned int i;
    unsigned int chunkSize = 64;
    unsigned int writes = ceil(dataFile.size() / chunkSize);
    char buff[chunkSize];
   
    while (dataFile.available()) {
      i = 0;
      while (i < chunkSize) {
        buff[i] = dataFile.read();
        ++i;
      }

      if (ftp.putWrite(buff, i) != SUCCESS){
        return;
      }
      writes --;

      info(F("Pending: "), false);
      info(writes, false);
      info(F("/"), false);
      info(ceil(dataFile.size() / chunkSize), true);
    }

    ftp.putEnd();
    ftp.sleep();
    dataFile.close();
  }
  else {
    info(F("FTP. File open failed: "));
    info(filename);
  }
}

Result postEntry(char *response) {
  info(F("SRAM: "), false);  
  info(availableMemory(), true);
  
  unsigned int temperature = readTemperature();
  unsigned int humidity = readHumidity();
  unsigned int moisture = readMoisture();
  unsigned int litioVoltage = readLitioVoltage();
  unsigned int liPoVoltage = readLipoVoltage();

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
  info(body);
  char endpoint[64];
  strcpy_P(endpoint, ENDPOINT);
  info(endpoint);
  
  HTTP http(BAUD_RATE, RX_PIN, TX_PIN, RST_PIN);
  http.wakeUp();
  http.connect(BEARER);
  result = http.post(endpoint, body, response);
  http.disconnect();
  http.sleep();

  return result;
}

void manageGarden() {
  char buff[32];
  buildImageName(buff, currentImage);
  Serial.println(buff);
  Result result = postEntry(buff);
  
  if (result == SUCCESS) {
    info(buff);
    StaticJsonBuffer<32> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buff);

    if (strcmp_P(root[F("action")], OPEN_VALVE) == 0){
      openValve();
      currentState = OPEN_VALVE_STATE;
    }
    else if (strcmp_P(root[F("action")], CLOSE_VALVE) == 0){
      closeValve();
      currentState = CLOSE_VALVE_STATE;
    }
    else if (strcmp_P(root[F("action")], RESET) == 0) {
      resetArudino();
    }

    timeToSleep = root[F("value")];
    elapsedTime = 0;
    retries = 0;

    buildImageName(buff, currentImage);
    if (takePicture(buff)){
      uploadFile(buff);
      currentImage++;
    }
  }
  else {
    info(F("POST error: "), false);
    info(result, true);
    retries ++;
    if (retries == MAX_RETRIES) resetArudino();
  }
}

/*
   the setup routine runs once when you press reset:
*/
void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial);

  // This setup is necessary for the logger and Arducam
  if (!SD.begin(SD_CS_PIN)) {
    info(F("Error starting SD"));
  }

  // This setup is necessary for the Arducam connection
  SPI.begin();
  Wire.begin();

  currentState = CLOSE_VALVE_STATE;

  info(F("Starting!"));

  //openValve();
  //closeValve();
}

/*
   the loop routine runs over and over again forever:
*/
void loop() {
  manageGarden();
  sleep();
}
