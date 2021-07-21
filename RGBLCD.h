#ifndef __RGBLCD_H_
#define __RGBLCD_H_

#include "Arduino.h"

#ifdef HAL_LTDC_MODULE_ENABLED
#include "User_Setup.h"
#include "stm32h7xx_hal.h"
#include "TFT_Drivers/LTDC_Defines.h"


enum LCD_STATE 
{
    DEFAULT_STATE,
    SET_WINDOW_X,
    SET_WINDOW_Y,
    SET_ROTATION,
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
    uint16_t    x_cur, y_cur;
    uint16_t    x_start;
    uint16_t    x_end;
    uint16_t    y_start;
    uint16_t    y_end;
    enum LCD_STATE     state_cur;
    enum LCD_ROTATION  rotation;
    uint8_t index;

public:

    LCDClass(uint32_t addr);
    ~LCDClass();
    void begin(void);
    void command(uint8_t cmd);
    void write(uint8_t data);
    void write(uint16_t data);
    void write(void* buf, size_t count);
};

#endif

#endif
