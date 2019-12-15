#include <ArduinoJson.h>
#include <Http.h>

#define RST_PIN 8
#define RX_PIN 10
#define TX_PIN 9

const char BEARER[] PROGMEM = "your.mobile.service.provider.apn";

// the setup routine runs once when you press reset:
void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Starting!");
}

// the loop routine runs over and over again forever:
void loop()
{
  HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);

  char response[32];
  char body[90];
  Result result;

  // Notice the bearer must be a pointer to the PROGMEM
  result = http.connect(BEARER);
  Serial.print(F("HTTP connect: "));
  Serial.println(result);

  sprintf(body, "{\"title\": \"%s\", \"body\": \"%s\", \"user_id\": \"%d\"}", "Arduino", "Test", 1);
  result = http.post("https://your.api", body, response);
  Serial.print(F("HTTP POST: "));
  Serial.println(result);
  if (result == SUCCESS)
  {
    Serial.println(response);
    StaticJsonBuffer<64> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(response);

    const char *id = root[F("id")];
    Serial.print(F("ID: "));
    Serial.println(id);
  }

  result = http.get("https://your.api", response);
  Serial.print(F("HTTP GET: "));
  Serial.println(result);
  if (result == SUCCESS)
  {
    Serial.println(response);
    StaticJsonBuffer<64> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(response);

    const char *id = root[F("id")];
    Serial.print(F("ID: "));
    Serial.println(id);
  }

  Serial.print(F("HTTP disconnect: "));
  Serial.print(http.disconnect());
}
