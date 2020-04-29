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

const char AT_OK[] PROGMEM = "OK";
const char AT_OK_[] = "OK";
const char AT_FTPPUT1_RESP[] PROGMEM = "1,1";
const char AT_FTPPUT2_RESP[] PROGMEM = "+FTPPUT: 2";
const char AT_FTPPUT20_RESP[] PROGMEM = "1,0";
const char CONNECTED[] PROGMEM = "+CREG: 0,1";
const char ROAMING[] PROGMEM = "+CREG: 0,5";
const char BEARER_OPEN[] PROGMEM = "+SAPBR: 1,1";

#include "GPRS.h"

Result FTP::putBegin(const char *apn,
                     const char *fileName,
                     const char *server,
                     const char *usr,
                     const char *pass,
                     const char *path)
{
  Result result = openGPRSContext(this, apn);

  char buffer[64];
  char tmp[24];

  delay(10000);
  if (sendCmdAndWaitForResp_P(AT_FTPCID, AT_OK, 2000) == FALSE)
    return ERROR_FTPCID;

  strcpy_P(tmp, server);
  sprintf_P(buffer, AT_FTPSERV, tmp);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    return ERROR_FTPSERV;

  strcpy_P(tmp, usr);
  sprintf_P(buffer, AT_FTPUN, tmp);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    return ERROR_FTPUN;

  strcpy_P(tmp, pass);
  sprintf_P(buffer, AT_FTPPW, tmp);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    return ERROR_FTPPW;

  sprintf_P(buffer, AT_FTPPUTNAME, fileName);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    return ERROR_FTPPUTNAME;

  sprintf_P(buffer, AT_FTPPUTPATH, path);
  if (sendCmdAndWaitForResp(buffer, AT_OK_, 2000) == FALSE)
    return ERROR_FTPPUTPATH;

  if (sendCmdAndWaitForResp_P(AT_FTPPUT1, AT_FTPPUT1_RESP, 10000) == FALSE)
    return ERROR_FTPPUT1;

  return result;
}

Result FTP::putWrite(const char *data, unsigned int size)
{
  Result result = SUCCESS;

  uint8_t attempts = 0;
  uint8_t MAX_ATTEMPTS = 10;

  while (putWriteStart(size) != SUCCESS || putWriteEnd(data, size) != SUCCESS)
  {
    attempts++;
    if (attempts == MAX_ATTEMPTS)
      return ERROR_FTPPUT11;
  }

  return result;
}

Result FTP::putWriteStart(unsigned int size)
{
  Result result = SUCCESS;

  char buffer[22];
  char resp[11];

  sprintf_P(buffer, AT_FTPPUT2, size);
  strcpy_P(resp, AT_FTPPUT2_RESP);
  if (sendCmdAndWaitForResp(buffer, resp, 2000) == FALSE)
    return ERROR_FTPPUT2;

  return result;
}

Result FTP::putWriteEnd(const char *data, unsigned int size)
{
  Result result = SUCCESS;
  char resp[4];

  write(data, size);

  strcpy_P(resp, AT_FTPPUT1_RESP);
  if (waitForResp(resp, 2000) == FALSE)
    return ERROR_FTPPUT11;

  return result;
}

Result FTP::putEnd()
{
  Result result = closeGPRSContext(this);

  if (sendCmdAndWaitForResp_P(AT_FTPPUT20, AT_FTPPUT20_RESP, 2000) == FALSE)
    return ERROR_FTPPUT20;

  return result;
}
