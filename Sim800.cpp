/*
 * Sim800.cpp
 * A library for SeeedStudio seeeduino GPRS shield
 *
 * Original work Copyright (c) 2013 seeed technology inc. [lawliet zou]
 * Modified work Copyright 2016 Antonio Carrasco
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

    return TRUE;
}

int SIM800::checkReadable(void)
{
    return serialSIM800.available();
}

int SIM800::readBuffer(char *buffer, int count, unsigned int timeOut)
{
    int i = 0;
    unsigned long timerStart,timerEnd;
    timerStart = millis();
    while(1) {
        while (serialSIM800.available()) {
            char c = serialSIM800.read();
            buffer[i] = c;
            buffer[i + 1] = '\0';
            ++i;
            if(i > count-1)break;
        }
        if(i > count-1)break;
        timerEnd = millis();
        if(timerEnd - timerStart > timeOut) {
            break;
        }
    }
    delay(500);
    while(serialSIM800.available()) {
        serialSIM800.read();
    }
    return TRUE;
}

void SIM800::cleanBuffer(char *buffer, int count)
{
    for(int i=0; i < count; i++) {
        buffer[i] = '\0';
    }
}

void SIM800::sendCmd(const char* cmd)
{
    serialSIM800.listen();
    serialSIM800.write(cmd);
}

int SIM800::sendATTest(void)
{
    int ret = sendCmdAndWaitForResp("AT\r\n", "OK", DEFAULT_TIMEOUT);
    return ret;
}

int SIM800::waitForResp(const char *resp, unsigned int timeout)
{
    int len = strlen(resp);
    int sum=0;
    unsigned long timerStart,timerEnd;
    timerStart = millis();

    while(1) {
        if(serialSIM800.available()) {
            char c = serialSIM800.read();
            if (debugMode) Serial.print(c);
            sum = (c==resp[sum]) ? sum+1 : 0;
            if(sum == len)break;
        }
        timerEnd = millis();
        if(timerEnd - timerStart > timeout) {
            return FALSE;
        }
    }

    while(serialSIM800.available()) {
        serialSIM800.read();
    }

    return TRUE;
}

void SIM800::sendEndMark(void)
{
    serialSIM800.println((char)26);
}

int SIM800::sendCmdAndWaitForResp(const char* cmd, const char *resp, unsigned timeout)
{
    delay(1000);
    sendCmd(cmd);
    return waitForResp(resp,timeout);
}

void SIM800::serialDebug(void)
{
    while(1) {
        if(serialSIM800.available()){
            Serial.write(serialSIM800.read());
        }
        if(Serial.available()){
            serialSIM800.write(Serial.read());
        }
    }
}

void SIM800::purgeSerial()
{
  while (serialSIM800.available()) serialSIM800.read();
}