#include "Extensions/EspidfSPI.h"

#if defined(USE_ESPIDF_SPI_WRAPPER)

#include <cstring>

EspidfSPI::EspidfSPI()
    : _host(determineHost()),
      _dmaChan(SPI_DMA_CH_AUTO),
      _busInitialised(false),
      _deviceCount(0),
      _active(nullptr),
      _lastRequestedFreq(0),
      _pinSCK(-1),
      _pinMISO(-1),
      _pinMOSI(-1),
      _pinSS(-1) {
  memset(&_buscfg, 0, sizeof(_buscfg));
}

EspidfSPI::~EspidfSPI() {
  end();
}

spi_host_device_t EspidfSPI::determineHost() const {
#if defined(CONFIG_IDF_TARGET_ESP32)
  #if defined(USE_HSPI_PORT)
    return HSPI_HOST;
  #elif defined(USE_FSPI_PORT)
    return SPI_HOST;
  #else
    return VSPI_HOST;
  #endif
#else
  #if defined(USE_HSPI_PORT)
    return SPI3_HOST;
  #else
    return SPI2_HOST;
  #endif
#endif
}

esp_err_t EspidfSPI::ensureBus() {
  if (_busInitialised) {
    return ESP_OK;
  }

  _buscfg.mosi_io_num = _pinMOSI;
  _buscfg.miso_io_num = _pinMISO;
  _buscfg.sclk_io_num = _pinSCK;
  _buscfg.quadwp_io_num = -1;
  _buscfg.quadhd_io_num = -1;
  _buscfg.max_transfer_sz = 4096;
  _buscfg.flags = 0;
  _buscfg.intr_flags = 0;

  esp_err_t err = spi_bus_initialize(_host, &_buscfg, _dmaChan);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "Failed to init SPI bus (%d)", err);
    return err;
  }

  _busInitialised = true;
  return ESP_OK;
}

void EspidfSPI::releaseDevices() {
  for (size_t i = 0; i < _deviceCount; ++i) {
    if (_devices[i].handle) {
      spi_bus_remove_device(_devices[i].handle);
      _devices[i].handle = nullptr;
    }
  }
  _deviceCount = 0;
}

void EspidfSPI::begin() {
  begin(_pinSCK, _pinMISO, _pinMOSI, _pinSS);
}

void EspidfSPI::begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss) {
  if (sck >= 0) _pinSCK = sck;
  if (miso >= 0) _pinMISO = miso;
  if (mosi >= 0) _pinMOSI = mosi;
  if (ss >= 0) _pinSS = ss;
  ensureBus();
}

void EspidfSPI::end() {
  if (!_busInitialised) return;

  if (_active) {
    spi_device_release_bus(_active);
    _active = nullptr;
  }

  releaseDevices();
  spi_bus_free(_host);
  _busInitialised = false;
}

esp_err_t EspidfSPI::ensureDevice(uint32_t freq, uint8_t mode, uint8_t bitOrder, spi_device_handle_t *outHandle) {
  if (freq == 0) {
    freq = _lastRequestedFreq ? _lastRequestedFreq : SPI_MASTER_FREQ_8M;
  }

  _lastRequestedFreq = freq;

  for (size_t i = 0; i < _deviceCount; ++i) {
    DeviceEntry &entry = _devices[i];
    if (entry.frequency == freq && entry.mode == mode && entry.bitOrder == bitOrder) {
      *outHandle = entry.handle;
      return ESP_OK;
    }
  }

  if (_deviceCount >= kMaxDevices) {
    // Reuse the oldest slot
    spi_bus_remove_device(_devices[0].handle);
    for (size_t i = 1; i < _deviceCount; ++i) {
      _devices[i - 1] = _devices[i];
    }
    _deviceCount--;
  }

  spi_device_interface_config_t devcfg = {};
  devcfg.command_bits = 0;
  devcfg.address_bits = 0;
  devcfg.dummy_bits = 0;
  devcfg.mode = mode;
  devcfg.clock_speed_hz = static_cast<int>(freq);
  devcfg.spics_io_num = -1;
  devcfg.queue_size = 3;
  devcfg.flags = 0;
  if (bitOrder == LSBFIRST) {
    devcfg.flags |= SPI_DEVICE_BIT_LSBFIRST;
  }

  esp_err_t err = spi_bus_add_device(_host, &devcfg, outHandle);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "Failed to add SPI device (%d)", err);
    return err;
  }

  _devices[_deviceCount].handle = *outHandle;
  _devices[_deviceCount].frequency = freq;
  _devices[_deviceCount].mode = mode;
  _devices[_deviceCount].bitOrder = bitOrder;
  _deviceCount++;

  return ESP_OK;
}

void EspidfSPI::beginTransaction(SPISettings settings) {
  if (!settings._clock) {
    settings = SPISettings(_lastRequestedFreq ? _lastRequestedFreq : SPI_MASTER_FREQ_8M, settings._bitOrder, settings._dataMode);
  }

  if (ensureBus() != ESP_OK) {
    return;
  }

  spi_device_handle_t handle = nullptr;
  if (ensureDevice(settings._clock, settings._dataMode, settings._bitOrder, &handle) != ESP_OK) {
    return;
  }

  esp_err_t err = spi_device_acquire_bus(handle, portMAX_DELAY);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "Failed to acquire SPI bus (%d)", err);
    return;
  }

  _active = handle;
}

void EspidfSPI::endTransaction() {
  if (_active) {
    spi_device_release_bus(_active);
    _active = nullptr;
  }
}

void EspidfSPI::setFrequency(uint32_t freq) {
  _lastRequestedFreq = freq;
}

void EspidfSPI::setHwCs(int use) {
  (void)use;
  // Not used, manual CS management in TFT_eSPI
}

void EspidfSPI::pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss) {
  if (_busInitialised) {
    return; // Pins cannot be changed without re-initialising bus
  }
  if (sck >= 0) _pinSCK = sck;
  if (miso >= 0) _pinMISO = miso;
  if (mosi >= 0) _pinMOSI = mosi;
  if (ss >= 0) _pinSS = ss;
}

uint8_t EspidfSPI::transfer(uint8_t data) {
  if (!_active) {
    return 0;
  }
  spi_transaction_t trans = {};
  trans.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  trans.length = 8;
  trans.tx_data[0] = data;
  if (spi_device_polling_transmit(_active, &trans) != ESP_OK) {
    return 0;
  }
  return trans.rx_data[0];
}

uint16_t EspidfSPI::transfer16(uint16_t data) {
  if (!_active) {
    return 0;
  }
  spi_transaction_t trans = {};
  trans.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  trans.length = 16;
  trans.tx_data[0] = static_cast<uint8_t>(data >> 8);
  trans.tx_data[1] = static_cast<uint8_t>(data & 0xFF);
  if (spi_device_polling_transmit(_active, &trans) != ESP_OK) {
    return 0;
  }
  return static_cast<uint16_t>(trans.rx_data[0] << 8 | trans.rx_data[1]);
}

void EspidfSPI::write32(uint32_t data) {
  if (!_active) {
    return;
  }
  spi_transaction_t trans = {};
  trans.flags = SPI_TRANS_USE_TXDATA;
  trans.length = 32;
  trans.tx_data[0] = static_cast<uint8_t>(data >> 24);
  trans.tx_data[1] = static_cast<uint8_t>((data >> 16) & 0xFF);
  trans.tx_data[2] = static_cast<uint8_t>((data >> 8) & 0xFF);
  trans.tx_data[3] = static_cast<uint8_t>(data & 0xFF);
  spi_device_polling_transmit(_active, &trans);
}

void EspidfSPI::sendBuffer(const uint8_t *data, size_t len) {
  if (!_active || data == nullptr || len == 0) {
    return;
  }
  spi_transaction_t trans = {};
  trans.length = len * 8;
  trans.tx_buffer = data;
  spi_device_polling_transmit(_active, &trans);
}

void EspidfSPI::writePattern(const uint8_t *data, size_t len, uint8_t repeat) {
  if (!data || !len || !repeat) {
    return;
  }
  for (uint8_t i = 0; i < repeat; ++i) {
    sendBuffer(data, len);
  }
}

#endif // USE_ESPIDF_SPI_WRAPPER
