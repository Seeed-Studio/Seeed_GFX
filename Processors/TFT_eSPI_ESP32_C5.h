        ////////////////////////////////////////////////////
        //     TFT_eSPI ESP32-C5 optimised driver          //
        ////////////////////////////////////////////////////

// ESP32-C5 support note:
// - Avoids direct SPI register-level writes (which differ across targets)
// - Uses Arduino-ESP32 SPIClass with transactions
// - Bulk pixel writes are optimised in the paired .c implementation
// - Optional DMA path mirrors other ESP32 backends via ESP-IDF spi_master

#ifndef _TFT_eSPI_ESP32_C5H_
#define _TFT_eSPI_ESP32_C5H_

// Processor ID reported by getSetup()
#define PROCESSOR_ID 0x32C5

// Include processor specific headers (for optional DMA support)
#include "driver/spi_master.h"

// Processor specific code used by SPI bus transaction startWrite and endWrite functions
#define SET_BUS_WRITE_MODE // Not used
#define SET_BUS_READ_MODE  // Not used

// SPI busy wait hook used by common code paths (e.g. end_tft_write()).
// The ESP32-C5 backend uses Arduino SPIClass calls which are blocking, so a no-op is
// sufficient here and avoids relying on non-standard SPIClass extensions.
#ifndef SPI_BUSY_CHECK
  #define SPI_BUSY_CHECK do { } while (0)
#endif

// Code to check if DMA is busy, used by SPI bus transaction startWrite and endWrite functions
#if !defined(TFT_PARALLEL_8_BIT) && !defined(SPI_18BIT_DRIVER)
  #define ESP32_DMA
  #define DMA_BUSY_CHECK  dmaWait()
#else
  #define DMA_BUSY_CHECK
#endif

// To be safe, SUPPORT_TRANSACTIONS is assumed mandatory
#if !defined (SUPPORT_TRANSACTIONS)
  #define SUPPORT_TRANSACTIONS
#endif

// Initialise processor specific SPI functions, used by init()
#define INIT_TFT_DATA_BUS

// If smooth fonts are enabled the filing system may need to be loaded
#ifdef SMOOTH_FONT
  #define FS_NO_GLOBALS
  #include <Seeed_Arduino_FS.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the DC (TFT Data/Command or Register Select (RS))pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_DC
  #define DC_C
  #define DC_D
#else
  #define DC_C digitalWrite(TFT_DC, LOW)
  #define DC_D digitalWrite(TFT_DC, HIGH)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the CS (TFT chip select) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_CS
  #define CS_L
  #define CS_H
#else
  #define CS_L digitalWrite(TFT_CS, LOW)
  #define CS_H digitalWrite(TFT_CS, HIGH)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Make sure TFT_RD is defined if not used to avoid an error message
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_RD
  #define TFT_RD -1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the WR (TFT Write) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#ifdef TFT_WR
  #define WR_L digitalWrite(TFT_WR, LOW)
  #define WR_H digitalWrite(TFT_WR, HIGH)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the touch screen chip select pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#if !defined TOUCH_CS || (TOUCH_CS < 0)
  #define T_CS_L
  #define T_CS_H
#else
  #define T_CS_L digitalWrite(TOUCH_CS, LOW)
  #define T_CS_H digitalWrite(TOUCH_CS, HIGH)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Make sure TFT_MISO is defined if not used to avoid an error message
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_MISO
  #define TFT_MISO -1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Macros to write commands/pixel colour data to a SPI ILI948x TFT
////////////////////////////////////////////////////////////////////////////////////////
#if  defined (SPI_18BIT_DRIVER) // SPI 18-bit colour

  #define tft_Write_8(C)   spi.transfer(C)

  #define tft_Write_16(C)  spi.transfer(((C) & 0xF800)>>8); \
                           spi.transfer(((C) & 0x07E0)>>3); \
                           spi.transfer(((C) & 0x001F)<<3)

  #define tft_Write_16S(C) spi.transfer((C) & 0xF8); \
                           spi.transfer(((C) & 0xE000)>>11 | ((C) & 0x07)<<5); \
                           spi.transfer(((C) & 0x1F00)>>5)

  #define tft_Write_32(C)  spi.transfer16((C)>>16); spi.transfer16((uint16_t)(C))

  #define tft_Write_32C(C,D) spi.transfer16(C); spi.transfer16(D)

  #define tft_Write_32D(C) spi.transfer16(C); spi.transfer16(C)

#else
  #if  defined (RPI_DISPLAY_TYPE) // RPi TFT type always needs 16-bit transfers
    #define tft_Write_8(C)   spi.transfer(C); spi.transfer(C)
    #define tft_Write_16(C)  spi.transfer((uint8_t)((C)>>8));spi.transfer((uint8_t)((C)>>0))
    #define tft_Write_16S(C) spi.transfer((uint8_t)((C)>>0));spi.transfer((uint8_t)((C)>>8))

    #define tft_Write_32(C) \
      tft_Write_16((uint16_t) ((C)>>16)); \
      tft_Write_16((uint16_t) ((C)>>0))

    #define tft_Write_32C(C,D) \
      spi.transfer(0); spi.transfer((C)>>8); \
      spi.transfer(0); spi.transfer((C)>>0); \
      spi.transfer(0); spi.transfer((D)>>8); \
      spi.transfer(0); spi.transfer((D)>>0)

    #define tft_Write_32D(C) \
      spi.transfer(0); spi.transfer((C)>>8); \
      spi.transfer(0); spi.transfer((C)>>0); \
      spi.transfer(0); spi.transfer((C)>>8); \
      spi.transfer(0); spi.transfer((C)>>0)

  #else
    #ifdef __AVR__
      #define tft_Write_8(C)   {SPDR=(C); while (!(SPSR&_BV(SPIF)));}
      #define tft_Write_16(C)  tft_Write_8((uint8_t)((C)>>8));tft_Write_8((uint8_t)((C)>>0))
      #define tft_Write_16S(C) tft_Write_8((uint8_t)((C)>>0));tft_Write_8((uint8_t)((C)>>8))
    #else
      #define tft_Write_8(C)   spi.transfer(C)
      #define tft_Write_16(C)  spi.transfer16(C)
      #define tft_Write_16S(C) spi.transfer16(((C)>>8) | ((C)<<8))
    #endif

    #define tft_Write_32(C) \
      tft_Write_16((uint16_t) ((C)>>16)); \
      tft_Write_16((uint16_t) ((C)>>0))

    #define tft_Write_32C(C,D) \
      tft_Write_16((uint16_t) (C)); \
      tft_Write_16((uint16_t) (D))

    #define tft_Write_32D(C) \
      tft_Write_16((uint16_t) (C)); \
      tft_Write_16((uint16_t) (C))
  #endif
#endif

#ifndef tft_Write_16N
  #define tft_Write_16N tft_Write_16
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Macros to read from display using SPI or software SPI
////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_SDA_READ)
  #define TFT_eSPI_ENABLE_8_BIT_READ
  #define SCLK_L digitalWrite(TFT_SCLK, LOW)
  #define SCLK_H digitalWrite(TFT_SCLK, HIGH)
#else
  #define tft_Read_8() spi.transfer(0)
#endif

#endif
