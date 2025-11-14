EPaper::EPaper() : _sleep(false), _entemp(true), _temp(16.00), _humi(50.00), TFT_eSprite(this)
{
    setColorDepth(EPD_COLOR_DEPTH);
    createSprite(_width, _height, 1);
    //createPalette(cmap, 16);
}

void EPaper::begin(uint8_t tc)
{
      
    setBitmapColor(1, 0);
    setTextFont(1);
    setTextColor(TFT_BLACK, TFT_WHITE, true);
    init(tc);
//     fillSprite(1);
// #ifdef EPD_HORIZONTAL_MIRROR
//     EPD_PUSH_OLD_COLORS_FLIP(_width, _height, _img8);
//     fillSprite(0);
//     EPD_PUSH_NEW_COLORS_FLIP(_width, _height, _img8);
// #else
//     EPD_PUSH_OLD_COLORS(_width, _height, _img8);
//     fillSprite(0);
//     EPD_PUSH_NEW_COLORS(_width, _height, _img8);
// #endif
//     EPD_UPDATE();
    EPD_WAKEUP();
}

 void EPaper::drawPixel(int32_t x, int32_t y, uint32_t color, uint8_t bpp)
 {
    _img8[y * (_width / (8 / bpp)) + (x / (8 / bpp))] = color;
 }

void EPaper::update()
{
    wake();
    EPD_SET_WINDOW(0, 0, (_width - 1), (_height - 1));
#ifdef TCON_ENABLE    
    size_t total_bytes = (_width * _height + 7) / 8;
    for (size_t i = 0; i < total_bytes; i++)
    {
        _img8[i] =~_img8[i];  
    }
#endif
#ifdef EPD_HORIZONTAL_MIRROR
    EPD_PUSH_NEW_COLORS_FLIP(_width, _height, _img8);
#else
    EPD_PUSH_NEW_COLORS(_width, _height, _img8);
#endif
    EPD_UPDATE();
    sleep();
}



void EPaper::update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data)
{
    if (_sleep)
    {
        EPD_WAKEUP();
        _sleep = false;
    }
    uint8_t *p = (uint8_t *)data;
    // only support x, y multiple of 8 (floor)
    x = (x / 8) * 8;
    y = (y / 8) * 8;


    if(_bpp == 4)
     pushImage(x, y, w, h / 2 , (uint16_t *)p);
    else
     pushImage(x, y, w, h , (uint16_t *)p);
    EPD_SET_WINDOW(x, y, (x + w - 1), (y + h - 1));
#ifdef EPD_HORIZONTAL_MIRROR
    EPD_PUSH_NEW_COLORS_FLIP(w, h, p);
#else
    EPD_PUSH_NEW_COLORS(w, h, p);
#endif
    EPD_UPDATE();
    sleep();
}

#ifdef USE_MUTIGRAY_EPAPER
void EPaper::update(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data, uint8_t grayLevel)
{
    if (_sleep)
    {
        EPD_WAKEUP();
        _sleep = false;
    }
    uint8_t *p = (uint8_t *)data;
    x = (x / 8) * 8;
    y = (y / 8) * 8;
    pushImage(x, y, w, h , (uint16_t *)p);
    EPD_SET_WINDOW(x, y, (x + w - 1), (y + h - 1));
#ifdef EPD_HORIZONTAL_MIRROR
    EPD_PUSH_NEW_GRAY_COLORS_FLIP(w, h, p, grayLevel);
#else
    EPD_PUSH_NEW_GRAY_COLORS(w, h, p, grayLevel);
#endif
    EPD_UPDATE();
    sleep();
}

void EPaper::updateGray()
{
    wake();
    EPD_SET_WINDOW(0, 0, (_width - 1), (_height - 1));
    
    // Push the sprite buffer to screen using gray mode
#ifdef EPD_HORIZONTAL_MIRROR
    EPD_PUSH_NEW_GRAY_COLORS_FLIP(_width, _height, _img8, GRAY_LEVEL4);
#else
    EPD_PUSH_NEW_GRAY_COLORS(_width, _height, _img8, GRAY_LEVEL4);
#endif
    EPD_UPDATE();
    sleep();
}

#endif 


void EPaper::sleep()
{
    if (_sleep)
        return;
    EPD_SLEEP();
    _sleep = true;
}

void EPaper::wake()
{
    if (!_sleep)
        return;
    if(_entemp)
        EPD_SET_TEMP(_temp);
    EPD_WAKEUP();
    _sleep = false;
}

void  EPaper::setTemp(GetTempCallback callback)
{
    _temp = callback();
    EPD_SET_TEMP(_temp);
}
float EPaper::getTemp()
{
    return _temp;
}
void  EPaper::setHumi(GetHumiCallback callback)
{
    _humi = callback();
}
float EPaper::getHumi()
{
    return _humi;
}
