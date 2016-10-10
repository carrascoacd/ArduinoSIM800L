#include "http.h"

const char *AT_HTTPPARA_URL = "AT+HTTPPARA=\"URL\",\"%s\"\n";

void HTTP::configureBearer(const char *apn){

  if (sendCmdAndWaitForResp("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n", "OK", 2) == 0)
    Serial.println("1");
  char httpApn[128];
  sprintf(httpApn, "AT+SAPBR=3,1,\"APN\",\"%s\"\n", apn);
  if (sendCmdAndWaitForResp(httpApn, "OK", 2) == 0)
    Serial.println("2");  
}


void HTTP::connect() {

  if (sendCmdAndWaitForResp("AT+SAPBR=1,1\n", "OK", 2) == 0)
    Serial.println("3");
  if (sendCmdAndWaitForResp("AT+HTTPINIT\n", "OK", 2) == 0)
    Serial.println("4");

}

void HTTP::disconnect() {

  if (sendCmdAndWaitForResp("AT+HTTPTERM\n", "OK", 2) == 0)
    Serial.println("7");
  if (sendCmdAndWaitForResp("AT+SAPBR=0,1\n", "OK", 2) == 0)
    Serial.println("8");
}

void HTTP::get(const char *url, char *response) {

  if (sendCmdAndWaitForResp("AT+HTTPPARA=\"CID\",1\n", "OK", 2) == 0)
    Serial.println("5");

  char httpPara[128];
  sprintf(httpPara, AT_HTTPPARA_URL, url);
  if (sendCmdAndWaitForResp(httpPara, "OK,", 2) == 0)
    Serial.println("6");
  
  if (sendCmdAndWaitForResp("AT+HTTPACTION=0\n", ",200,", 5) == 0) {
    sendCmd("AT+HTTPREAD\n");

    char buffer[256];
    if (readBuffer(buffer, 256) == 0){
      
      int start_index = 0;
      int i = 0;
      while (i < (sizeof(buffer) - 1) && start_index == 0) {
        char c = buffer[i];
        if ('{' == c){
          start_index = i;
        }
        ++i;
      }

      int end_index = 0;
      int j = sizeof(buffer) - 1;
      while (j >= 0 && end_index == 0) {
        char c = buffer[j];
        if ('}' == c) {
          end_index = j;
        }
        --j;
      }

      for(int k = 0; k < (end_index - start_index) + 2; ++k){
        response[k] = buffer[start_index + k];
        response[k + 1] = '\0';
      }
      
    }
  }

}


// const String AT = "AT+HTTPREAD";
// const String DELIMITER = "\n";
// const String OK = "OK";
// const String ERROR = "OK";
// const String RIGHT_RESPONSE = ",200,";

// Response SIM800L::httpRead(){

//   Response response = {"", true};
//   if (_responseBuffer.indexOf(OK) != -1){
//     int from = _responseBuffer.lastIndexOf(AT) + 1;
//     int to = _responseBuffer.lastIndexOf(DELIMITER);
//     response.body = _responseBuffer.substring(from, to);
//     response.error = false;
//   }
//   return response;
// }

// bool SIM800L::hasResponse() {
//   _responseBuffer.indexOf(RIGHT_RESPONSE) != -1;
// }

// void SIM800L::purgeSerial()
// {
//   while (_simSerial->available()) _simSerial->read();
// }


