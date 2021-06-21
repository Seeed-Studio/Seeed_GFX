
writecommand(TFT_SET_ROTATION);

switch (m)
{
    case 0:
        tft_Write_8(TFT_ROTATION_UP);
        _width  = 800;
        _height = 480;
    break;
    case 1:
        tft_Write_8(TFT_ROTATION_LEFT);
        _width  = 480;
        _height = 800;
    break;
    case 2:
        tft_Write_8(TFT_ROTATION_RIGHT);
        _width  = 480;
        _height = 800;
    break;
    case 3:
        tft_Write_8(TFT_ROTATION_DOWN);
        _width  = 800;
        _height = 480;
    break;
    default:
        tft_Write_8(TFT_ROTATION_UP);
        _width  = 800;
        _height = 480;
    break;
}