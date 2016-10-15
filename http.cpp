/*
 * Http.cpp
 * A HTTP library for the SIM800L board
 *
 * Copyright 2016 Antonio Carrasco
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
#include <string.h>

const char *BEARER_PROFILE_GPRS = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n";
const char *BEARER_PROFILE_APN = "AT+SAPBR=3,1,\"APN\",\"%s\"\n";
const char *OPEN_GPRS_CONTEXT = "AT+SAPBR=1,1\n";
const char *CLOSE_GPRS_CONTEXT = "AT+SAPBR=0,1\n";
const char *HTTP_INIT = "AT+HTTPINIT\n";
const char *HTTP_CID = "AT+HTTPPARA=\"CID\",1\n";
const char *HTTP_PARA = "AT+HTTPPARA=\"URL\",\"%s\"\n";
const char *HTTP_GET = "AT+HTTPACTION=0\n";
const char *HTTP_POST = "AT+HTTPACTION=1\n";
const char *HTTP_DATA = "AT+HTTPDATA=%d,%d\n";
const char *HTTP_READ = "AT+HTTPREAD\n";
const char *HTTP_CLOSE = "AT+HTTPTERM\n";
const char *HTTP_CONTENT = "AT+HTTPPARA=\"CONTENT\",\"application/json\"\n";
const char *OK = "OK";
const char *DOWNLOAD = "DOWNLOAD";
const char *HTTP_200 = ",200,";

Result HTTP::configureBearer(const char *apn){

  Result result = SUCCESS;

  if (preInit() == FALSE){
    result = ERROR_INITIALIZATION;
  }
  else{
    if (sendCmdAndWaitForResp(BEARER_PROFILE_GPRS, OK, 2000) == FALSE)
      result = ERROR_BEARER_PROFILE_GPRS;
    
    char httpApn[128];
    sprintf(httpApn, BEARER_PROFILE_APN, apn);
    if (sendCmdAndWaitForResp(httpApn, OK, 2000) == FALSE)
      result = ERROR_BEARER_PROFILE_APN;
  }
  return result;
}

Result HTTP::connect() {

  Result result = SUCCESS;
  if (sendCmdAndWaitForResp(OPEN_GPRS_CONTEXT, OK, 2000) == FALSE)
    result = ERROR_OPEN_GPRS_CONTEXT;
  if (sendCmdAndWaitForResp(HTTP_INIT, OK, 2000) == FALSE)
    result = ERROR_HTTP_INIT;

  return result;
}

Result HTTP::disconnect() {

  Result result = SUCCESS;
  if (sendCmdAndWaitForResp(HTTP_CLOSE, OK, 2000) == FALSE)
    result = ERROR_HTTP_CLOSE;
  if (sendCmdAndWaitForResp(CLOSE_GPRS_CONTEXT, OK, 2000) == FALSE)
    result = ERROR_CLOSE_GPRS_CONTEXT;

  return result;
}

Result HTTP::post(const char *uri, const char *body, char *response) {

  Result result = setHTTPSession(uri);

  if (sendCmdAndWaitForResp(HTTP_CONTENT, OK, 5000) == FALSE)
    result = ERROR_HTTP_CONTENT;

  char httpData[128];
  int delayToDownload = 5000;
  sprintf(httpData, HTTP_DATA, strlen(body), delayToDownload);
  if (sendCmdAndWaitForResp(httpData, DOWNLOAD, 5000) == FALSE){
    result = ERROR_HTTP_DATA;
  }

  sendCmd(body);
  delay(delayToDownload);

  if (sendCmdAndWaitForResp(HTTP_POST, HTTP_200, 5000) == TRUE) {
    sendCmd(HTTP_READ);
    result = SUCCESS;
    readResponse(response);
  }
  else {
    result = ERROR_HTTP_POST;
  }
}

Result HTTP::get(const char *uri, char *response) {

  Result result = setHTTPSession(uri);
  
  if (sendCmdAndWaitForResp(HTTP_GET, HTTP_200, 5000) == TRUE) {
    sendCmd(HTTP_READ);
    result = SUCCESS;
    readResponse(response);
  }
  else {
    result = ERROR_HTTP_GET;
  }

  return result;
}

Result HTTP::setHTTPSession(const char *uri){

  Result result;
  if (sendCmdAndWaitForResp(HTTP_CID, OK, 2000) == FALSE)
    result = ERROR_HTTP_CID;

  char httpPara[512];
  sprintf(httpPara, HTTP_PARA, uri);

  if (sendCmdAndWaitForResp(httpPara, OK, 2000) == FALSE)
    result = ERROR_HTTP_PARA;

  return result;
}

void HTTP::readResponse(char *response){
  
  char buffer[256];  
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(response, sizeof(response));

  if (readBuffer(buffer, sizeof(buffer)) == TRUE){
    parseJSONResponse(buffer, sizeof(buffer), response);
  }
}

void HTTP::parseJSONResponse(const char *buffer, unsigned int bufferSize, char *response){

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