
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=58Y36H29935PW&item_name=Support+me+to+continue+developing+features&currency_code=EUR&source=url)

# Arduino SIM800L library
A smart HTTP & FTP library based on Seeeduino that implements the AT HTTP commands to perform GET and POST requests to a JSON API as well as FTP uploads.

## Support
* Your board has to support the standard SoftwareSerial library. It doesn't work with HardwareSerial based boards for the moment.
* The API response format has to be a valid JSON.
* The library has been tested against Arduino Uno and Arduino Nano.

## Instalation
Download the ZIP library and then import it: Sketch -> Include Library -> Add .ZIP Library ...

## Quick start!

How to do a GET request! :+1:

``` c++
unsigned int RX_PIN = 7;
unsigned int TX_PIN = 8;
unsigned int RST_PIN = 12;

const char BEARER[] PROGMEM = "gprs-service.com";

HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
http.connect(BEARER);

char response[256];
Result result = http.get("your.api.com", response);

Serial.println(response);

http.disconnect();

```

How to do a POST request! :+1:

``` c++
unsigned int RX_PIN = 7;
unsigned int TX_PIN = 8;
unsigned int RST_PIN = 12;
const char BEARER[] PROGMEM = "gprs-service.com";

HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);
http.connect(BEARER);

char response[256];
Result result = http.post("your.api.com", "{\"date\":\"12345678\"}", response);

Serial.println(response);

http.disconnect();
```

How to do a FTP upload! :+1:

``` c++
unsigned int RX_PIN = 7;
unsigned int TX_PIN = 8;
unsigned int RST_PIN = 12;
const char BEARER[] PROGMEM = "gprs-service.com";
const char FTP_SERVER[] PROGMEM = "ftp.server";
const char FTP_USER[] PROGMEM = "user";
const char FTP_PASS[] PROGMEM = "pass";

FTP ftp(9600, RX_PIN, TX_PIN, RST_PIN);
ftp.putBegin(BEARER, "example.txt", FTP_SERVER, FTP_USER, FTP_PASS);
ftp.putWrite("hello!", sizeof("hello!"));
ftp.putEnd();

```

I suggest the [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) library for parsing the JSON response, then you can play with the values easily.

## HTTP. How it works?
In order to perform a request, the library follows these steps:

##### Configure Bearer:

  - AT+CREG? -> try until 0,1 (connected to the network)
  - AT+SAPBR=3,1,"Contype","GPRS" -> wait for OK
  - AT+SAPBR=3,1,"APN","movistar.es" -> wait for OK
  - AT+SAPBR=1,1 -> wait for OK

##### HTTP GET:

  - AT+HTTPINIT -> wait for OK
  - AT+HTTPPARA="CID",1 -> wait for OK
  - AT+HTTPPARA="URL","your.api.com"-> wait for OK
  - AT+HTTPSSL=0 -> wait for OK (1 when URL starts with "https://")
  - AT+HTTPACTION=0 -> wait for 200
  - AT+HTTPREAD -> read buffer and parse it
  - AT+HTTPTERM -> wait for OK
  - AT+SAPBR=0,1

##### HTTP POST:
  - AT+HTTPINIT -> wait for OK
  - AT+HTTPPARA="CID",1 -> wait for OK
  - AT+HTTPPARA="URL","your.api.com" -> wait for OK

  For example, if we have this body: {"location_id": 238, "fill_percent": 90}

  - AT+HTTPPARA="CONTENT","application/json"
  - AT+HTTPDATA=strlen(body),10000 -> wait for DOWNLOAD, then write the body and wait 10000
  - AT+HTTPSSL=0 -> wait for OK (1 when URL starts with "https://")
  - AT+HTTPACTION=1 -> wait for ,200,
  - AT+HTTPREAD -> read buffer and parse it
  - AT+HTTPTERM -> wait for OK
  - AT+SAPBR=0,1

## Future improvements

- Support of HardwareSerial.
- Support of more content types, not only JSON (application/json).


## Development
- Lint: `cppcheck --enable=all .`