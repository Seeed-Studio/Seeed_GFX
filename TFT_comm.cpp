#include <Arduino.h>
#include "RGBLCD.h"


LCDClass RGB565LCD(LTDC_BUFFER_ADDRESS);
enum LCD_STATE _state;
static uint16_t border[4];

void interface_begin()
{
    pinMode(LED_RED, OUTPUT);
    for(int i = 0;i < 6; i++)
    {
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
    }
    RGB565LCD.begin();
}

uint8_t interface_transfer(uint8_t data)
{
    static volatile uint8_t temp_data[4];
    static volatile uint8_t FLAG = 0;
    uint16_t temp_border[2];
    enum LCD_ROTATION _rotation;

    _state      = RGB565LCD.get_state();
    _rotation   = RGB565LCD.get_rotation(); 
    switch (_state)
    {
        case SET_X:
            temp_data[FLAG] = data;
            FLAG ++;
            if(FLAG == 4)
            {
                temp_border[0] = (uint16_t)(temp_data[0]<<8 | temp_data[1]);
                temp_border[1] = (uint16_t)(temp_data[2]<<8 | temp_data[3]);
                if(_rotation == ROTATION_LEFT || _rotation == ROTATION_RIGHT)
                {
                    temp_border[0] = temp_border[0] >= TFT_HEIGHT ? TFT_HEIGHT : temp_border[0];
                    temp_border[1] = temp_border[1] >= TFT_HEIGHT ? TFT_HEIGHT : temp_border[1];
                }   
                else
                {
                    temp_border[0] = temp_border[0] >= TFT_WIDTH ? TFT_WIDTH : temp_border[0];
                    temp_border[1] = temp_border[1] >= TFT_WIDTH ? TFT_WIDTH : temp_border[1];
                }
                RGB565LCD.set_x_border(temp_border[0],temp_border[1]);
                border[0] = temp_border[0];
                border[1] = temp_border[1];              
                FLAG = 0;
            }
        break;

        case SET_Y:
            temp_data[FLAG] = data;
            FLAG ++;
            if(FLAG == 4)
            {
                temp_border[0] = (uint16_t)(temp_data[0]<<8 | temp_data[1]);
                temp_border[1] = (uint16_t)(temp_data[2]<<8 | temp_data[3]);
                if(_rotation == ROTATION_LEFT || _rotation == ROTATION_RIGHT)
                {
                    temp_border[0] = temp_border[0] >= TFT_HEIGHT ? TFT_HEIGHT : temp_border[0];
                    temp_border[1] = temp_border[1] >= TFT_HEIGHT ? TFT_HEIGHT : temp_border[1];
                }   
                else
                {
                    temp_border[0] = temp_border[0] >= TFT_WIDTH ? TFT_WIDTH : temp_border[0];
                    temp_border[1] = temp_border[1] >= TFT_WIDTH ? TFT_WIDTH : temp_border[1];
                }
                RGB565LCD.set_y_border(temp_border[0],temp_border[1]);
                border[2] = temp_border[0];
                border[3] = temp_border[1];     
                FLAG = 0;
            }
        break;

        case SET_ROTATION:     
            switch (data)
            {
                case (TFT_ROTATION_UP):     RGB565LCD.set_rotation(ROTATION_UP);        break;
                case (TFT_ROTATION_LEFT):   RGB565LCD.set_rotation(ROTATION_LEFT);      break;
                case (TFT_ROTATION_RIGHT):  RGB565LCD.set_rotation(ROTATION_RIGHT);     break;
                case (TFT_ROTATION_DOWN):   RGB565LCD.set_rotation(ROTATION_DOWN);      break;
                default:                    RGB565LCD.set_rotation(ROTATION_UP);        break;
            }        
        break;
        default:
        break;
    }
    return 1;
}

uint16_t interface_transfer16(uint16_t data)
{
    uint16_t temp_border[4];

    _state = RGB565LCD.get_state();
    switch (_state)
    {
        case DRAW_PIXEL_OR_SET_WINDOW:
            RGB565LCD.set_color(data);
            RGB565LCD.get_x_border(&temp_border[0],&temp_border[1]);
            RGB565LCD.get_y_border(&temp_border[2],&temp_border[3]);
            if( (temp_border[0] == temp_border[1]) && (temp_border[2] == temp_border[3]) )
            {
                RGB565LCD.set_x(temp_border[0]);
                RGB565LCD.set_y(temp_border[2]);
                RGB565LCD.draw_pixel();
            }
            RGB565LCD.set_state(DEFAULT_STATE);
        break;
        case DEFAULT_STATE:
            RGB565LCD.set_color(data);
            RGB565LCD.get_x_border(&temp_border[0],&temp_border[1]);
            RGB565LCD.get_y_border(&temp_border[2],&temp_border[3]);
            RGB565LCD.set_x(temp_border[0]);
            RGB565LCD.set_y(temp_border[2]);
            RGB565LCD.draw_pixel();
            temp_border[0] ++;
            if(temp_border[0] > temp_border[1])
            {
                temp_border[0] = border[0];
                temp_border[2] ++;
            }
            RGB565LCD.set_x_border(temp_border[0],temp_border[1]); 
            RGB565LCD.set_y_border(temp_border[2],temp_border[3]); 
        break;
        default:
        break;
    }
    return 1;
}

void interface_transfer(void *data, uint32_t count)
{ 

}

void interface_end()
{

}

void interface_writeCommand(uint8_t c)
{
    switch (c)
    {
        case TFT_CASET: RGB565LCD.set_state(SET_X); break;
        case TFT_PASET: RGB565LCD.set_state(SET_Y); break;
        case TFT_RAMWR: RGB565LCD.set_state(DRAW_PIXEL_OR_SET_WINDOW);  break;
        case TFT_SET_ROTATION:  RGB565LCD.set_state(SET_ROTATION);      break;
        default: break;
    }
}

void interface_writeData(uint8_t d)
{

}
