/*
 * Ftp.cpp
 * Ftp library for the SIM800L board
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

#include "Ftp.h"
#include <string.h>

const char AT_FTPCID[] PROGMEM = "AT+FTPCID=1\r\n";
const char AT_FTPSERV[] PROGMEM = "AT+FTPSERV=\"%s\"\r\n";
const char AT_FTPUN[] PROGMEM = "AT+FTPUN=\"%s\"\r\n";
const char AT_FTPPW[] PROGMEM = "AT+FTPPW=\"%s\"\r\n";
const char AT_FTPPUTNAME[] PROGMEM = "AT+FTPPUTNAME=\"%s\"\r\n";
const char AT_FTPPUTPATH[] PROGMEM = "AT+FTPPUTPATH=\"%s\"\r\n";
const char AT_FTPPUT1[] PROGMEM = "AT+FTPPUT=1\r\n";
const char AT_FTPPUT2[] PROGMEM = "AT+FTPPUT=2,%d\r\n";
const char AT_FTPPUT20[] PROGMEM = "AT+FTPPUT=2,0\r\n";
const char BEARER_PROFILE_GPRS[] PROGMEM = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n";
const char BEARER_PROFILE_APN[] PROGMEM = "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n";
const char QUERY_BEARER[] PROGMEM = "AT+SAPBR=2,1\r\n";
const char OPEN_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=1,1\r\n";
const char CLOSE_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=0,1\r\n";
const char REGISTRATION_STATUS[] PROGMEM = "AT+CREG?\r\n";
const char SLEEP_MODE_2[] PROGMEM = "AT+CSCLK=2\r\n";

const char OK[] PROGMEM = "OK";
const char AT_FTPPUT1_RESP[] PROGMEM = "1,1";
const char AT_FTPPUT2_RESP[] PROGMEM = "+FTPPUT: 2";
const char AT_FTPPUT20_RESP[] PROGMEM = "1,0";
const char CONNECTED[] PROGMEM = "+CREG: 0,1";
const char ROAMING[] PROGMEM = "+CREG: 0,5";
const char BEARER_OPEN[] PROGMEM = "+SAPBR: 1,1";

Result FTP::configureBearer(const char *apn)
{
  Result result = SUCCESS;
  char buffer[64];
  char resp1[12];
  char resp2[12];
  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  sendATTest();

  strcpy_P(buffer, REGISTRATION_STATUS);
  strcpy_P(resp1, CONNECTED);
  strcpy_P(resp2, ROAMING);
  while ((sendCmdAndWaitForResp(buffer, resp1, 2000) != TRUE &&
          sendCmdAndWaitForResp(buffer, resp2, 2000) != TRUE) &&
         attempts < MAX_ATTEMPTS)
  {
    attempts++;
    delay(1000 * attempts);
    if (attempts == MAX_ATTEMPTS)
    {
      attempts = 0;
      preInit();
    }
  }
  attempts = 0;

  strcpy_P(buffer, BEARER_PROFILE_GPRS);
  strcpy_P(resp1, OK);
  if (sendCmdAndWaitForResp(buffer, resp1, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_GPRS;

  sprintf_P(buffer, BEARER_PROFILE_APN, apn);
  strcpy_P(resp1, OK);
  if (sendCmdAndWaitForResp(buffer, resp1, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_APN;

  strcpy_P(buffer, QUERY_BEARER);
  strcpy_P(resp1, BEARER_OPEN);
  while (sendCmdAndWaitForResp(buffer, resp1, 2000) == FALSE && attempts < MAX_ATTEMPTS)
  {
    attempts++;
    if (attempts == MAX_ATTEMPTS)
    {
      result = ERROR_OPEN_GPRS_CONTEXT;
    }
  }

  attempts = 0;

  strcpy_P(buffer, OPEN_GPRS_CONTEXT);
  strcpy_P(resp1, OK);
  while (sendCmdAndWaitForResp(buffer, resp1, 2000) == FALSE && attempts < MAX_ATTEMPTS)
  {
    attempts++;
    if (attempts == MAX_ATTEMPTS)
    {
      result = ERROR_QUERY_GPRS_CONTEXT;
    }
  }

  return result;
}

Result FTP::putBegin(const char *fileName,
                     const char *server,
                     const char *usr,
                     const char *pass,
                     const char *path)
{
  Result result = SUCCESS;

  char buffer[64];
  char resp[12];

  delay(10000);
  strcpy_P(buffer, AT_FTPCID);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPCID;
  }

  sprintf_P(buffer, AT_FTPSERV, server);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPSERV;
  }

  sprintf_P(buffer, AT_FTPUN, usr);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPUN;
  }

  sprintf_P(buffer, AT_FTPPW, pass);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPPW;
  }

  sprintf_P(buffer, AT_FTPPUTNAME, fileName);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPPUTNAME;
  }

  sprintf_P(buffer, AT_FTPPUTPATH, path);
  strcpy_P(resp, OK);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPPUTPATH;
  }

  strcpy_P(buffer, AT_FTPPUT1);
  strcpy_P(resp, AT_FTPPUT1_RESP);
  if (sendCmdAndWaitForResp(buffer, resp, 10000) == FALSE)
  {
    return ERROR_FTPPUT1;
  }

  return result;
}

Result FTP::putWrite(const char *data, unsigned int size)
{
  Result result = SUCCESS;

  unsigned int attempts = 0;
  unsigned int MAX_ATTEMPTS = 10;

  while (putWriteStart(size) != SUCCESS || putWriteEnd(data, size) != SUCCESS)
  {
    attempts++;
    if (attempts == MAX_ATTEMPTS)
    {
      return ERROR_FTPPUT11;
    }
  }

  return result;
}

Result FTP::putWriteStart(unsigned int size)
{
  Result result = SUCCESS;

  char buffer[32];
  char resp[32];

  sprintf_P(buffer, AT_FTPPUT2, size);
  strcpy_P(resp, AT_FTPPUT2_RESP);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
  {
    return ERROR_FTPPUT2;
  }

  return result;
}

Result FTP::putWriteEnd(const char *data, unsigned int size)
{
  Result result = SUCCESS;

  char buffer[32];
  char resp[32];

  write(data, size);

  strcpy_P(resp, AT_FTPPUT1_RESP);
  if (waitForResp(resp, 2000) == FALSE)
  {
    return ERROR_FTPPUT11;
  }

  return result;
}

Result FTP::putEnd()
{
  Result result = SUCCESS;
  serialSIM800.flush();

  char buffer[32];
  char resp[12];

  strcpy_P(buffer, AT_FTPPUT20);
  strcpy_P(resp, AT_FTPPUT20_RESP);
  if (sendCmdAndWaitForResp(AT_FTPPUT20, AT_FTPPUT20_RESP, 2000) == FALSE)
  {
    return ERROR_FTPPUT20;
  }
  return result;
}
