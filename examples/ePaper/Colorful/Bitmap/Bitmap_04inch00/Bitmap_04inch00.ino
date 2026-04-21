/*
 * Supported Colors:
 * - TFT_WHITE
 * - TFT_BLACK
 * - TFT_YELLOW
 * - TFT_GREEN
 * - TFT_BLUE
 * - TFT_RED
 */

#include "TFT_eSPI.h"
#include "image.h"

#ifdef EPAPER_ENABLE
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
  Serial.begin(115200);
  delay(2000);
  Serial.println("4.0\" Colorful E-Paper Bitmap Display Example");
  
  epaper.begin();
  
  // Clear screen to white
  epaper.fillScreen(TFT_WHITE);
  epaper.update();
  delay(1000);
  
  // Display color bitmap image using pushImage API
  // pushImage(x, y, width, height, image_data)
  epaper.pushImage(0, 0, 400, 600, (uint16_t *)gImage_4inch0);
  epaper.update();
  
  Serial.println("Color bitmap displayed successfully");
  
  // Put display to sleep to save power
  epaper.sleep();
#else
  Serial.begin(115200);
  Serial.println("EPAPER_ENABLE not defined. Please select the correct setup file.");
#endif
}

void loop()
{
  // Nothing to do here
}
