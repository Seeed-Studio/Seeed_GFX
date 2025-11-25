#pragma once

#include <Arduino.h>
#include <SPI.h>

#define CONFIG_TFT_eSPI_ESPIDF

#if defined(ESP32) && defined(CONFIG_TFT_eSPI_ESPIDF)
#define USE_ESPIDF_SPI_WRAPPER
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include <freertos/FreeRTOS.h>
#include <esp_err.h>
#include <esp_log.h>

class EspidfSPI {
public:
  EspidfSPI();
  ~EspidfSPI();

  void begin();
  void begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss = -1);
  void end();

  void beginTransaction(SPISettings settings);
  void endTransaction();

  void setFrequency(uint32_t freq);
  void setHwCs(int use);
  void pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss = -1);

  uint8_t transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void write32(uint32_t data);
  void writePattern(const uint8_t *data, size_t len, uint8_t repeat);

private:
  static constexpr const char *kTag = "SeeedSPI";
  static constexpr size_t kMaxDevices = 4;

  struct DeviceEntry {
    spi_device_handle_t handle = nullptr;
    uint32_t frequency = 0;
    uint8_t mode = 0;
    uint8_t bitOrder = MSBFIRST;
  };

  spi_host_device_t determineHost() const;
  esp_err_t ensureBus();
  esp_err_t ensureDevice(uint32_t freq, uint8_t mode, uint8_t bitOrder, spi_device_handle_t *outHandle);
  void releaseDevices();
  void sendBuffer(const uint8_t *data, size_t len);

  spi_host_device_t _host;
  spi_dma_chan_t _dmaChan;
  bool _busInitialised;
  spi_bus_config_t _buscfg;
  DeviceEntry _devices[kMaxDevices];
  size_t _deviceCount;
  spi_device_handle_t _active;
  uint32_t _lastRequestedFreq;
  int8_t _pinSCK;
  int8_t _pinMISO;
  int8_t _pinMOSI;
  int8_t _pinSS;
};

extern EspidfSPI spi;

#endif // USE_ESPIDF_SPI_WRAPPER
