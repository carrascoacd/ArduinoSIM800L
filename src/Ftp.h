/*
 * Ftp.h
 * FTP library for the SIM800L board
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

#ifndef __FTP_H__
#define __FTP_H__

#include "Sim800.h"
#include "Result.h"

class FTP : public SIM800
{

public:
  FTP(unsigned int baudRate,
      unsigned int rxPin,
      unsigned int txPin,
      unsigned int rstPin) : SIM800(baudRate, rxPin, txPin, rstPin){};

  Result putBegin(const char *apn,
                  const char *fileName,
                  const char *server,
                  const char *usr,
                  const char *pass,
                  const char *path = "/");
  Result putWrite(const char *data, unsigned int size);
  Result putEnd();

private:
  Result putWriteStart(unsigned int size);
  Result putWriteEnd(const char *data, unsigned int size);
};

#endif
