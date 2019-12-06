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

const char BEARER_PROFILE_GPRS[] PROGMEM = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n";
const char BEARER_PROFILE_APN[] PROGMEM = "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n";
const char QUERY_BEARER[] PROGMEM = "AT+SAPBR=2,1\r\n";
const char OPEN_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=1,1\r\n";
const char CLOSE_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=0,1\r\n";
const char HTTP_INIT[] PROGMEM = "AT+HTTPINIT\r\n";
const char HTTP_CID[] PROGMEM = "AT+HTTPPARA=\"CID\",1\r\n";
const char HTTP_PARA[] PROGMEM = "AT+HTTPPARA=\"URL\",\"%s\"\r\n";
const char HTTP_GET[] PROGMEM = "AT+HTTPACTION=0\r\n";
const char HTTP_POST[] PROGMEM = "AT+HTTPACTION=1\n";
const char HTTP_DATA[] PROGMEM = "AT+HTTPDATA=%d,%d\r\n";
const char HTTP_READ[] PROGMEM = "AT+HTTPREAD\r\n";
const char HTTP_CLOSE[] PROGMEM = "AT+HTTPTERM\r\n";
const char HTTP_CONTENT[] PROGMEM = "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n";
const char HTTPS_ENABLE[] PROGMEM = "AT+HTTPSSL=1\r\n";
const char HTTPS_DISABLE[] PROGMEM = "AT+HTTPSSL=0\r\n";
const char NORMAL_MODE[] PROGMEM = "AT+CFUN=1,1\r\n";
const char REGISTRATION_STATUS[] PROGMEM = "AT+CREG?\r\n";
const char SIGNAL_QUALITY[] PROGMEM = "AT+CSQ\r\n";
const char READ_VOLTAGE[] PROGMEM = "AT+CBC\r\n";
const char OK[] PROGMEM = "OK\r\n";
const char DOWNLOAD[] PROGMEM = "DOWNLOAD";
const char HTTP_2XX[] PROGMEM = ",2XX,";
const char HTTPS_PREFIX[] PROGMEM = "https://";
const char CONNECTED[] PROGMEM = "+CREG: 0,1";
const char ROAMING[] PROGMEM = "+CREG: 0,5";
const char BEARER_OPEN[] PROGMEM = "+SAPBR: 1,1";
const char OK_[] = "OK";

Result HTTP::configureBearer(const char *apn)
{
  Result result = SUCCESS;
  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  sendATTest();

  while ((sendCmdAndWaitForResp_P(REGISTRATION_STATUS, CONNECTED, 2000) != TRUE &&
          sendCmdAndWaitForResp_P(REGISTRATION_STATUS, ROAMING, 2000) != TRUE) &&
         attempts < MAX_ATTEMPTS)
  {
    sendCmdAndWaitForResp_P(READ_VOLTAGE, OK_, 1000);
    sendCmdAndWaitForResp_P(SIGNAL_QUALITY, OK_, 1000);
    attempts++;
    delay(1000 * attempts);
    if (attempts == MAX_ATTEMPTS)
    {
      attempts = 0;
      preInit();
    }
  }

  if (sendCmdAndWaitForResp_P(BEARER_PROFILE_GPRS, OK, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_GPRS;

  char httpApn[64];
  sprintf_P(httpApn, BEARER_PROFILE_APN, apn);
  if (sendCmdAndWaitForResp(httpApn, OK_, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_APN;

  return result;
}

Result HTTP::connect()
{
  Result result = SUCCESS;
  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  while (sendCmdAndWaitForResp_P(QUERY_BEARER, BEARER_OPEN, 2000) == FALSE && attempts < MAX_ATTEMPTS)
  {
    attempts++;
    if (sendCmdAndWaitForResp_P(OPEN_GPRS_CONTEXT, OK, 2000) == FALSE)
    {
      result = ERROR_OPEN_GPRS_CONTEXT;
    }
    else
    {
      result = SUCCESS;
    }
  }

  if (sendCmdAndWaitForResp_P(HTTP_INIT, OK, 2000) == FALSE)
    result = ERROR_HTTP_INIT;

  return result;
}

Result HTTP::disconnect()
{

  Result result = SUCCESS;

  if (sendCmdAndWaitForResp_P(CLOSE_GPRS_CONTEXT, OK, 2000) == FALSE)
    result = ERROR_CLOSE_GPRS_CONTEXT;

  if (sendCmdAndWaitForResp_P(HTTP_CLOSE, OK, 2000) == FALSE)
    result = ERROR_HTTP_CLOSE;

  return result;
}

Result HTTP::post(const char *uri, const char *body, char *response)
{

  Result result = setHTTPSession(uri);
  char buffer[32];
  char resp[16];

  unsigned int delayToDownload = 10000;
  sprintf_P(buffer, HTTP_DATA, strlen(body), delayToDownload);
  strcpy_P(resp, DOWNLOAD);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    result = ERROR_HTTP_DATA;
  }

  purgeSerial();
  sendCmd(body);

  if (sendCmdAndWaitForResp_P(HTTP_POST, HTTP_2XX, delayToDownload) == TRUE)
  {
    strcpy_P(buffer, HTTP_READ);
    sendCmd(buffer);
    readResponse(response);
    result = SUCCESS;
  }
  else
  {
    result = ERROR_HTTP_POST;
  }

  return result;
}

Result HTTP::get(const char *uri, char *response)
{

  Result result = setHTTPSession(uri);
  char buffer[16];
  char resp[16];
  
  if (sendCmdAndWaitForResp_P(HTTP_GET, HTTP_2XX, 2000) == TRUE)
  {
    strcpy_P(buffer, HTTP_READ);
    sendCmd(buffer);
    result = SUCCESS;
    readResponse(response);
  }
  else
  {
    result = ERROR_HTTP_GET;
  }

  return result;
}

unsigned int HTTP::readVoltage()
{
  char buffer[32];
  char voltage[8];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(voltage, sizeof(voltage));

  strcpy_P(buffer, READ_VOLTAGE);
  sendCmd(buffer);

  if (readBuffer(buffer, sizeof(buffer)) == TRUE)
  {
    parseATResponse(buffer, 4, 7, voltage);
  }
  return atoi(voltage);
}

unsigned int HTTP::readVoltagePercentage()
{
  char buffer[32];
  char voltage[8];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(voltage, sizeof(voltage));

  strcpy_P(buffer, READ_VOLTAGE);
  sendCmd(buffer);

  if (readBuffer(buffer, sizeof(buffer)) == TRUE)
  {
    parseATResponse(buffer, 2, 4, voltage);
  }
  return atoi(voltage);
}

unsigned int HTTP::readSignalStrength()
{
  char buffer[32];
  char signals[8];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(signals, sizeof(signals));

  strcpy_P(buffer, SIGNAL_QUALITY);
  sendCmd(buffer);
  if (readBuffer(buffer, sizeof(buffer)) == TRUE)
  {
    parseATResponse(buffer, 2, 2, signals);
  }
  return atoi(signals);
}

Result HTTP::setHTTPSession(const char *uri)
{
  Result result;
  char buffer[128];

  if (sendCmdAndWaitForResp_P(HTTP_CID, OK, 2000) == FALSE)
    result = ERROR_HTTP_CID;

  sprintf_P(buffer, HTTP_PARA, uri);
  if (sendCmdAndWaitForResp(buffer, OK_, 2000) == FALSE)
    result = ERROR_HTTP_PARA;

  // TODO check this
  bool https = strncmp_P(HTTPS_PREFIX, uri, strlen_P(HTTPS_PREFIX)) == 0;
  if (sendCmdAndWaitForResp_P(https ? HTTPS_ENABLE : HTTPS_DISABLE, OK, 2000) == FALSE)
  {
    result = https ? ERROR_HTTPS_ENABLE : ERROR_HTTPS_DISABLE;
  }

  if (sendCmdAndWaitForResp_P(HTTP_CONTENT, OK, 2000) == FALSE)
    result = ERROR_HTTP_CONTENT;

  return result;
}

void HTTP::readResponse(char *response)
{
  char buffer[128];
  cleanBuffer(buffer, sizeof(buffer));
  cleanBuffer(response, sizeof(response));

  if (readBuffer(buffer, sizeof(buffer)) == TRUE)
  {
    parseJSONResponse(buffer, sizeof(buffer), response);
  }
}
