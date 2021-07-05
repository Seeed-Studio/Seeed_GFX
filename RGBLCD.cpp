#include <Arduino.h>
#include <User_Setup.h>
#include "RGBLCD.h"

#ifdef HAL_LTDC_MODULE_ENABLED

extern void MX_LTDC_Init(const uint32_t addr);

LCDClass::LCDClass(uint32_t addr)
{
    DataBuffer = (volatile uint16_t*)addr;
    /*border*/
    x_start = 0;
    x_end   = TFT_WIDTH;
    y_start = 0;
    y_end   = TFT_HEIGHT;
    x_border_left = x_border_right = 0;
    y_border_left = y_border_right = 0;
    /*direction*/
    rotation = ROTATION_UP;
    /*start point*/
    x = 0;
    y = 0;
}

LCDClass::~LCDClass()
{

}

void LCDClass::begin(void)
{
    MX_LTDC_Init((uint32_t)DataBuffer);
}

void LCDClass::set_state(enum LCD_STATE _state)
{
    state = _state;
}

enum LCD_STATE LCDClass::get_state(void)
{
    return state;
}

void LCDClass::set_x(uint16_t _x)
{
    x = _x;
}

void LCDClass::set_x_border(uint16_t _x_s , uint16_t _x_e)
{
    x_start = _x_s;
    x_end   = _x_e;
}

void LCDClass::get_x_border(uint16_t* _x_s , uint16_t* _x_e)
{
    *_x_s = x_start;
    *_x_e = x_end;
}

uint16_t LCDClass::get_x(void)
{
    return x;
}
void LCDClass::set_y(uint16_t _y)
{
    y = _y;
}

void LCDClass::set_y_border(uint16_t _y_s , uint16_t _y_e)
{
    y_start = _y_s;
    y_end   = _y_e;
}

void LCDClass::get_y_border(uint16_t* _y_s , uint16_t* _y_e)
{
    *_y_s = y_start;
    *_y_e = y_end;
}

uint16_t LCDClass::get_y(void)
{
    return y;
}

void LCDClass::set_color(uint32_t _color)
{
    color = _color;
}

void LCDClass::set_rotation(enum LCD_ROTATION  _rotation)
{
  rotation = _rotation;
}

enum LCD_ROTATION LCDClass::get_rotation(void)
{
  return rotation;
}

void LCDClass::draw_pixel(void)
{
    switch(rotation)
    {
      case ROTATION_UP:
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( (TFT_WIDTH * y) + x ) * 2 ) = (uint16_t)(color & 0xffff);   
      break;
      case ROTATION_LEFT:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( ( TFT_WIDTH * x ) + TFT_WIDTH - y - 1) * 2 ) = (uint16_t)(color & 0xffff);
      break;
      case ROTATION_RIGHT:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( ( TFT_WIDTH * (TFT_HEIGHT - x - 1) ) + y ) * 2 ) = (uint16_t)(color & 0xffff);
      break;
      case ROTATION_DOWN:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( (TFT_WIDTH * (TFT_HEIGHT - y - 1) ) + TFT_WIDTH - x - 1) * 2 ) = (uint16_t)(color & 0xffff);
      break;
      default:
      break;
    }
}

#endif