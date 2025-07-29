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

    epaper.fillScreen(EPAPER_BLUE); // change the color to BLUE
    epaper.update(); // update the display
    printf("BLUE\r\n");

    epaper.fillScreen(EPAPER_GREEN); // change the color to GREEN
    epaper.update(); // update the display
    printf("GREEN\r\n");

    epaper.fillScreen(EPAPER_YELLO); // change the color to YELLO
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
