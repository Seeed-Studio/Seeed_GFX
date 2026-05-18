/* 
This is a black and white dual-color screen, with example code supporting 16 levels of grayscale.
   TFT_GRAY_0   -> black
   ...
   TFT_GRAY_15  -> white
*/

#include "TFT_eSPI.h"

#ifdef EPAPER_ENABLE  // Only compile this code if EPAPER_ENABLE is defined in User_Setup.h
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);
  epaper.update();                       // refresh once to clear the screen
  epaper.initGrayMode(GRAY_LEVEL16);     // switch to 16-level gray mode

  // 16 levels of gray: TFT_GRAY_0 (black) ... TFT_GRAY_15 (white)
  const uint8_t grayLevels[16] = {
    TFT_GRAY_0,  TFT_GRAY_1,  TFT_GRAY_2,  TFT_GRAY_3,
    TFT_GRAY_4,  TFT_GRAY_5,  TFT_GRAY_6,  TFT_GRAY_7,
    TFT_GRAY_8,  TFT_GRAY_9,  TFT_GRAY_10, TFT_GRAY_11,
    TFT_GRAY_12, TFT_GRAY_13, TFT_GRAY_14, TFT_GRAY_15
  };

  int16_t screenW = epaper.width();
  int16_t screenH = epaper.height();
  int16_t bandH   = screenH / 16;        // height of each gray band

  for (uint8_t i = 0; i < 16; i++) {
    int16_t y = i * bandH;
    // Make the last band absorb any remainder pixels so the screen is fully covered
    int16_t h = (i == 15) ? (screenH - y) : bandH;
    epaper.fillRect(0, y, screenW, h, grayLevels[i]);
  }

  epaper.update();

  // Example: display a 16-level grayscale image
  // epaper.fillScreen(TFT_GRAY_15);
  // epaper.pushImage(0, 0, 800, 480, (uint16_t *)L4_GRAY);
  // epaper.update();
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
}