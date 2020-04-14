/*
 * Sim800.cpp
 * A library for SeeedStudio seeeduino GPRS shield
 *
 * Original work Copyright (c) 2013 seeed technology inc. [lawliet zou]
 * Modified work Copyright 2019 Antonio Carrasco
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

#include "Sim800.h"

const char SLEEP_MODE_2[] PROGMEM = "AT+CSCLK=2\r\n";
const char SLEEP_MODE_1[] PROGMEM = "AT+CSCLK=1\r\n";
const char SLEEP_MODE_0[] PROGMEM = "AT+CSCLK=0\r\n";
const char AT_OK[] PROGMEM = "OK";

int SIM800::preInit(void)
{
    pinMode(resetPin, OUTPUT);

    digitalWrite(resetPin, HIGH);
    delay(200);
    digitalWrite(resetPin, LOW);
    delay(2000);
    digitalWrite(resetPin, HIGH);
    delay(3000);

    purgeSerial();
    serialSIM800.flush();

    return TRUE;
}

int SIM800::checkReadable(void)
{
    return serialSIM800.available();
}

int SIM800::readBuffer(char *buffer, int count, unsigned int timeOut)
{
    int i = 0;
    unsigned long timerStart = millis();
    while (1)
    {
        while (serialSIM800.available())
        {
            char c = serialSIM800.read();
            buffer[i] = c;
            buffer[i + 1] = '\0';
            ++i;
            if (i > count - 1)
                break;
        }
        if (i > count - 1)
            break;

        unsigned long timerEnd = millis();
        if (timerEnd - timerStart > timeOut)
        {
            break;
        }
    }

    while (serialSIM800.available())
    {
        serialSIM800.read();
    }
    return TRUE;
}

void SIM800::cleanBuffer(char *buffer, int count)
{
    for (int i = 0; i < count; i++)
    {
        buffer[i] = '\0';
    }
}

void SIM800::sendCmd(const char *cmd, unsigned int delayBeforeSend)
{
    serialSIM800.listen();
    serialSIM800.flush();
    delay(delayBeforeSend);
    write(cmd);
    serialSIM800.flush();
}

int SIM800::sendATTest(void)
{
    int ret = sendCmdAndWaitForResp("AT\r\n", "OK", DEFAULT_TIMEOUT);
    return ret;
}

int SIM800::waitForResp(const char *resp, unsigned int timeout)
{
    int len = strlen(resp);
    int sum = 0;
    unsigned long timerStart = millis();

    while (1)
    {
        if (serialSIM800.available())
        {
            char c = serialSIM800.read();
            
            #ifdef DEBUG
              Serial.print(c);
            #endif
                
            sum = (c == resp[sum] || resp[sum] == 'X') ? sum + 1 : 0;
            if (sum == len)
                break;
        }
        unsigned long timerEnd = millis();
        if (timerEnd - timerStart > timeout)
        {
            return FALSE;
        }
    }

    while (serialSIM800.available())
    {
        serialSIM800.read();
    }

    return TRUE;
}

void SIM800::sendEndMark(void)
{
    serialSIM800.println((char)26);
}

int SIM800::sendCmdAndWaitForResp(const char *cmd, const char *resp, unsigned timeout)
{
    sendCmd(cmd);
    return waitForResp(resp, timeout);
}

int SIM800::sendCmdAndWaitForResp_P(const char *cmd, const char *resp, unsigned timeout)
{
    char cmdBuff[128];
    char respBuff[32];
    strcpy_P(cmdBuff, cmd);
    strcpy_P(respBuff, resp);

    return sendCmdAndWaitForResp(cmdBuff, respBuff, timeout);
}

void SIM800::serialDebug(void)
{
    while (1)
    {
        if (serialSIM800.available())
        {
            Serial.write(serialSIM800.read());
        }
        if (Serial.available())
        {
            serialSIM800.write(Serial.read());
        }
    }
}

void SIM800::purgeSerial()
{
    while (serialSIM800.available())
        serialSIM800.read();
}

void SIM800::write(const char *data)
{
    serialSIM800.listen();
    serialSIM800.write(data);
}

void SIM800::write(const char *data, unsigned int size)
{
    serialSIM800.listen();
    serialSIM800.write(data, size);
}

void SIM800::sleep(bool force)
{
    if (force)
    {
        sendCmdAndWaitForResp_P(SLEEP_MODE_1, AT_OK, 2000);
    }
    else
    {
        sendCmdAndWaitForResp_P(SLEEP_MODE_2, AT_OK, 2000);
    }
}

void SIM800::wakeUp()
{
    preInit();
}
