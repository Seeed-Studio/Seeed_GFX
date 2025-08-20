#ifndef DYNAMIC_SETUP_H
#define DYNAMIC_SETUP_H


#ifndef BOARD_SCREEN_COMBO
  #error "Hardware configuration is missing! Please create a 'driver.h' file in your sketch folder and define BOARD_SCREEN_COMBO. Use the online configurator to get the correct content."
#endif



#if BOARD_SCREEN_COMBO == 1
#include <User_Setups/Setup1_ILI9341.h> 
#elif BOARD_SCREEN_COMBO == 2
#include <User_Setups/Setup2_ST7735.h>
#elif BOARD_SCREEN_COMBO == 3
#include <User_Setups/Setup3_ILI9163.h>
#elif BOARD_SCREEN_COMBO == 4
#include <User_Setups/Setup4_S6D02A1.h> 
#elif BOARD_SCREEN_COMBO == 5
#include <User_Setups/Setup5_RPi_ILI9486.h>
#elif BOARD_SCREEN_COMBO == 6
#include <User_Setups/Setup6_RPi_Wr_ILI9486.h>
#elif BOARD_SCREEN_COMBO == 7
#include <User_Setups/Setup7_ST7735_128x128.h>
#elif BOARD_SCREEN_COMBO == 8
#include <User_Setups/Setup8_ILI9163_128x128.h>
#elif BOARD_SCREEN_COMBO == 9
#include <User_Setups/Setup9_ST7735_Overlap.h>
#elif BOARD_SCREEN_COMBO == 10
#include <User_Setups/Setup10_RPi_touch_ILI9486.h>
#elif BOARD_SCREEN_COMBO == 11
#include <User_Setups/Setup11_RPi_touch_ILI9486.h>
#elif BOARD_SCREEN_COMBO == 12
#include <User_Setups/Setup12_M5Stack_Basic_Core.h>
#elif BOARD_SCREEN_COMBO == 13
#include <User_Setups/Setup13_ILI9481_Parallel.h>
#elif BOARD_SCREEN_COMBO == 14
#include <User_Setups/Setup14_ILI9341_Parallel.h>
#elif BOARD_SCREEN_COMBO == 15
#include <User_Setups/Setup15_HX8357D.h>
#elif BOARD_SCREEN_COMBO == 16
#include <User_Setups/Setup16_ILI9488_Parallel.h>
#elif BOARD_SCREEN_COMBO == 17
#include <User_Setups/Setup17_ePaper.h>

#elif BOARD_SCREEN_COMBO == 18
#include <User_Setups/Setup18_ST7789.h> 
#elif BOARD_SCREEN_COMBO == 19
#include <User_Setups/Setup19_RM68140_Parallel.h>	
#elif BOARD_SCREEN_COMBO == 20
#include <User_Setups/Setup20_ILI9488.h>
#elif BOARD_SCREEN_COMBO == 21
#include <User_Setups/Setup21_ILI9488.h>
#elif BOARD_SCREEN_COMBO == 22
#include <User_Setups/Setup22_TTGO_T4.h>
#elif BOARD_SCREEN_COMBO == 23
#include <User_Setups/Setup23_TTGO_TM.h>
#elif BOARD_SCREEN_COMBO == 24
#include <User_Setups/Setup24_ST7789.h>
#elif BOARD_SCREEN_COMBO == 25
#include <User_Setups/Setup25_TTGO_T_Display.h>
#elif BOARD_SCREEN_COMBO == 26
#include <User_Setups/Setup26_TTGO_T_Wristband.h>
#elif BOARD_SCREEN_COMBO == 27
#include <User_Setups/Setup27_RPi_ST7796_ESP32.h> 
#elif BOARD_SCREEN_COMBO == 28
#include <User_Setups/Setup28_RPi_ST7796_ESP8266.h>

#elif BOARD_SCREEN_COMBO == 29
#include <User_Setups/Setup29_ILI9341_STM32.h>
#elif BOARD_SCREEN_COMBO == 30
#include <User_Setups/Setup30_ILI9341_Parallel_STM32.h>	
#elif BOARD_SCREEN_COMBO == 31
#include <User_Setups/Setup31_ST7796_Parallel_STM32.h>
#elif BOARD_SCREEN_COMBO == 32
#include <User_Setups/Setup32_ILI9341_STM32F103.h>

#elif BOARD_SCREEN_COMBO == 33
#include <User_Setups/Setup33_RPi_ILI9486_STM32.h>

#elif BOARD_SCREEN_COMBO == 34
#include <User_Setups/Setup34_ILI9481_Parallel_STM32.h>
#elif BOARD_SCREEN_COMBO == 35
#include <User_Setups/Setup35_ILI9341_STM32_Port_Bus.h>

#elif BOARD_SCREEN_COMBO == 36
#include <User_Setups/Setup36_RPi_touch_ST7796.h>

#elif BOARD_SCREEN_COMBO == 42
#include <User_Setups/Setup42_ILI9341_ESP32.h>
#elif BOARD_SCREEN_COMBO == 43
#include <User_Setups/Setup43_ST7735.h>	
#elif BOARD_SCREEN_COMBO == 44
#include <User_Setups/Setup44_TTGO_CameraPlus.h>
#elif BOARD_SCREEN_COMBO == 45
#include <User_Setups/Setup45_TTGO_T_Watch.h>
#elif BOARD_SCREEN_COMBO == 46
#include <User_Setups/Setup46_GC9A01_ESP32.h> 
#elif BOARD_SCREEN_COMBO == 47
#include <User_Setups/Setup47_ST7735.h>

#elif BOARD_SCREEN_COMBO == 50
#include <User_Setups/Setup50_SSD1963_Parallel.h>
#elif BOARD_SCREEN_COMBO == 51
#include <User_Setups/Setup51_LilyPi_ILI9481.h>
#elif BOARD_SCREEN_COMBO == 52
#include <User_Setups/Setup52_LilyPi_ST7796.h>

#elif BOARD_SCREEN_COMBO == 60
#include <User_Setups/Setup60_RP2040_ILI9341.h>
#elif BOARD_SCREEN_COMBO == 61
#include <User_Setups/Setup61_RP2040_ILI9341_PIO_SPI.h>
#elif BOARD_SCREEN_COMBO == 62
#include <User_Setups/Setup62_RP2040_Nano_Connect_ILI9341.h>
#elif BOARD_SCREEN_COMBO == 66
#include <User_Setups/Setup66_Seeed_XIAO_Round.h>

#elif BOARD_SCREEN_COMBO == 70
#include <User_Setups/Setup70_ESP32_S2_ILI9341.h>
#elif BOARD_SCREEN_COMBO == 71 // Remapped from 70b for simplicity
#include <User_Setups/Setup70b_ESP32_S3_ILI9341.h>
#elif BOARD_SCREEN_COMBO == 72 // Remapped from 70c
#include <User_Setups/Setup70c_ESP32_C3_ILI9341.h>
#elif BOARD_SCREEN_COMBO == 73 // Remapped from 70d
#include <User_Setups/Setup70d_ILI9488_S3_Parallel.h>
#elif BOARD_SCREEN_COMBO == 74 // Remapped from 71
#include <User_Setups/Setup71_ESP32_S2_ST7789.h>
#elif BOARD_SCREEN_COMBO == 75 // Remapped from 72
#include <User_Setups/Setup72_ESP32_ST7789_172x320.h>

#elif BOARD_SCREEN_COMBO == 100
#include <User_Setups/Setup100_RP2040_ILI9488_parallel.h>
#elif BOARD_SCREEN_COMBO == 101
#include <User_Setups/Setup101_RP2040_ILI9481_parallel.h>
#elif BOARD_SCREEN_COMBO == 102
#include <User_Setups/Setup102_RP2040_ILI9341_parallel.h>
#elif BOARD_SCREEN_COMBO == 103
#include <User_Setups/Setup103_RP2040_ILI9486_parallel.h>
#elif BOARD_SCREEN_COMBO == 104
#include <User_Setups/Setup104_RP2040_ST7796_parallel.h>
#elif BOARD_SCREEN_COMBO == 105
#include <User_Setups/Setup105_RP2040_ST7796_16bit_parallel.h>
#elif BOARD_SCREEN_COMBO == 106
#include <User_Setups/Setup106_RP2040_ILI9481_16bit_parallel.h>
#elif BOARD_SCREEN_COMBO == 107
#include <User_Setups/Setup107_RP2040_ILI9341_16bit_parallel.h>
#elif BOARD_SCREEN_COMBO == 108
#include <User_Setups/Setup108_RP2040_ST7735.h>

#elif BOARD_SCREEN_COMBO == 135
#include <User_Setups/Setup135_ST7789.h>
#elif BOARD_SCREEN_COMBO == 136
#include <User_Setups/Setup136_LilyGo_TTV.h>
#elif BOARD_SCREEN_COMBO == 137
#include <User_Setups/Setup137_LilyGo_TDisplay_RP2040.h>
#elif BOARD_SCREEN_COMBO == 138
#include <User_Setups/Setup138_Pico_Explorer_Base_RP2040_ST7789.h>

#elif BOARD_SCREEN_COMBO == 200
#include <User_Setups/Setup200_GC9A01.h>
#elif BOARD_SCREEN_COMBO == 201
#include <User_Setups/Setup201_WT32_SC01.h>
#elif BOARD_SCREEN_COMBO == 202
#include <User_Setups/Setup202_SSD1351_128.h>
#elif BOARD_SCREEN_COMBO == 203
#include <User_Setups/Setup203_ST7789.h>
#elif BOARD_SCREEN_COMBO == 204
#include <User_Setups/Setup204_ESP32_TouchDown.h>
#elif BOARD_SCREEN_COMBO == 205
#include <User_Setups/Setup205_ESP32_TouchDown_S3.h>
#elif BOARD_SCREEN_COMBO == 206
#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>
#elif BOARD_SCREEN_COMBO == 207
#include <User_Setups/Setup207_LilyGo_T_HMI.h>
#elif BOARD_SCREEN_COMBO == 209
#include <User_Setups/Setup209_LilyGo_T_Dongle_S3.h>
#elif BOARD_SCREEN_COMBO == 210
#include <User_Setups/Setup210_LilyGo_T_Embed_S3.h>
#elif BOARD_SCREEN_COMBO == 211
#include <User_Setups/Setup211_LilyGo_T_QT_Pro_S3.h>
#elif BOARD_SCREEN_COMBO == 212
#include <User_Setups/Setup212_LilyGo_T_PicoPro.h>
#elif BOARD_SCREEN_COMBO == 213
#include <User_Setups/Setup213_LilyGo_T_Beam_Shield.h>

#elif BOARD_SCREEN_COMBO == 250
#include <User_Setups/Setup250_ESP32_S3_Box_Lite.h>
#elif BOARD_SCREEN_COMBO == 251
#include <User_Setups/Setup251_ESP32_S3_Box.h>

#elif BOARD_SCREEN_COMBO == 301
#include <User_Setups/Setup301_BW16_ST7735.h>
#elif BOARD_SCREEN_COMBO == 302
#include <User_Setups/Setup302_Waveshare_ESP32S3_GC9A01.h>

#elif BOARD_SCREEN_COMBO == 500
#include <User_Setups/Setup500_Seeed_Wio_Terminal.h>
#elif BOARD_SCREEN_COMBO == 501
#include <User_Setups/Setup501_Seeed_XIAO_Round_Display.h>
#elif BOARD_SCREEN_COMBO == 502
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup502_Seeed_XIAO_EPaper_7inch5.h>
#elif BOARD_SCREEN_COMBO == 503
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup503_Seeed_XIAO_EPaper_5inch83.h>
#elif BOARD_SCREEN_COMBO == 504
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup504_Seeed_XIAO_EPaper_2inch9.h>
#elif BOARD_SCREEN_COMBO == 505
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup505_Seeed_XIAO_EPaper_1inch54.h>
#elif BOARD_SCREEN_COMBO == 506
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup506_Seeed_XIAO_EPaper_4inch26.h>
#elif BOARD_SCREEN_COMBO == 507
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup507_Seeed_XIAO_EPaper_4inch2.h>
#elif BOARD_SCREEN_COMBO == 508
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup508_Seeed_XIAO_EPaper_2inch13.h>
#elif BOARD_SCREEN_COMBO == 509
#define USE_COLORFULL_EPAPER
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup509_Seeed_XIAO_EPaper_7inch3_colorful.h>    // Setup file for Seeed XIAO with 7.3inch RGB ePaper
#elif BOARD_SCREEN_COMBO == 520
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup520_Seeed_reTerminal_E1001.h>
#elif BOARD_SCREEN_COMBO == 521
#define USE_COLORFULL_EPAPER
#define ENABLE_EPAPER_BOARD_PIN_SETUPS
#include <User_Setups/Setup521_Seeed_reTerminal_E1002.h>

#elif BOARD_SCREEN_COMBO == 666
#include <User_Setups/Setup666_XIAO_ILI9341.h>


#elif BOARD_SCREEN_COMBO == 901
#include <User_Setups/Dustin_ILI9488.h>
#elif BOARD_SCREEN_COMBO == 902
#include <User_Setups/Dustin_ST7796.h>
#elif BOARD_SCREEN_COMBO == 903
#include <User_Setups/Dustin_ILI9488_Pico.h>
#elif BOARD_SCREEN_COMBO == 904
#include <User_Setups/Dustin_ST7789_Pico.h>
#elif BOARD_SCREEN_COMBO == 905
#include <User_Setups/Dustin_GC9A01_Pico.h>
#elif BOARD_SCREEN_COMBO == 906
#include <User_Setups/Dustin_GC9A01_ESP32.h>
#elif BOARD_SCREEN_COMBO == 907
#include <User_Setups/Dustin_STT7789_ESP32.h>
#elif BOARD_SCREEN_COMBO == 908
#include <User_Setups/Dustin_ILI9341_ESP32.h>
#elif BOARD_SCREEN_COMBO == 999
#include <User_Setups/ILI9225.h>


#else
  // If BOARD_SCREEN_COMBO was defined but did not match any of the above IDs
  #error "The provided BOARD_SCREEN_COMBO ID is not a valid or supported configuration."
#endif



#endif
