/*
 * Http.cpp
 * A HTTP library for the SIM800L board
 *
 * Copyright 2019 Antonio Carrasco
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
#include "GPRS.h"

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
const char SIGNAL_QUALITY[] PROGMEM = "AT+CSQ\r\n";
const char READ_VOLTAGE[] PROGMEM = "AT+CBC\r\n";

const char AT_OK[] PROGMEM = "OK";
const char AT_OK_[] = "OK";
const char DOWNLOAD[] PROGMEM = "DOWNLOAD";
const char HTTP_2XX[] PROGMEM = ",2XX,";
const char HTTPS_PREFIX[] PROGMEM = "https://";


Result HTTP::connect(const char *apn)
{
  Result result = openGPRSContext(this, apn);

  if (sendCmdAndWaitForResp_P(HTTP_INIT, AT_OK, 2000) == FALSE)
    result = ERROR_HTTP_INIT;

  return result;
}

Result HTTP::disconnect()
{
  Result result = closeGPRSContext(this);

  if (sendCmdAndWaitForResp_P(HTTP_CLOSE, AT_OK, 2000) == FALSE)
    result = ERROR_HTTP_CLOSE;

  return result;
}

Result HTTP::post(const char *uri, const char *body, char *response)
{
  Result result;
  setHTTPSession(uri);
  
  char buffer[32];
  char resp[16];

  unsigned int delayToDownload = 10000;
  sprintf_P(buffer, HTTP_DATA, strlen(body), delayToDownload);
  strcpy_P(resp, DOWNLOAD);
  
  sendCmdAndWaitForResp(buffer, resp, 2000);

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
  Result result;
  setHTTPSession(uri);

  if (sendCmdAndWaitForResp_P(HTTP_GET, HTTP_2XX, 2000) == TRUE)
  {
    char buffer[16];
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
  sendCmd(buffer, 500);

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
  Result result = SUCCESS;
  char buffer[128];

  if (sendCmdAndWaitForResp_P(HTTP_CID, AT_OK, 2000) == FALSE)
    result = ERROR_HTTP_CID;

  sprintf_P(buffer, HTTP_PARA, uri);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    result = ERROR_HTTP_PARA;

  bool https = strncmp_P(uri, HTTPS_PREFIX, strlen_P(HTTPS_PREFIX)) == 0;
  if (sendCmdAndWaitForResp_P(https ? HTTPS_ENABLE : HTTPS_DISABLE, AT_OK, 2000) == FALSE)
  {
    result = https ? ERROR_HTTPS_ENABLE : ERROR_HTTPS_DISABLE;
  }

  if (sendCmdAndWaitForResp_P(HTTP_CONTENT, AT_OK, 2000) == FALSE)
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
