/*
 * sim800.h
 * A library for SeeedStudio seeeduino GPRS shield 
 *
 * Copyright (c) 2013 seeed technology inc.
 * Author        :   lawliet zou
 * Create Time   :   Dec 2013
 * Change Log    :
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

#include "Arduino.h"
#include <SoftwareSerial.h>

#define TRUE                    1
#define FALSE                   0

#define SIM800_TX_PIN           8
#define SIM800_RX_PIN           7
#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     12

#define UART_DEBUG

#ifdef UART_DEBUG
#define ERROR(x)            Serial.println(x)
#define DEBUG(x)            Serial.println(x);
#else
#define ERROR(x)
#define DEBUG(x)
#endif

#define DEFAULT_TIMEOUT     5

/** SIM800 class.
 *  Used for SIM800 communication. attention that SIM800 module communicate with MCU in serial protocol
 */
class SIM800
{

public:
    /** Create SIM800 Instance 
     *  @param tx   uart transmit pin to communicate with SIM800
     *  @param rx   uart receive pin to communicate with SIM800
     *  @param baudRate baud rate of uart communication
     */
    SIM800(int baudRate):serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN){
        powerPin = SIM800_POWER_PIN;
        pinMode(powerPin,OUTPUT);
        serialSIM800.begin(baudRate);
    };
    
    /** Power on SIM800
     */
    void preInit(void);
    
    /** Check if SIM800 readable
     */
    int checkReadable(void);
    
    /** read from SIM800 module and save to buffer array
     *  @param  buffer  buffer array to save what read from SIM800 module
     *  @param  count   the maximal bytes number read from SIM800 module
     *  @param  timeOut time to wait for reading from SIM800 module 
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int readBuffer(char* buffer,int count, unsigned int timeOut = DEFAULT_TIMEOUT);

    
    /** clean Buffer
     *  @param buffer   buffer to clean
     *  @param count    number of bytes to clean
     */
    void cleanBuffer(char* buffer, int count);
    
    /** send AT command to SIM800 module
     *  @param cmd  command array which will be send to GPRS module
     */
    void sendCmd(const char* cmd);

    /**send "AT" to SIM800 module
     */
    int sendATTest(void);
    
    /**send '0x1A' to SIM800 Module
     */
    void sendEndMark(void);
    
    /** check SIM800 module response before time out
     *  @param  *resp   correct response which SIM800 module will return
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      0 on success
     *      -1 on error
     */ 
    int waitForResp(const char* resp, unsigned timeout);

    /** send AT command to GPRS module and wait for correct response
     *  @param  *cmd    AT command which will be send to GPRS module
     *  @param  *resp   correct response which GPRS module will return
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int sendCmdAndWaitForResp(const char* cmd, const char *resp, unsigned timeout);


    /** used for serial debug, you can specify tx and rx pin and then communicate with GPRS module with common AT commands
     */
    void serialDebug(void);
    
    int powerPin;
    SoftwareSerial serialSIM800;

private:
    
};

#endif