#ifndef SETUP_CI_DEFAULT_H
#define SETUP_CI_DEFAULT_H

// Fallback configuration used by CI builds when no user setup is supplied.
// Provides a minimal ST7735 SPI configuration so sketches compile.

#ifndef USER_SETUP_INFO
#define USER_SETUP_INFO "CI Default Setup"
#endif

#ifndef ST7735_DRIVER
#define ST7735_DRIVER
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH 128
#endif

#ifndef TFT_HEIGHT
#define TFT_HEIGHT 160
#endif

#ifndef ST7735_REDTAB
#define ST7735_REDTAB
#endif

#ifndef LOAD_GLCD
#define LOAD_GLCD
#endif

#ifndef LOAD_FONT2
#define LOAD_FONT2
#endif

#ifndef LOAD_FONT4
#define LOAD_FONT4
#endif

#ifndef LOAD_FONT6
#define LOAD_FONT6
#endif

#ifndef LOAD_FONT7
#define LOAD_FONT7
#endif

#ifndef LOAD_FONT8
#define LOAD_FONT8
#endif

#ifndef LOAD_GFXFF
#define LOAD_GFXFF
#endif

#ifndef SMOOTH_FONT
#define SMOOTH_FONT
#endif

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 20000000
#endif

#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 10000000
#endif

#endif // SETUP_CI_DEFAULT_H
