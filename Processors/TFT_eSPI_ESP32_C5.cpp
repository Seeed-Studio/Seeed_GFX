// Wrapper to ensure the ESP32-C5 backend is compiled as C++ (Arduino builds compile .cpp
// units as C++ while this project includes processor sources from TFT_eSPI.cpp).

#include "TFT_eSPI_ESP32_C5.c"
