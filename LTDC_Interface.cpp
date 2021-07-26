
#include <Arduino.h>
#include "RGBLCD.h"
#include <avr/pgmspace.h>
#ifdef HAL_LTDC_MODULE_ENABLED

EXTMEM uint8_t videobuffer[TFT_WIDTH * TFT_HEIGHT * 2];

LCDClass RGB565LCD((uint32_t)videobuffer);

enum LCD_STATE _state;

void interface_begin()
{
    RGB565LCD.begin();
}

uint8_t interface_transfer(uint8_t data)
{
   RGB565LCD.write((uint8_t)data);
   return data;
}

uint16_t interface_transfer16(uint16_t data)
{
    RGB565LCD.write(data);
    return data;
}

void interface_transfer(void *data, uint32_t count)
{
    RGB565LCD.write(data, count);
}

void interface_end()
{
}

void interface_writeCommand(uint8_t c)
{
    RGB565LCD.command(c);
}

void interface_writeData(uint8_t d)
{
    RGB565LCD.write(d);
}

#endif
