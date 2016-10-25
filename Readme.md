# SIM800L
A smart library based on Seeeduino that implements the AT HTTP commands to perform GET and POST requests to a JSON API.
In order to perform a request, the library follows these steps:

##### Configure Bearer:

  - AT+CREG? -> try until 0,1 (connected to the network)
  - AT+SAPBR=3,1,"Contype","GPRS" -> wait for OK
  - AT+SAPBR=3,1,"APN","movistar.es" -> wait for OK
  - AT+SAPBR=1,1 -> wait for OK

##### HTTP GET:

  - AT+HTTPINIT -> wait for OK
  - AT+HTTPPARA="CID",1  -> wait for OK
  - AT+HTTPPARA="URL","smartgarden.herokuapp.com"  -> wait for OK 
  - AT+HTTPACTION=0 -> wait for ,200,
  - AT+HTTPREAD -> read buffer and parse it
  - AT+HTTPTERM -> wait for OK
  - AT+SAPBR=0,1

##### HTTP POST:
  - AT+HTTPINIT -> wait for OK
  - AT+HTTPPARA="CID",1  -> wait for OK
  - AT+HTTPPARA="URL","smartgarden.herokuapp.com"  -> wait for OK 
  
  For example, if we have this body: {"location_id": 238, "fill_percent": 90}

  - AT+HTTPPARA="CONTENT","application/json"
  - AT+HTTPDATA=strlen(body),10000 -> wait for DOWNLOAD, then write the body and wait 10000
  - AT+HTTPACTION=1 -> wait for ,200,
  - AT+HTTPREAD -> read buffer and parse it
  - AT+HTTPTERM -> wait for OK
  - AT+SAPBR=0,1

