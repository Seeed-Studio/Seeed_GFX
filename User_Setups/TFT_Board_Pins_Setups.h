#if defined(USE_XIAO_TFT_DISPLAY_BOARD) //Applicable to 0.96/1.14/1.47 Inch Display Powered by XIAO nRF52840(ESP32-S3) Plus
#define TFT_BL   D18
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO -1
#define TFT_MOSI D10
#define TFT_SCLK D8
#define TFT_CS   D2
#define TFT_DC   D3
#define TFT_RST  D17

#else
// Default pins for BOARD_SCREEN_COMBO 75.
#define TFT_BL D6
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO -1
#define TFT_MOSI D10
#define TFT_SCLK D8
#define TFT_CS   D7
#define TFT_DC   D16
#define TFT_RST  D11
#endif
