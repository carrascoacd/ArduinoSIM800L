/*
 * Http.cpp
 * A HTTP library for the SIM800L board
 *
 * Copyright 2018 Antonio Carrasco
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Http.h"
#include "Parser.h"

#define BEARER_PROFILE_GPRS "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n"
#define BEARER_PROFILE_APN "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n"
#define QUERY_BEARER "AT+SAPBR=2,1\r\n"
#define OPEN_GPRS_CONTEXT "AT+SAPBR=1,1\r\n"
#define CLOSE_GPRS_CONTEXT "AT+SAPBR=0,1\r\n"
#define HTTP_INIT "AT+HTTPINIT\r\n"
#define HTTP_CID "AT+HTTPPARA=\"CID\",1\r\n"
#define HTTP_PARA "AT+HTTPPARA=\"URL\",\"%s\"\r\n"
#define HTTP_GET "AT+HTTPACTION=0\r\n"
#define HTTP_POST "AT+HTTPACTION=1\n"
#define HTTP_DATA "AT+HTTPDATA=%d,%d\r\n"
#define HTTP_READ "AT+HTTPREAD\r\n"
#define HTTP_CLOSE "AT+HTTPTERM\r\n"
#define HTTP_CONTENT "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"
#define HTTPS_ENABLE "AT+HTTPSSL=1\r\n"
#define HTTPS_DISABLE "AT+HTTPSSL=0\r\n"
#define NORMAL_MODE "AT+CFUN=1,1\r\n"
#define REGISTRATION_STATUS "AT+CREG?\r\n"
#define SIGNAL_QUALITY "AT+CSQ\r\n"
#define READ_VOLTAGE "AT+CBC\r\n"
#define SLEEP_MODE_2 "AT+CSCLK=2\r\n"
#define SLEEP_MODE_1 "AT+CSCLK=1\r\n"
#define SLEEP_MODE_0 "AT+CSCLK=0\r\n"
#define OK "OK\r\n"
#define DOWNLOAD "DOWNLOAD"
#define HTTP_2XX ",2XX,"
#define HTTPS_PREFIX "https://"
#define CONNECTED "+CREG: 0,1"
#define ROAMING "+CREG: 0,5"
#define BEARER_OPEN "+SAPBR: 1,1"

Result HTTP::configureBearer(const char *apn){

  Result result = SUCCESS;

  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  sendATTest();

  while ((sendCmdAndWaitForResp(REGISTRATION_STATUS, CONNECTED, 2000) != TRUE &&
          sendCmdAndWaitForResp(REGISTRATION_STATUS, ROAMING, 2000) != TRUE)
            && attempts < MAX_ATTEMPTS){
    sendCmdAndWaitForResp(READ_VOLTAGE, OK, 1000);
    sendCmdAndWaitForResp(SIGNAL_QUALITY, OK, 1000);
    attempts ++;
    delay(1000 * attempts);
    if (attempts == MAX_ATTEMPTS) {
      attempts = 0;
      preInit();
    }
  }

  if (sendCmdAndWaitForResp(BEARER_PROFILE_GPRS, OK, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_GPRS;

  char httpApn[64];
  sprintf(httpApn, BEARER_PROFILE_APN, apn);
  if (sendCmdAndWaitForResp(httpApn, OK, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_APN;

  return result;
}

Result HTTP::connect() {

  Result result = SUCCESS;
  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  while (sendCmdAndWaitForResp(QUERY_BEARER, BEARER_OPEN, 2000) == FALSE && attempts < MAX_ATTEMPTS){
    attempts ++;
    if (sendCmdAndWaitForResp(OPEN_GPRS_CONTEXT, OK, 2000) == FALSE){
      result = ERROR_OPEN_GPRS_CONTEXT;
    }
    else {
      result = SUCCESS;
    }
  }

  if (sendCmdAndWaitForResp(HTTP_INIT, OK, 2000) == FALSE)
    result = ERROR_HTTP_INIT;

  return result;
}

Result HTTP::disconnect() {

  Result result = SUCCESS;

  if (sendCmdAndWaitForResp(CLOSE_GPRS_CONTEXT, OK, 2000) == FALSE)
    result = ERROR_CLOSE_GPRS_CONTEXT;
  if (sendCmdAndWaitForResp(HTTP_CLOSE, OK, 2000) == FALSE)
    result = ERROR_HTTP_CLOSE;

  return result;
}

Result HTTP::post(const char *uri, const char *body, char *response) {

  Result result = setHTTPSession(uri);

  char httpData[32];
  unsigned int delayToDownload = 10000;
  sprintf(httpData, HTTP_DATA, strlen(body), 10000);
  if (sendCmdAndWaitForResp(httpData, DOWNLOAD, 2000) == FALSE){
    result = ERROR_HTTP_DATA;
  }

  purgeSerial();
  sendCmd(body);

  if (sendCmdAndWaitForResp(HTTP_POST, HTTP_2XX, delayToDownload) == TRUE) {
    sendCmd(HTTP_READ);
    readResponse(response);
    result = SUCCESS;
  }
  else {
    result = ERROR_HTTP_POST;
  }

  return result;
}

Result HTTP::get(const char *uri, char *response) {

  Result result = setHTTPSession(uri);

  if (sendCmdAndWaitForResp(HTTP_GET, HTTP_2XX, 2000) == TRUE) {
    sendCmd(HTTP_READ);
    result = SUCCESS;
    readResponse(response);
  }
  else {
    result = ERROR_HTTP_GET;
  }

  return result;
}

void HTTP::sleep(bool force){
  if (force){
    sendCmdAndWaitForResp(SLEEP_MODE_1, OK, 2000);
  }
  else {
    sendCmdAndWaitForResp(SLEEP_MODE_2, OK, 2000);
  }
}

void HTTP::wakeUp(){
  preInit();
  sendCmdAndWaitForResp(HTTP_GET, SLEEP_MODE_0, 2000);
}

unsigned int HTTP::readVoltage(){
  char buffer[32];
  char voltage[8];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(voltage, sizeof(voltage));

  sendCmd(READ_VOLTAGE);

  if (readBuffer(buffer, sizeof(buffer)) == TRUE){
    parseATResponse(buffer, 4, 7, voltage);
  }
  return atoi(voltage);
}

unsigned int HTTP::readVoltagePercentage(){
  char buffer[32];
  char voltage[8];  
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(voltage, sizeof(voltage));

  sendCmd(READ_VOLTAGE);

  if (readBuffer(buffer, sizeof(buffer)) == TRUE){
    parseATResponse(buffer, 2, 4, voltage);
  }
  return atoi(voltage);  
}

unsigned int HTTP::readSignalStrength(){
  char buffer[32];
  char signals[8];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(signals, sizeof(signals));

  sendCmd(SIGNAL_QUALITY);
  if (readBuffer(buffer, sizeof(buffer)) == TRUE){
    parseATResponse(buffer, 2, 2, signals);
  }
  return atoi(signals);
}

Result HTTP::setHTTPSession(const char *uri){
  Result result;
  if (sendCmdAndWaitForResp(HTTP_CID, OK, 2000) == FALSE)
    result = ERROR_HTTP_CID;

  char httpPara[128];
  sprintf(httpPara, HTTP_PARA, uri);

  if (sendCmdAndWaitForResp(httpPara, OK, 2000) == FALSE)
    result = ERROR_HTTP_PARA;

  bool https = strncmp(HTTPS_PREFIX, uri, strlen(HTTPS_PREFIX)) == 0;
  if (sendCmdAndWaitForResp(https ? HTTPS_ENABLE : HTTPS_DISABLE, OK, 2000) == FALSE) {
    result = https ? ERROR_HTTPS_ENABLE : ERROR_HTTPS_DISABLE;
  }

  if (sendCmdAndWaitForResp(HTTP_CONTENT, OK, 2000) == FALSE)
    result = ERROR_HTTP_CONTENT;

  return result;
}

void HTTP::readResponse(char *response){
  char buffer[128];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(response, sizeof(response));

  if (readBuffer(buffer, sizeof(buffer)) == TRUE){
    parseJSONResponse(buffer, sizeof(buffer), response);
  }
}
