/*
 * GPRS.cpp
 * GPRS module that implements the basic AT sequences to interact with GPRS
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

#include "GPRS.h"
#include <string.h>

const char BEARER_PROFILE_GPRS[] PROGMEM = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n";
const char BEARER_PROFILE_APN[] PROGMEM = "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n";
const char QUERY_BEARER[] PROGMEM = "AT+SAPBR=2,1\r\n";
const char OPEN_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=1,1\r\n";
const char CLOSE_GPRS_CONTEXT[] PROGMEM = "AT+SAPBR=0,1\r\n";

const char SIGNAL_QUALITY[] PROGMEM = "AT+CSQ\r\n";
const char READ_VOLTAGE[] PROGMEM = "AT+CBC\r\n";
const char REGISTRATION_STATUS[] PROGMEM = "AT+CREG?\r\n";

const char CONNECTED[] PROGMEM = "+CREG: 0,1";
const char ROAMING[] PROGMEM = "+CREG: 0,5";
const char BEARER_OPEN[] PROGMEM = "+SAPBR: 1,1";
const char AT_OK[] PROGMEM = "OK";
const char AT_OK_[] = "OK";

Result openGPRSContext(SIM800 *sim800, const char *apn)
{
  Result result = SUCCESS;
  uint8_t attempts = 0;
  uint8_t MAX_ATTEMPTS = 10;

  sim800->sendATTest();

  while ((sim800->sendCmdAndWaitForResp_P(REGISTRATION_STATUS, CONNECTED, 2000) != TRUE &&
          sim800->sendCmdAndWaitForResp_P(REGISTRATION_STATUS, ROAMING, 2000) != TRUE) &&
         attempts < MAX_ATTEMPTS)
  {
    sim800->sendCmdAndWaitForResp_P(READ_VOLTAGE, AT_OK, 1000);
    sim800->sendCmdAndWaitForResp_P(SIGNAL_QUALITY, AT_OK, 1000);
    attempts++;
    delay(1000 * attempts);
    if (attempts == MAX_ATTEMPTS)
    {
      attempts = 0;
      sim800->preInit();
    }
  }

  if (sim800->sendCmdAndWaitForResp_P(BEARER_PROFILE_GPRS, AT_OK, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_GPRS;

  char httpApn[64];
  char tmp[24];
  strcpy_P(tmp, apn);
  sprintf_P(httpApn, BEARER_PROFILE_APN, tmp);
  if (sim800->sendCmdAndWaitForResp(httpApn, AT_OK_, 2000) == FALSE)
    result = ERROR_BEARER_PROFILE_APN;

  while (sim800->sendCmdAndWaitForResp_P(QUERY_BEARER, BEARER_OPEN, 2000) == FALSE && attempts < MAX_ATTEMPTS)
  {
    attempts++;
    if (sim800->sendCmdAndWaitForResp_P(OPEN_GPRS_CONTEXT, AT_OK, 2000) == FALSE)
    {
      result = ERROR_OPEN_GPRS_CONTEXT;
    }
    else
    {
      result = SUCCESS;
    }
  }

  return result;
}

Result closeGPRSContext(SIM800 *sim800)
{
  Result result = SUCCESS;

  if (sim800->sendCmdAndWaitForResp_P(CLOSE_GPRS_CONTEXT, AT_OK, 2000) == FALSE)
    result = ERROR_CLOSE_GPRS_CONTEXT;

  return result;
}
