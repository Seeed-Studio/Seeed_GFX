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
#include <cstdint>

#ifdef EPAPER_ENABLE
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
  Serial.begin(115200);
  delay(2000);
  Serial.println("7.09\" Colorful E-Paper Bitmap Display Example");

  epaper.begin();

  // Clear screen to white
  epaper.fillScreen(TFT_WHITE);
  epaper.update();
  delay(1000);

  // Display color bitmap image using pushImage API
  // pushImage(x, y, width, height, image_data)
  // The panel is natively portrait 1200x1600 and the asset is stored
  // portrait (landscape composition): hold the glass landscape to view it,
  // same convention as the 13.3" Bitmap example.
  epaper.pushImage(0, 0, 1200, 1600, (uint16_t *)gImage_7inch09);
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
