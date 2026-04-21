 /*
 * Supported Colors:
 * - TFT_WHITE  (White)
 * - TFT_BLACK  (Black)
 * - TFT_RED    (Red)
 * - TFT_YELLOW (Yellow)
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
  Serial.println("2.9\" BWRY E-Paper Bitmap Display Example");
  
  epaper.begin();
  
  // Clear screen to white
  epaper.fillScreen(TFT_WHITE);
  epaper.update();
  delay(1000);
  
  // Display 4-color bitmap image using pushImage API
  // pushImage(x, y, width, height, image_data)
  epaper.pushImage(0, 0, 128, 296, (uint16_t *)gImage_2inch9_BWRY);
  epaper.update();
  
  Serial.println("4-color bitmap displayed successfully");
  
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
