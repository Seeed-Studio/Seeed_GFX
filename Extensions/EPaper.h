#include "TFT_eSPI.h"

class EPaper : public TFT_eSprite
{
public:
    explicit EPaper();

    void begin(uint8_t tc = TAB_COLOUR);
    void drawBufferPixel(int32_t x, int32_t y, uint32_t color, uint8_t bpp);
    void update();
    void update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);

    bool setCanvasSize(uint16_t w, uint16_t h);
    void setOutputRotation(uint8_t rotation);
    uint8_t getOutputRotation() const;

    
#ifdef  USE_MUTIGRAY_EPAPER
    void update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data, uint8_t grayLevel);
    void updateGray();
#endif
    void sleep();
    void wake();
    
    typedef float (*GetTempCallback)();
    typedef float (*GetHumiCallback)();
    void  setTemp(GetTempCallback callback);
    float getTemp();
    void  setHumi(GetHumiCallback callback);
    float getHumi();
private:
    bool _sleep;
    bool _entemp;
    float _temp;
    float _humi;
    uint8_t _outputRotation;

    bool pushRotatedArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool useGray = false, uint8_t grayLevel = 0);
    uint32_t readSpritePixelRaw(int32_t x, int32_t y) const;
    void writeLinePixel(uint8_t *line, uint16_t column, uint32_t value) const;
    bool mapToSprite(int32_t destX, int32_t destY, int32_t &srcX, int32_t &srcY) const;
};
