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
  Serial.println("4.26\" E-Paper Bitmap Display Example");
  
  epaper.begin();
  
  // Clear screen to white
  epaper.fillScreen(TFT_WHITE);
  epaper.update();
  delay(1000);
  
  // Display bitmap image using drawBitmap API
  // drawBitmap(x, y, bitmap_data, width, height, color)
  epaper.drawBitmap(0, 0, gImage_4inch26, 800, 480, TFT_BLACK);
  epaper.update();
  
  Serial.println("Bitmap displayed successfully");
  
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
