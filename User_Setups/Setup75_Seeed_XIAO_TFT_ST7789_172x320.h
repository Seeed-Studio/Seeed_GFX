// Seeed XIAO TFT display with ST7789 172 x 320 panel
#define USER_SETUP_ID 75

#define ST7789_DRIVER

#define TFT_RGB_ORDER TFT_BGR

#define TFT_WIDTH 172
#define TFT_HEIGHT 320

#ifdef ENABLE_TFT_BOARD_PIN_SETUPS
#include "TFT_Board_Pins_Setups.h"
#else
#define TFT_BL D6
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO -1
#define TFT_MOSI D10
#define TFT_SCLK D8
#define TFT_CS   D7
#define TFT_DC   D16
#define TFT_RST  D11
#endif

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#ifdef CONFIG_IDF_TARGET_ESP32S3
#define USE_HSPI_PORT
#endif

#include "XIAO_SPI_Frequency.h"
