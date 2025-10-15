
#ifndef EPD_WIDTH
#define EPD_WIDTH 800
#endif

#ifndef EPD_HEIGHT
#define EPD_HEIGHT 480
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH EPD_WIDTH
#endif

#ifndef TFT_HEIGHT
#define TFT_HEIGHT EPD_HEIGHT
#endif

#define EPD_COLOR_DEPTH 1

#define EPD_NOP 0xFF
#define EPD_PNLSET 0x00
#define EPD_DISPON 0x04
#define EPD_DISPOFF 0x03
#define EPD_SLPIN 0x07
#define EPD_SLPOUT 0xFF
#define EPD_PTLIN 0x91  // Partial display in
#define EPD_PTLOUT 0x92 // Partial display out
#define EPD_PTLW 0x90

#define TFT_SWRST 0xFF
#define TFT_CASET 0xFF
#define TFT_PASET 0xFF
#define TFT_RAMWR 0xFF
#define TFT_RAMRD 0xFF
#define TFT_INVON EPD_DISPON
#define TFT_INVOFF EPD_DISPOFF

#define TFT_INIT_DELAY 0

#ifdef TFT_BUSY
#define CHECK_BUSY()               \
    do                             \
    {                              \
        delay(10);                 \
        if (digitalRead(TFT_BUSY)) \
            break;                 \
    } while (true)
#else
#define CHECK_BUSY()
#endif

#define EPD_UPDATE()        \
    do                      \
    {                       \
        writecommand(0x12); \
        CHECK_BUSY();       \
    } while (0)

#define EPD_SLEEP()         \
    do                      \
    {                       \
        writecommand(0X50); \
        writedata(0xf7);    \
        writecommand(0x02); \
        CHECK_BUSY();       \
        writecommand(0x07); \
        writedata(0xA5);    \
    } while (0)

#define EPD_INIT_FULL()          \
    do                           \
    {                            \
        writecommand(0x01);      \
        writedata(0x07);         \
        writedata(0x07);         \
        writedata(0x3f);         \
        writedata(0x3f);         \
        writecommand(0x06);      \
        writedata(0x17);         \
        writedata(0x17);         \
        writedata(0x28);         \
        writedata(0x17);         \
        writecommand(0x04);      \
        delay(100);              \
        CHECK_BUSY();            \
        writecommand(0x00);      \
        writedata(0x1F);         \
        writecommand(0x61);      \
        writedata(EPD_WIDTH >> 8); \
        writedata(EPD_WIDTH & 0xFF); \
        writedata(EPD_HEIGHT >> 8); \
        writedata(EPD_HEIGHT & 0xFF); \
        writecommand(0x50);      \
        writedata(0x10);         \
        writedata(0x07);         \
    } while (0)

#define EPD_INIT_FAST()     \
    do                      \
    {                       \
        writecommand(0X00); \
        writedata(0x1F);    \
        writecommand(0X50); \
        writedata(0x10);    \
        writedata(0x07);    \
        writecommand(0x04); \
        delay(100);         \
        CHECK_BUSY();       \
        writecommand(0x06); \
        writedata(0x27);    \
        writedata(0x27);    \
        writedata(0x18);    \
        writedata(0x17);    \
        writecommand(0xE0); \
        writedata(0x02);    \
        writecommand(0xE5); \
        writedata(0x5A);    \
    } while (0)

#define EPD_INIT_PARTIAL()           \
    do                               \
    {                                \
        writecommand(0x04);          \
        delay(100);                  \
        CHECK_BUSY();                \
        writecommand(0xE0);          \
        writedata(0x02);             \
        writecommand(0xE5);          \
        writedata(0x6E);             \
        writecommand(0x00);          \
        writedata(0x1F);             \
    } while (0)

#define EPD_WAKEUP()                 \
    do                               \
    {                                \
        digitalWrite(TFT_RST, LOW);  \
        delay(10);                   \
        digitalWrite(TFT_RST, HIGH); \
        delay(10);                   \
        CHECK_BUSY();                \
        EPD_INIT_FAST();             \
    } while (0)

#define EPD_SET_WINDOW(x1, y1, x2, y2)                  \
    do                                                  \
    {                                                   \
    } while (0)

#define EPD_PUSH_NEW_COLORS(w, h, colors)   \
    do                                      \
    {                                       \
        writecommand(0x13);                 \
        for (int i = 0; i < w * h / 8; i++) \
        {                                   \
            writedata(~colors[i]);           \
        }                                   \
    } while (0)

#define EPD_PUSH_NEW_COLORS_FLIP(w, h, colors)                         \
    do                                                                 \
    {                                                                  \
        writecommand(0x13);                                            \
        uint16_t bytes_per_row = (w) / 8;                              \
        for (uint16_t row = 0; row < (h); row++)                       \
        {                                                              \
            uint16_t start = row * bytes_per_row;                      \
            for (uint16_t col = 0; col < bytes_per_row; col++)         \
            {                                                          \
                uint8_t b = colors[start + (bytes_per_row - 1 - col)]; \
                b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);             \
                b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);             \
                b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);             \
                writedata(b);                                          \
            }                                                          \
        }                                                              \
    } while (0)

#define EPD_PUSH_OLD_COLORS(w, h, colors)   \
    do                                      \
    {                                       \
        writecommand(0x10);                 \
        for (int i = 0; i < w * h / 8; i++) \
        {                                   \
            writedata(~colors[i]);           \
        }                                   \
    } while (0)

#define EPD_PUSH_OLD_COLORS_FLIP(w, h, colors)                         \
    do                                                                 \
    {                                                                  \
        writecommand(0x10);                                            \
        uint16_t bytes_per_row = (w) / 8;                              \
        for (uint16_t row = 0; row < (h); row++)                       \
        {                                                              \
            uint16_t start = row * bytes_per_row;                      \
            for (uint16_t col = 0; col < bytes_per_row; col++)         \
            {                                                          \
                uint8_t b = colors[start + (bytes_per_row - 1 - col)]; \
                b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);             \
                b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);             \
                b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);             \
                writedata(b);                                          \
            }                                                          \
        }                                                              \
    } while (0)