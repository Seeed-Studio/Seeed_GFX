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
#include <TFT_Interface.h>


#ifdef HASSPI
TFT_Interface::TFT_Interface(SPIClass* spi) {
    this->_SPI = spi;
}
#else
TFT_Interface::TFT_Interface() {

}
#endif

TFT_Interface::~TFT_Interface() {

}

void TFT_Interface::begin() {
    #ifdef HASSPI
    this->_SPI->begin();
    #else
    interface_begin();
    #endif
    return;
}

void TFT_Interface::end() {
    #ifdef HASSPI
    this->_SPI->end();
    #else
    interface_end();
    #endif
    return;
}


byte TFT_Interface::transfer(uint8_t data) {
    #ifdef HASSPI
    return this->_SPI->transfer(data);
    #else
    return interface_transfer(data);
    #endif
}

uint16_t TFT_Interface::transfer16(uint16_t data) {
    #ifdef HASSPI
    return this->_SPI->transfer16(data);
    #else
    return interface_transfer16(data);
    #endif
}
void TFT_Interface::transfer(void* buf, size_t count) {
    #ifdef HASSPI
    return this->_SPI->transfer(buf, count);
    #else
    return interface_transfer(buf, count);
    #endif
}
#if defined (__SAMD55__)
void TFT_Interface::transfer(const void* txbuf, void* rxbuf, size_t count, bool block) {
    #ifdef HASSPI
    return this->_SPI->transfer(txbuf, rxbuf, count, block);
    #else
    return;
    #endif
}
#endif

#ifdef HASSPI
void TFT_Interface::beginTransaction(SPISettings settings) {
    this->_SPI->beginTransaction(settings);
}
void TFT_Interface::endTransaction() {
    this->_SPI->endTransaction();
}
#else
void TFT_Interface::beginTransaction() {

}
void TFT_Interface::endTransaction() {

}
#endif

void TFT_Interface::writeCommand(uint8_t c) {
    #ifdef HASSPI
    DC_C;
    transfer(c);
    #else
    interface_writeCommand(c);
    #endif

}
void TFT_Interface::writeData(uint8_t d) {
    #ifdef HASSPI
    DC_D;
    transfer(d);
    #else
    interface_writeData(d);
    #endif
}