#ifndef EPD_WIDTH
#define EPD_WIDTH 128
#endif

#ifndef EPD_HEIGHT
#define EPD_HEIGHT 296
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH EPD_WIDTH
#endif

#ifndef TFT_HEIGHT
#define TFT_HEIGHT EPD_HEIGHT
#endif

#define EPD_COLOR_DEPTH 4



#define EPD_NOP 0xFF     // No operation command (not supported)
#define EPD_PNLSET 0x00  // Panel setting (R00H PSR)
#define EPD_DISPON 0x04  // Power on (R04H PON)
#define EPD_DISPOFF 0x02 // Power off (R02H POF)
#define EPD_SLPIN 0x07   // Enter deep sleep (R07H DSLP)
#define EPD_SLPOUT 0xFF  // Exit sleep (not supported, requires wake-up)
#define EPD_PTLIN 0x91   // Partial display in (R91H PTIN)
#define EPD_PTLOUT 0x92  // Partial display out (R92H PTOUT)
#define EPD_PTLW 0x90    // Partial display window setting (R90H PTL)

#define TFT_SWRST 0xFF   // Software reset (not supported)
#define TFT_CASET 0xFF   // Column address setting (not supported)
#define TFT_PASET 0xFF   // Page address setting (not supported)
#define TFT_RAMWR 0x13   // Write RAM (R13H DTM2, red data)
#define TFT_RAMRD 0xFF   // Read RAM (not supported)
#define TFT_INVON 0xFF   // Display inversion on (not supported)
#define TFT_INVOFF 0xFF  // Display inversion off (not supported)
#define TFT_INIT_DELAY 0 // Initialization delay (none)

#ifdef TFT_BUSY
#define CHECK_BUSY()                   \
    do                                 \
    {                                  \
        while (!digitalRead(TFT_BUSY)) \
            ;                          \
    } while (0)
#else
#define CHECK_BUSY()
#endif

#define EPD_INIT()          \
    do                      \
    {                       \
        writecommand(0x4D);\
        writedata(0x78);\
        writecommand(0x00); \
        writedata(0x0F);\
        writedata(0x29);\
        writecommand(0x01); \
        writedata(0x07);\
        writedata(0x00);\
        writecommand(0x03); \
        writedata(0x10);\
        writedata(0x54);\
        writedata(0x44);\
        writecommand(0x06); \
        writedata(0x05);\
        writedata(0x00);\
        writedata(0x3F);\
        writedata(0x0A);\
        writedata(0x25);\
        writedata(0x12);\
        writedata(0x1A); \
        writecommand(0x50);\
        writedata(0x37);\
        writecommand(0x60); \
        writedata(0x02);\
        writedata(0x02);\
        writecommand(0x61); \
        writedata(128/256);  \ 
        writedata(128%256);  \ 
        writedata(296/256);  \   
        writedata(296%256);    \
        writecommand(0xE7);\
        writedata(0x1C);\
        writecommand(0xE3); \
        writedata(0x22);\
        writecommand(0xB4);\
        writedata(0xD0);\
        writecommand(0xB5);\
        writedata(0x03);\
        writecommand(0xE9);\
        writedata(0x01); \
        writecommand(0x30);\
        writedata(0x08);  \
        writecommand(0x04);\
        CHECK_BUSY();            \
    } while (0)

#define EPD_UPDATE()        \
    do                      \
    {                       \
        writecommand(0x12); \
        writedata(0x00);    \
        CHECK_BUSY();       \
    } while (0)

#define EPD_SLEEP()         \
    do                      \
    {                       \
        writecommand(0x02); \
        CHECK_BUSY();       \
        delay(100);         \
        writecommand(0x07); \
        writedata(0xA5);    \
    } while (0)

#define EPD_WAKEUP()                 \
    do                               \
    {                                \
        digitalWrite(TFT_RST, LOW);  \
        delay(20);                   \
        digitalWrite(TFT_RST, HIGH); \
        delay(20);                   \
        CHECK_BUSY();                \
        EPD_INIT();                  \
    } while (0)

#define EPD_SET_WINDOW(x1, y1, x2, y2)                  \
    do                                                  \
    {                                                   \
    } while (0)

#define COLOR_GET(color) ( \
    (color) == 0x00 ? 0x01 : \
    (color) == 0x01 ? 0x02 : \
    (color) == 0x02 ? 0x03 : \
    (color) == 0x03 ? 0x00 : \
    0x00 \
)

#define EPD_PUSH_NEW_COLORS(w, h, colors)   \
    do                                      \
    {                                       \
        uint16_t bytes_per_row = (w) / 2;   \
        uint8_t temp1, temp2, temp3, temp4;               \
        writecommand(0x10);                 \
        for (uint16_t row = 0; row < (h) ; row++)        \
        {                                   \
            for(uint16_t col = 0; col < bytes_per_row; col+=2)   \
            {                               \
                uint8_t b = (colors[bytes_per_row *row+col ]) ;   \
                uint8_t c = (colors[bytes_per_row *row+col + 1]) ;   \
                temp1 =  (b >> 4) & 0x0F;\
                temp2 =   b & 0x0F;\
                temp3 =  (c >> 4) & 0x0F;\
                temp4 =   c & 0x0F;\
                writedata(((COLOR_GET(temp1) <<6)|( COLOR_GET(temp2) << 4 ) |( COLOR_GET(temp3) << 2 ) |( COLOR_GET(temp4) << 0 )));\
            }                               \
        }                                   \
    } while (0)

#define EPD_PUSH_NEW_COLORS_FLIP(w, h, colors)                         \
    do                                                                 \
    {                                                                  \
        uint16_t bytes_per_row = (w) / 2;                              \
        uint8_t temp1, temp2, temp3, temp4;               \
        writecommand(0x10);                                            \
        for (uint16_t row = 0; row < (h); row++)                       \
        {                                                              \
            uint16_t start = row * bytes_per_row;                      \
            for (uint16_t col = 0; col < bytes_per_row; col+=2)         \
            {                                                          \
                uint8_t b = (colors[bytes_per_row *row + (bytes_per_row - 1 - col)]) ;\
                uint8_t c = (colors[bytes_per_row *row + (bytes_per_row - 1 - col) - 1]) ;\
                temp1 =  (b >> 4) & 0x0F;\
                temp2 =   b & 0x0F;\
                temp3 =  (c >> 4) & 0x0F;\
                temp4 =   c & 0x0F;\
                writedata(((COLOR_GET(temp1) <<6)|( COLOR_GET(temp2) << 4 ) |( COLOR_GET(temp3) << 2 ) |( COLOR_GET(temp4) << 0 )));\
            }                                                          \
        }                                                              \
    } while (0)

#define EPD_PUSH_OLD_COLORS(w, h, colors)       \
    do                                          \
    {                                           \
    } while (0)

#define EPD_PUSH_OLD_COLORS_FLIP(w, h, colors)                         \
    do                                                                 \
    {                                                                  \
                                                     \
    } while (0)

#define EPD_SET_TEMP(temp)                  \
    do                                      \
    {                                       \
    } while (0)  