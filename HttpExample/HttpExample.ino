#include <ArduinoJson.h>
#include <Http.h>

HTTP http;
Result result;

unsigned long lastRunTime = 0;
unsigned long waitForRunTime = 0;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Starting!");
}

// the loop routine runs over and over again forever:
void loop() {
  if (shouldTrackTimeEntry()) trackTimeEntry();
}

// functions
void print(const char *message, long result = -1){
  if (result != -1){
    char buffer[100];
    sprintf(buffer, message, result);
    Serial.println(buffer);
  }
  else {
    Serial.println(message);
  }
}

bool shouldTrackTimeEntry(){
  unsigned long elapsedTime = millis() - lastRunTime;
  print("Elapsed time: %ld", elapsedTime);
  return elapsedTime >= waitForRunTime;
}

void trackTimeEntry(){
  
  print("Cofigure bearer: %d", http.configureBearer("movistar.es"));
  print("HTTP connect: %d", http.connect());
    
  char response[256];
  
  result = http.get("smartgarden.herokuapp.com/api", response);
  print("HTTP GET: %d", result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    lastRunTime = millis();
    waitForRunTime = root["waitForRunTime"];
    
    print("Last run time: %ld", lastRunTime);
    print("Next post in: %ld", waitForRunTime);
  }
  
  result = http.post("smartgarden.herokuapp.com/api", "{\"name\":\"Antonio\"}", response);
  print("HTTP POST: %d", result);
  if (result == SUCCESS) {
    Serial.println(response);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    Serial.print("name: ");
    Serial.println((const char *)root["name"]);
  }

  print("HTTP disconnect: %d", http.disconnect());
}
