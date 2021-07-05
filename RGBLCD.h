#ifndef __RGBLCD_H_
#define __RGBLCD_H_

#ifdef LTDC_SUPPORT
#include "stm32h7xx_hal.h"
#include "TFT_Drivers/LTDC_Defines.h"


enum LCD_STATE 
{
    DEFAULT_STATE,
    SET_X,
    SET_Y,
    DRAW_PIXEL_OR_SET_WINDOW,
    SET_ROTATION
};

enum LCD_ROTATION
{
    ROTATION_UP,
    ROTATION_LEFT,
    ROTATION_RIGHT,
    ROTATION_DOWN
};



class LCDClass
{
private:
    volatile uint16_t*   DataBuffer;
    volatile uint16_t    x;
    volatile uint16_t    x_start;
    volatile uint16_t    x_end;
    volatile uint16_t    y;
    volatile uint16_t    y_start;
    volatile uint16_t    y_end;
    volatile uint32_t    color;
    volatile enum LCD_STATE     state;
    volatile enum LCD_ROTATION  rotation;
public:
    volatile uint16_t x_border_left;
    volatile uint16_t x_border_right;
    volatile uint16_t y_border_left;
    volatile uint16_t y_border_right;

    LCDClass(uint32_t addr);
    ~LCDClass();
    void begin(void);

    void set_state(enum LCD_STATE _state);
    enum LCD_STATE get_state(void);
    void set_x(uint16_t _x);
    void set_x_border(uint16_t _x_s , uint16_t _x_e);
    void get_x_border(uint16_t* _x_s , uint16_t* _x_e);
    uint16_t get_x(void);
    void set_y(uint16_t _y);
    void set_y_border(uint16_t _y_s , uint16_t _y_e);
    void get_y_border(uint16_t* _y_s , uint16_t* _y_e);
    uint16_t get_y(void);
    void set_color(uint32_t _color);
    void set_rotation(enum LCD_ROTATION  _rotation);
    enum LCD_ROTATION get_rotation(void);

    void draw_pixel(void);
};

#endif

#endif
