#if defined(USE_XIAO_TFT_DISPLAY_BOARD)
#define TFT_BL D6
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO -1
#define TFT_MOSI D10
#define TFT_SCLK D8
#define TFT_CS   D7
#define TFT_DC   D16
#define TFT_RST  D11

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
