#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_INIT_DELAY 0

#define TFT_NOP     0x25
#define TFT_SWRST   0x26

#define TFT_CASET   0x35
#define TFT_PASET   0x36
#define TFT_RAMWR   0x37

#define TFT_RAMRD   0x45
#define TFT_IDXRD   0x46

#define TFT_MADCTL  0x55
#define TFT_MAD_MY  0x56
#define TFT_MAD_MX  0x57
#define TFT_MAD_MV  0x58
#define TFT_MAD_ML  0x59
#define TFT_MAD_BGR 0x5A
#define TFT_MAD_MH  0x5B
#define TFT_MAD_RGB 0x5C

#define TFT_INVOFF  0x65
#define TFT_INVON   0x66

#define TFT_SET_BLOCK   0x75
#define TFT_FILL_BLOCK  0x76

#define TFT_SET_ROTATION    0x80
#define TFT_ROTATION_UP     0x81
#define TFT_ROTATION_LEFT   0x82
#define TFT_ROTATION_RIGHT  0x83
#define TFT_ROTATION_DOWN   0x84
