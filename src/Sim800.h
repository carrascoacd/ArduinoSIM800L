/*
 * Sim800.h
 * A library for SeeedStudio seeeduino GPRS shield
 *
 * Original work Copyright (c) 2013 seeed technology inc. [lawliet zou]
 * Modified work Copyright 2018 Antonio Carrasco
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

#ifndef __SIM800_H__
#define __SIM800_H__

#include <Arduino.h>
#include <SoftwareSerial.h>

#define TRUE 1
#define FALSE 0
#define DEFAULT_TIMEOUT 5000

// Comment or uncomment this to debug the library
// #define DEBUG true

/** SIM800 class.
 *  Used for SIM800 communication. attention that SIM800 module communicate with MCU in serial protocol
 */

class SIM800
{

public:
    /** Create SIM800 Instance
     *  @param baudRate baud rate of uart communication
     *  @param rxPin uart receive pin to communicate with SIM800
     *  @param txPin uart transmit pin to communicate with SIM800
     */
    SIM800(unsigned int baudRate,
           unsigned int rxPin,
           unsigned int txPin,
           unsigned int rstPin) : serialSIM800(txPin, rxPin)
    {
        serialSIM800.begin(baudRate);
        resetPin = rstPin;
    };

    /** Power on SIM800
     */
    int preInit(void);

    /** Check if SIM800 readable
     */
    int checkReadable(void);

    /** read from SIM800 module and save to buffer array
     *  @param  buffer  buffer array to save what read from SIM800 module
     *  @param  count   the maximal bytes number read from SIM800 module
     *  @param  timeOut time to wait for reading from SIM800 module
     *  @returns
     *      TRUE on success
     *      ERROR on error
     */
    int readBuffer(char *buffer, int count, unsigned int timeOut = DEFAULT_TIMEOUT);

    /** clean Buffer
     *  @param buffer   buffer to clean
     *  @param count    number of bytes to clean
     */
    void cleanBuffer(char *buffer, int count);

    /** send AT command to SIM800 module
     *  @param cmd  command array which will be send to GPRS module
     *  @param delayBeforeSend  integer indicating the sime to wait before sending a command
     */
    void sendCmd(const char *cmd, unsigned int delayBeforeSend = 10);

    /**send "AT" to SIM800 module
     */
    int sendATTest(void);

    /**send '0x1A' to SIM800 Module
     */
    void sendEndMark(void);

    /** check SIM800 module response before time out
     *  @param  *resp   correct response which SIM800 module will return. A 'X' char
     *  can be used to allow any char matching character, example:
     *  '200' and '201' matches with '20X'
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      TRUE on success
     *      ERROR on error
     */
    int waitForResp(const char *resp, unsigned timeout);

    /** send AT command to GPRS module and wait for correct response
     *  @param  *cmd    AT command which will be send to GPRS module
     *  @param  *resp   correct response which GPRS module will return
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      TRUE on success
     *      ERROR on error
     */
    int sendCmdAndWaitForResp(const char *cmd, const char *resp, unsigned timeout);

    /** 
     *  The sendCmdAndWaitForResp_P() function is similar to sendCmdAndWaitForResp() 
     *  except that cmd and resp are pointers to a string in program space.
     */
    int sendCmdAndWaitForResp_P(const char *cmd, const char *resp, unsigned timeout);

    /** used for serial debug, you can specify tx and rx pin and then communicate with GPRS module with common AT commands
     */
    void serialDebug(void);

    void purgeSerial();

    void write(const char *data);
    void write(const char *data, unsigned int size);

    void sleep(bool force = FALSE);
    void wakeUp();

protected:
    SoftwareSerial serialSIM800;
    unsigned int resetPin;
};

#endif