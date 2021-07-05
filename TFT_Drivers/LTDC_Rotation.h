
writecommand(TFT_SET_ROTATION);

switch (m)
{
    case 0:
        tft_Write_8(TFT_ROTATION_UP);
        _width  = TFT_WIDTH;
        _height = TFT_HEIGHT;
    break;
    case 1:
        tft_Write_8(TFT_ROTATION_LEFT);
        _width  = TFT_HEIGHT;
        _height = TFT_WIDTH;
    break;
    case 2:
        tft_Write_8(TFT_ROTATION_RIGHT);
        _width  = TFT_HEIGHT;
        _height = TFT_WIDTH;
    break;
    case 3:
        tft_Write_8(TFT_ROTATION_DOWN);
        _width  = TFT_WIDTH;
        _height = TFT_HEIGHT;
    break;
    default:
        tft_Write_8(TFT_ROTATION_UP);
        _width  = TFT_WIDTH;
        _height = TFT_HEIGHT;
    break;
}