/*This is a 4-color electronic ink screen, which can only display 4 colors. 

Here is the 4 colors you can display:
1.TFT_GRAY_0    black
2.TFT_GRAY_1      |
3.TFT_GRAY_2      |
4.TFT_GRAY_3    white

*/

#include "TFT_eSPI.h"

#ifdef EPAPER_ENABLE // Only compile this code if the EPAPER_ENABLE is defined in User_Setup.h
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
    epaper.begin();
    epaper.initGrayMode(GRAY_LEVEL4);

    epaper.fillCircle(25, 25, 15, TFT_GRAY_1);
    epaper.fillRect(epaper.width() - 40,  10, 30, 30, TFT_GRAY_1);
    
    for (int i = 0; i < epaper.height() / 80; i++)
    {
        epaper.setTextColor(TFT_GRAY_2);
        epaper.setTextSize(i + 1);
        epaper.drawLine(10, 70 + 60 * i, epaper.width() - 10, 70 + 60 * i, TFT_GRAY_1);
        epaper.drawString("Hello ePaper", 10, 80 + 60 * i);
    }
    
    epaper.update(); // update the display

#endif
}

void loop()
{
    // put your main code here, to run repeatedly:
}
