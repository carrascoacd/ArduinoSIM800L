#include "http.h"

const char *AT_HTTPPARA_URL = "AT+HTTPPARA=\"URL\",\"%s\"\n";


Result HTTP::configureBearer(const char *apn){

  Result result = OK;
  if (sendCmdAndWaitForResp("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n", "OK", 2) == FALSE)
    result = ERROR_BEARER_PROFILE_GPRS;
  
  char httpApn[128];
  sprintf(httpApn, "AT+SAPBR=3,1,\"APN\",\"%s\"\n", apn);
  if (sendCmdAndWaitForResp(httpApn, "OK", 2) == FALSE)
    result = ERROR_BEARER_PROFILE_APN;
  
  return result;
}


Result HTTP::connect() {

  Result result = OK;
  if (sendCmdAndWaitForResp("AT+SAPBR=1,1\n", "OK", 2) == FALSE)
    result = ERROR_OPEN_GPRS_CONTEXT;
  if (sendCmdAndWaitForResp("AT+HTTPINIT\n", "OK", 2) == FALSE)
    result = ERROR_HTTP_INIT;

  return result;
}

Result HTTP::disconnect() {

  Result result = OK;
  if (sendCmdAndWaitForResp("AT+HTTPTERM\n", "OK", 2) == FALSE)
    result = ERROR_HTTP_CLOSE;
  if (sendCmdAndWaitForResp("AT+SAPBR=0,1\n", "OK", 2) == FALSE)
    result = ERROR_CLOSE_GPRS_CONTEXT;

  return result;
}

Result HTTP::get(const char *url, char *response) {

  Result result = OK;

  if (sendCmdAndWaitForResp("AT+HTTPPARA=\"CID\",1\n", "OK", 2) == FALSE)
    result = ERROR_HTTP_CID;

  char httpPara[128];
  sprintf(httpPara, AT_HTTPPARA_URL, url);
  if (sendCmdAndWaitForResp(httpPara, "OK,", 2) == FALSE)
    result = ERROR_HTTP_PARA;
  
  if (sendCmdAndWaitForResp("AT+HTTPACTION=0\n", ",200,", 5) == TRUE) {
    sendCmd("AT+HTTPREAD\n");
    result = OK;

    char buffer[256];
    cleanBuffer(buffer, sizeof(buffer));
    if (readBuffer(buffer, sizeof(buffer)) == TRUE){
      parseJSONResponse(buffer, response, sizeof(buffer));
    }

  }
  else {
    result = ERROR_HTTP_GET;
  }

  return result;
}

void HTTP::parseJSONResponse(const char *buffer, char *response, unsigned int bufferSize){

  cleanBuffer(response, sizeof(response));

  int start_index = 0;
  int i = 0;
  while (i < bufferSize - 1 && start_index == 0) {
    char c = buffer[i];
    if ('{' == c){
      start_index = i;
    }
    ++i;
  }

  int end_index = 0;
  int j = bufferSize - 1;
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