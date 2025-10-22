#include "TFT_eSPI.h"

class EPaper : public TFT_eSprite
{
public:
    explicit EPaper();

    void begin(uint8_t tc = TAB_COLOUR);

    void update();
    void update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
#ifdef  USE_MUTIGRAY_EPAPER
    void update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data, uint8_t grayLevel);
#endif
    void sleep();
    void wake();

private:
    bool _sleep;
};

