EPaper::EPaper() : _sleep(false), _entemp(true), _temp(16.00), _humi(50.00), _outputRotation(0), TFT_eSprite(this)
{
    setColorDepth(EPD_COLOR_DEPTH);
    setCanvasSize(_width, _height);
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

 void EPaper::drawBufferPixel(int32_t x, int32_t y, uint32_t color, uint8_t bpp)
 {
    _img8[y * (_width / (8 / bpp)) + (x / (8 / bpp))] = color;
 }

void EPaper::update()
{
    wake();
    uint16_t wx1 = 0;
    uint16_t wy1 = 0;
    uint16_t wx2 = _width - 1;
    uint16_t wy2 = _height - 1;
    EPD_SET_WINDOW(wx1, wy1, wx2, wy2);

    bool rotated = ((_outputRotation & 0x03) != 0);
    bool pushed = true;

    if (rotated) {
        pushed = pushRotatedArea(wx1, wy1, _width, _height);
        EPD_SET_WINDOW(wx1, wy1, wx2, wy2);
    }

    if (!rotated || !pushed) {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_COLORS_FLIP(_width, _height, _img8);
#else
        EPD_PUSH_NEW_COLORS(_width, _height, _img8);
#endif
    }
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
    uint16_t wx2 = x + w - 1;
    uint16_t wy2 = y + h - 1;
    EPD_SET_WINDOW(x, y, wx2, wy2);

    bool rotated = ((_outputRotation & 0x03) != 0);
    bool pushed = true;

    if (rotated) {
        pushed = pushRotatedArea(x, y, w, h);
        EPD_SET_WINDOW(x, y, wx2, wy2);
    } else {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_COLORS_FLIP(w, h, p);
#else
        EPD_PUSH_NEW_COLORS(w, h, p);
#endif
    }

    if (rotated && !pushed) {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_COLORS_FLIP(w, h, p);
#else
        EPD_PUSH_NEW_COLORS(w, h, p);
#endif
    }
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
    uint16_t wx2 = x + w - 1;
    uint16_t wy2 = y + h - 1;
    EPD_SET_WINDOW(x, y, wx2, wy2);

    bool rotated = ((_outputRotation & 0x03) != 0);
    bool pushed = true;

    if (rotated) {
        pushed = pushRotatedArea(x, y, w, h, true, grayLevel);
        EPD_SET_WINDOW(x, y, wx2, wy2);
    } else {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_GRAY_COLORS_FLIP(w, h, p, grayLevel);
#else
        EPD_PUSH_NEW_GRAY_COLORS(w, h, p, grayLevel);
#endif
    }

    if (rotated && !pushed) {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_GRAY_COLORS_FLIP(w, h, p, grayLevel);
#else
        EPD_PUSH_NEW_GRAY_COLORS(w, h, p, grayLevel);
#endif
    }
    EPD_UPDATE();
    sleep();
}

void EPaper::updateGray()
{
    wake();
    EPD_SET_WINDOW(0, 0, (_width - 1), (_height - 1));
    
    // Push the sprite buffer to screen using gray mode
    bool rotated = ((_outputRotation & 0x03) != 0);
    bool pushed = true;

    if (rotated) {
        pushed = pushRotatedArea(0, 0, _width, _height, true, GRAY_LEVEL4);
        EPD_SET_WINDOW(0, 0, (_width - 1), (_height - 1));
    }

    if (!rotated || !pushed) {
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_GRAY_COLORS_FLIP(_width, _height, _img8, GRAY_LEVEL4);
#else
        EPD_PUSH_NEW_GRAY_COLORS(_width, _height, _img8, GRAY_LEVEL4);
#endif
    }
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

bool EPaper::setCanvasSize(uint16_t w, uint16_t h)
{
    if (_created) deleteSprite();
    return createSprite(w, h, 1) != nullptr;
}

void EPaper::setOutputRotation(uint8_t rotation)
{
    _outputRotation = rotation & 0x03;
}

uint8_t EPaper::getOutputRotation() const
{
    return _outputRotation;
}

bool EPaper::pushRotatedArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool useGray, uint8_t grayLevel)
{
    if (!_created || w == 0 || h == 0) return false;

    size_t lineBytes = ((uint32_t)w * _bpp + 7) >> 3;
    if (lineBytes == 0) return false;

    uint8_t *line = (uint8_t *)malloc(lineBytes);
    if (line == nullptr) return false;

    for (uint16_t row = 0; row < h; ++row)
    {
        memset(line, 0, lineBytes);
        uint16_t destY = y + row;

        for (uint16_t col = 0; col < w; ++col)
        {
            int32_t srcX, srcY;
            if (!mapToSprite(x + col, destY, srcX, srcY)) continue;
            uint32_t value = readSpritePixelRaw(srcX, srcY);
            writeLinePixel(line, col, value);
        }

        EPD_SET_WINDOW(x, destY, x + w - 1, destY);
#if defined(USE_MUTIGRAY_EPAPER)
        if (useGray)
        {
        #ifdef EPD_HORIZONTAL_MIRROR
            EPD_PUSH_NEW_GRAY_COLORS_FLIP(w, 1, line, grayLevel);
        #else
            EPD_PUSH_NEW_GRAY_COLORS(w, 1, line, grayLevel);
        #endif
            continue;
        }
#endif
#ifdef EPD_HORIZONTAL_MIRROR
        EPD_PUSH_NEW_COLORS_FLIP(w, 1, line);
#else
        EPD_PUSH_NEW_COLORS(w, 1, line);
#endif
    }

    free(line);
    return true;
}

uint32_t EPaper::readSpritePixelRaw(int32_t x, int32_t y) const
{
    if (x < 0 || y < 0) return 0;

    if (_bpp == 1)
    {
        if (x >= _dwidth || y >= _dheight) return 0;
        int32_t bitIndex = x + y * _bitwidth;
        uint8_t byteValue = _img8[(bitIndex) >> 3];
        return (byteValue >> (7 - (bitIndex & 7))) & 0x01;
    }

    if (_bpp == 4)
    {
        if (x >= _dwidth || y >= _dheight) return 0;
        int32_t index = (x + y * _iwidth) >> 1;
        uint8_t data = _img4[index];
        if ((x & 0x01) == 0) return data >> 4;
        return data & 0x0F;
    }

    if (_bpp == 8)
    {
        if (x >= _dwidth || y >= _dheight) return 0;
        return _img8[x + y * _iwidth];
    }

    if (_bpp == 16)
    {
        if (x >= _dwidth || y >= _dheight) return 0;
        return _img[x + y * _iwidth];
    }

    return 0;
}

void EPaper::writeLinePixel(uint8_t *line, uint16_t column, uint32_t value) const
{
    switch (_bpp)
    {
        case 1:
        {
            uint16_t byteIndex = column >> 3;
            uint8_t bitMask = 0x80 >> (column & 0x07);
            if (value) line[byteIndex] |= bitMask;
            else line[byteIndex] &= ~bitMask;
            break;
        }
        case 4:
        {
            uint16_t byteIndex = column >> 1;
            uint8_t nibble = (uint8_t)(value & 0x0F);
            if ((column & 0x01) == 0)
            {
                line[byteIndex] = (uint8_t)((nibble << 4) | (line[byteIndex] & 0x0F));
            }
            else
            {
                line[byteIndex] = (uint8_t)((line[byteIndex] & 0xF0) | nibble);
            }
            break;
        }
        case 8:
        {
            line[column] = (uint8_t)(value & 0xFF);
            break;
        }
        case 16:
        {
            uint16_t *dest = (uint16_t *)line;
            dest[column] = (uint16_t)value;
            break;
        }
        default:
            break;
    }
}

bool EPaper::mapToSprite(int32_t destX, int32_t destY, int32_t &srcX, int32_t &srcY) const
{
    int32_t sw = width();
    int32_t sh = height();

    switch (_outputRotation & 0x03)
    {
        case 0:
            srcX = destX;
            srcY = destY;
            break;
        case 1:
            srcX = destY;
            srcY = sh - destX - 1;
            break;
        case 2:
            srcX = sw - destX - 1;
            srcY = sh - destY - 1;
            break;
        default:
            srcX = sw - destY - 1;
            srcY = destX;
            break;
    }

    if (srcX < 0 || srcY < 0) return false;
    if (srcX >= sw || srcY >= sh) return false;

    return true;
}
