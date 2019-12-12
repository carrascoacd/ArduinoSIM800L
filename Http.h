/*
 * Http.h
 * HTTP library for the SIM800L board
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

#ifndef __HTTP_H__
#define __HTTP_H__

#include "Sim800.h"
#include "Result.h"

class HTTP : public SIM800
{

public:
  HTTP(unsigned int baudRate,
       unsigned int rxPin,
       unsigned int txPin,
       unsigned int rstPin) : SIM800(baudRate, rxPin, txPin, rstPin){};
  Result connect(const char *apn);
  Result disconnect();
  Result get(const char *uri, char *response);
  Result post(const char *uri, const char *body, char *response);
  unsigned int readVoltage();
  unsigned int readVoltagePercentage();
  unsigned int readSignalStrength();

private:
  void readResponse(char *response);
  Result setHTTPSession(const char *uri);
};

#endif
