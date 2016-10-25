#include <ArduinoJson.h>
#include <Http.h>

HTTP http;
Result result;

void print(const char *message, int result = -1){
  if (result != -1){
    char buffer[100];
    sprintf(buffer, message, result);
    Serial.println(buffer);
  }
  else {
    Serial.println(message);
  }
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  while(!Serial);

  //http.preInit();
}

// the loop routine runs over and over again forever:
void loop() {
  
  print("Cofigure bearer: %d", http.configureBearer("movistar.es"));
  print("HTTP connect: %d", http.connect());
    
  char response[256];
  
  result = http.get("smartgarden.herokuapp.com", response);
  print("HTTP GET: %d", result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    Serial.print("date: ");
    Serial.println((const char *)root["date"]);
    Serial.print("name: ");
    Serial.println((const char *)root["name"]);
  }

  result = http.post("smartgarden.herokuapp.com", "{\"name\":\"Antonio\"}", response);
  print("HTTP POST: %d", result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    Serial.print("date: ");
    Serial.println((const char *)root["date"]);
    Serial.print("name: ");
    Serial.println((const char *)root["name"]);
  }

  print("HTTP disconnect: %d", http.disconnect());
 
}
