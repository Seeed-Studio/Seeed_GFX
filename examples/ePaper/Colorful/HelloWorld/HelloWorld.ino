#include "TFT_eSPI.h"

#ifdef EPAPER_ENABLE // Only compile this code if the EPAPER_ENABLE is defined in User_Setup.h
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
    epaper.begin();
    epaper.fillScreen(EPAPER_WHITE);

    epaper.fillCircle(25, 25, 15, EPAPER_RED);
    epaper.fillRect(epaper.width() - 40,  10, 30, 30, EPAPER_GREEN);
    
    for (int i = 0; i < epaper.height() / 80; i++)
    {
        epaper.setTextColor(EPAPER_BLUE);
        epaper.setTextSize(i + 1);
        epaper.drawLine(10, 70 + 60 * i, epaper.width() - 10, 70 + 60 * i, EPAPER_BLACK);
        epaper.drawString("Hello ePaper", 10, 80 + 60 * i);
    }
    
    epaper.update(); // update the display

#endif
}

void loop()
{
    // put your main code here, to run repeatedly:
}
