/*
 * Parser.cpp
 * Parser module to deal with parsing
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

#include "Parser.h"
#include <string.h>

void parseATResponse(const char *buffer, unsigned int size, unsigned int offset, char *response)
{
  const char *twoPointsPointer = strchr(buffer, ':');
  unsigned int twoPointsIndex = (int)(twoPointsPointer - buffer);
  unsigned int valueStartIndex = twoPointsIndex + offset;
  for (unsigned int i = valueStartIndex; i < valueStartIndex + size; ++i)
  {
    response[i - valueStartIndex] = buffer[i];
    response[i - valueStartIndex + 1] = '\0';
  }
}

void parseJSONResponse(const char *buffer, unsigned int bufferSize, char *response)
{
  unsigned int start_index = 0;
  unsigned int i = 0;
  while (i < bufferSize - 1 && start_index == 0)
  {
    char c = buffer[i];
    if ('{' == c)
    {
      start_index = i;
    }
    ++i;
  }

  unsigned int end_index = 0;
  int j = bufferSize - 1;
  while (j >= 0 && end_index == 0)
  {
    char c = buffer[j];
    if ('}' == c)
    {
      end_index = j;
    }
    --j;
  }

  for (int k = 0; k < (end_index - start_index) + 2; ++k)
  {
    response[k] = buffer[start_index + k];
    response[k + 1] = '\0';
  }
}
