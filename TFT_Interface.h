/**
    The MIT License (MIT)

    A interface abstraction layer

    Copyright (C) 2019  Seeed Technology Co.,Ltd.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <Arduino.h>
#include <User_Setup.h>

// interface if you want to use this library with you protocol, you neeed to realize them
extern void interface_begin();
extern uint8_t interface_transfer(uint8_t data);
extern uint16_t interface_transfer16(uint16_t data);
extern void interface_transfer(void* data, uint32_t count);
extern void interface_end();
extern void interface_writeCommand(uint8_t c);
extern void interface_writeData(uint8_t d);


#ifndef TFT_DC
    #define DC_C // No macro allocated so it generates no code
    #define DC_D // No macro allocated so it generates no code
#else
    #define DC_C digitalWrite(TFT_DC, LOW)
    #define DC_D digitalWrite(TFT_DC, HIGH)
#endif


#ifndef TFT_CS
    #define CS_L // No macro allocated so it generates no code
    #define CS_H // No macro allocated so it generates no code
#else
    #define CS_L digitalWrite(TFT_CS, LOW)
    #define CS_H digitalWrite(TFT_CS, HIGH)
#endif

// Use single register write for CS_L and DC_C if pins are both in range 0-31

#define CS_L_DC_C CS_L; DC_C

/*
    if you have Arduino SPI library, then you can use SPI library as interface.
    Otherwise, you can implement the following interface by yourself

*/

#ifdef HASSPI
    #ifdef KENDRYTE_K210
        #include <SPIClass.h>
    #else
        #include <SPI.h>
    #endif
#endif

class TFT_Interface {
  public:
    #ifdef HASSPI
    TFT_Interface(SPIClass* spi);
    #else
    TFT_Interface();
    #endif
    ~TFT_Interface();
    void begin();
    void end();
    void writeCommand(uint8_t c);
    void writeData(uint8_t d);
    #ifdef HASSPI
    void beginTransaction(SPISettings settings);
    void endTransaction();
    #else
    void beginTransaction();
    void endTransaction();
    #endif
    byte transfer(uint8_t data);
    uint16_t transfer16(uint16_t data);
    void transfer(void* buf, size_t count);
    #if defined (__SAMD55__)
    void transfer(const void* txbuf, void* rxbuf, size_t count, bool block = true);
    #endif
  private:
    #ifdef HASSPI
    SPIClass* _SPI;
    #endif
};