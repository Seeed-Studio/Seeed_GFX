/*This is a 4-color electronic ink screen, which can only display 4 colors. 

Here is the 4 colors you can display:
1.TFT_GRAY_0    black
2.TFT_GRAY_1      |
3.TFT_GRAY_2      |
4.TFT_GRAY_3    white

*/

#include "TFT_eSPI.h"
#include "image.h"
#ifdef EPAPER_ENABLE  // Only compile this code if the EPAPER_ENABLE is defined in User_Setup.h
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE  
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);
  epaper.update(); // update the display
  epaper.initGrayMode(GRAY_LEVEL4);


  epaper.fillRect(0,  0, epaper.width(), epaper.height() / 4, TFT_GRAY_0);
  epaper.fillRect(0,  epaper.height() * 1 / 4, epaper.width(), epaper.height() / 4, TFT_GRAY_1);
  epaper.fillRect(0,  epaper.height() * 2 / 4, epaper.width(), epaper.height() / 4, TFT_GRAY_2);
  epaper.fillRect(0,  epaper.height() * 3 / 4, epaper.width(), epaper.height() / 4, TFT_GRAY_3);
  epaper.update();

  epaper.fillScreen(TFT_GRAY_3);
  epaper.pushImage(0, 0, 800, 480, (uint16_t *)L4_GRAY);
  epaper.update();
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
}
