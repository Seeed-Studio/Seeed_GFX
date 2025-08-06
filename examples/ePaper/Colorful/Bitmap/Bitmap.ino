/*This is a 6-color electronic ink screen, which can only display 6 colors. 
Any other colors will be mapped to similar ones.

Here is the 6 colors you can display:
1.TFT_WHITE
2.TFT_BLACK
3.TFT_YELLOW
4.TFT_GREEN
5.TFT_BLUE
6.TFT_RED
*/
#include "TFT_eSPI.h"
#include "image.h"
#include "image_1.h"
#ifdef EPAPER_ENABLE // Only compile this code if the EPAPER_ENABLE is defined in User_Setup.h
EPaper epaper;
#endif

void setup()
{
#ifdef EPAPER_ENABLE
    epaper.begin();
    printf("init\r\n");

    epaper.update(0,0,epaper.width(),epaper.height(), (uint16_t *)gImage); // display the image1
    printf("image1\r\n");
    epaper.update(0,0,epaper.width(),epaper.height(), (uint16_t *)gImage_1); // display the image2
    printf("image2\r\n");

    epaper.fillScreen(TFT_BLUE); // change the color to BLUE
    epaper.update(); // update the display
    printf("BLUE\r\n");

    epaper.fillScreen(TFT_GREEN); // change the color to GREEN
    epaper.update(); // update the display
    printf("GREEN\r\n");

    epaper.fillScreen(TFT_YELLOW); // change the color to YELLO
    epaper.update(); // update the display
    printf("YELLO\r\n");

    epaper.sleep();
    printf("test over\r\n");
#endif
}

void loop()
{
    // put your main code here, to run repeatedly:
}
