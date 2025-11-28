#include "TFT_eSPI.h"

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
  for (int i = 0; i < epaper.height() / 80; i++)
  {
    epaper.setTextSize(i + 1);
    epaper.drawString("Hello ePaper", 10, 80 + 60 * i);
    epaper.update(); // update the display
  }

  // random rectangles
  for (int i = 0; i < 10; i++)
  {
    epaper.fillRect(random(epaper.width()), random(epaper.height()), random(30), random(30), TFT_GRAY_2);
    epaper.update(); // update the display
  }

  // random circles
  for (int i = 0; i < 10; i++)
  {
    epaper.fillCircle(random(epaper.width()), random(epaper.height()), random(30), TFT_GRAY_1);
    epaper.update(); // update the display
  }

  delay(2000);

  for (int i = 0; i < epaper.width() / 80; i++)
  {
    // automatically wakes up, but you need to sleep again after drawing
    epaper.fillRect(10 + 80 * i, 10, 40, 40, TFT_GRAY_2);
    epaper.update();
  }
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
}
