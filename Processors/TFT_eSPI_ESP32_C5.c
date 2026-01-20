        ////////////////////////////////////////////////////
        //     TFT_eSPI ESP32-C5 optimised driver          //
        ////////////////////////////////////////////////////

    #include <string.h>
    #include <assert.h>

////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////

// Select the SPI port to use (matches existing ESP32 backend patterns)
#if !defined (TFT_PARALLEL_8_BIT)
  #ifdef USE_HSPI_PORT
    SPIClass spi = SPIClass(HSPI);
  #elif defined(USE_FSPI_PORT)
    SPIClass spi = SPIClass(FSPI);
  #else
    SPIClass& spi = SPI;
  #endif
#endif

#ifdef ESP32_DMA
  // DMA SPI handle
  spi_device_handle_t dmaHAL;
  #define DMA_CHANNEL SPI_DMA_CH_AUTO
  // ESP32-C5: use the general-purpose SPI host (SPI2)
  spi_host_device_t spi_host = SPI2_HOST;
  static bool dmaBusOwned = false;

  // Optional DMA-capable bounce buffer to avoid PSRAM / non-DMA memory issues.
  // Enable by defining TFT_DMA_BOUNCE_BUFFER in your setup.
  #if defined(TFT_DMA_BOUNCE_BUFFER) && __has_include("esp_heap_caps.h")
    #include "esp_heap_caps.h"
    static uint16_t* dmaBounce = nullptr;
    static uint32_t  dmaBouncePixels = 0;
  #endif

  // Some ESP32 targets/cores require clearing DMA conf to stop retransmission.
  #if __has_include("soc/spi_reg.h")
    #include "soc/spi_reg.h"
    static void IRAM_ATTR dma_end_callback(spi_transaction_t *spi_tx)
    {
      (void)spi_tx;
      WRITE_PERI_REG(SPI_DMA_CONF_REG(SPI_DMA_CH_AUTO), 0b11);
    }
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_SDA_READ) && !defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           tft_Read_8
** Description:             Bit bashed SPI to read bidirectional SDA line
***************************************************************************************/
uint8_t TFT_eSPI::tft_Read_8(void)
{
  uint8_t  ret = 0;

  for (uint8_t i = 0; i < 8; i++) {
    ret <<= 1;
    SCLK_L;
    if (digitalRead(TFT_MOSI)) ret |= 1;
    SCLK_H;
  }

  return ret;
}

/***************************************************************************************
** Function name:           beginSDA
** Description:             Detach SPI from pin to permit software SPI
***************************************************************************************/
void TFT_eSPI::begin_SDA_Read(void)
{
  spi.end();
}

/***************************************************************************************
** Function name:           endSDA
** Description:             Attach SPI pins after software SPI
***************************************************************************************/
void TFT_eSPI::end_SDA_Read(void)
{
  spi.begin();
}

////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){
  while (len>1) {tft_Write_32D(color); len-=2;}
  if (len) {tft_Write_16(color);}
}

void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){
  uint16_t *data = (uint16_t*)data_in;
  if(_swapBytes) {
    while (len>1) {tft_Write_16(*data); data++; tft_Write_16(*data); data++; len -=2;}
    if (len) {tft_Write_16(*data);}
    return;
  }

  while (len>1) {tft_Write_16S(*data); data++; tft_Write_16S(*data); data++; len -=2;}
  if (len) {tft_Write_16S(*data);}
}

void TFT_eSPI::busDir(uint32_t mask, uint8_t mode)
{
  (void)mask;
  pinMode(TFT_D0, mode);
  pinMode(TFT_D1, mode);
  pinMode(TFT_D2, mode);
  pinMode(TFT_D3, mode);
  pinMode(TFT_D4, mode);
  pinMode(TFT_D5, mode);
  pinMode(TFT_D6, mode);
  pinMode(TFT_D7, mode);
}

void TFT_eSPI::gpioMode(uint8_t gpio, uint8_t mode)
{
  (void)gpio;
  (void)mode;
}

uint8_t TFT_eSPI::readByte(void)
{
  uint8_t b = 0;

  busDir(0, INPUT);
  digitalWrite(TFT_RD, LOW);

  b |= digitalRead(TFT_D0) << 0;
  b |= digitalRead(TFT_D1) << 1;
  b |= digitalRead(TFT_D2) << 2;
  b |= digitalRead(TFT_D3) << 3;
  b |= digitalRead(TFT_D4) << 4;
  b |= digitalRead(TFT_D5) << 5;
  b |= digitalRead(TFT_D6) << 6;
  b |= digitalRead(TFT_D7) << 7;

  digitalWrite(TFT_RD, HIGH);
  busDir(0, OUTPUT);

  return b;
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined (RPI_WRITE_STROBE)
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){
  if(len) { tft_Write_16(color); len--; }
  while(len--) {WR_L; WR_H;}
}

void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;

  if (_swapBytes) while ( len-- ) {tft_Write_16S(*data); data++;}
  else while ( len-- ) {tft_Write_16(*data); data++;}
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined (SPI_18BIT_DRIVER)
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  uint8_t r = (color & 0xF800)>>8;
  uint8_t g = (color & 0x07E0)>>3;
  uint8_t b = (color & 0x001F)<<3;

  while ( len-- ) {tft_Write_8(r); tft_Write_8(g); tft_Write_8(b);}
}

void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

  uint16_t *data = (uint16_t*)data_in;
  if (_swapBytes) {
    while ( len-- ) {
      uint16_t color = *data >> 8 | *data << 8;
      tft_Write_8((color & 0xF800)>>8);
      tft_Write_8((color & 0x07E0)>>3);
      tft_Write_8((color & 0x001F)<<3);
      data++;
    }
  }
  else {
    while ( len-- ) {
      tft_Write_8((*data & 0xF800)>>8);
      tft_Write_8((*data & 0x07E0)>>3);
      tft_Write_8((*data & 0x001F)<<3);
      data++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
#else // Standard SPI 16-bit colour TFT
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock
** Description:             Write a block of pixels of the same colour
**                          Optimised for ESP32-C5 by batching writes.
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){

#if defined(ESP32) && !defined(RPI_DISPLAY_TYPE)
  if (!len) return;

  // Write in small batches to reduce per-call overhead.
  // 64 bytes = 32 pixels
  uint8_t buf[64];
  const uint8_t hi = (uint8_t)(color >> 8);
  const uint8_t lo = (uint8_t)(color);

  while (len) {
    uint32_t chunk = len;
    if (chunk > (sizeof(buf) / 2)) chunk = (sizeof(buf) / 2);

    for (uint32_t i = 0; i < chunk; i++) {
      buf[i * 2 + 0] = hi;
      buf[i * 2 + 1] = lo;
    }

    spi.writeBytes(buf, chunk * 2);
    len -= chunk;
  }
#else
  while ( len-- ) {tft_Write_16(color);}
#endif
}

/***************************************************************************************
** Function name:           pushPixels
** Description:             Write a sequence of pixels
**                          Optimised for ESP32-C5 by batching writes.
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

#if defined(ESP32) && !defined(RPI_DISPLAY_TYPE)
  uint16_t *data = (uint16_t*)data_in;

  // 64 bytes = 32 pixels
  uint8_t buf[64];

  while (len) {
    uint32_t chunk = len;
    if (chunk > (sizeof(buf) / 2)) chunk = (sizeof(buf) / 2);

    if (_swapBytes) {
      for (uint32_t i = 0; i < chunk; i++) {
        uint16_t v = data[i];
        buf[i * 2 + 0] = (uint8_t)(v >> 8);
        buf[i * 2 + 1] = (uint8_t)(v);
      }
    }
    else {
      for (uint32_t i = 0; i < chunk; i++) {
        uint16_t v = (uint16_t)(data[i] >> 8 | data[i] << 8);
        buf[i * 2 + 0] = (uint8_t)(v >> 8);
        buf[i * 2 + 1] = (uint8_t)(v);
      }
    }

    spi.writeBytes(buf, chunk * 2);
    data += chunk;
    len  -= chunk;
  }
#else
  uint16_t *data = (uint16_t*)data_in;

  if (_swapBytes) while ( len-- ) {tft_Write_16(*data); data++;}
  else while ( len-- ) {tft_Write_16S(*data); data++;}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//                                DMA FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
#if defined (ESP32_DMA) && !defined (TFT_PARALLEL_8_BIT) //       DMA FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           dmaBusy
** Description:             Check if DMA is busy
***************************************************************************************/
bool TFT_eSPI::dmaBusy(void)
{
  if (!DMA_Enabled || !spiBusyCheck) return false;

  spi_transaction_t *rtrans;
  esp_err_t ret;
  uint8_t checks = spiBusyCheck;
  for (int i = 0; i < checks; ++i)
  {
    ret = spi_device_get_trans_result(dmaHAL, &rtrans, 0);
    if (ret == ESP_OK) spiBusyCheck--;
  }

  if (spiBusyCheck == 0) return false;
  return true;
}


/***************************************************************************************
** Function name:           dmaWait
** Description:             Wait until DMA is over (blocking!)
***************************************************************************************/
void TFT_eSPI::dmaWait(void)
{
  if (!DMA_Enabled || !spiBusyCheck) return;
  spi_transaction_t *rtrans;
  esp_err_t ret;
  for (int i = 0; i < spiBusyCheck; ++i)
  {
    ret = spi_device_get_trans_result(dmaHAL, &rtrans, portMAX_DELAY);
    assert(ret == ESP_OK);
  }
  spiBusyCheck = 0;
}


/***************************************************************************************
** Function name:           pushPixelsDMA
** Description:             Push pixels to TFT (len must be less than 32767)
***************************************************************************************/
// This will byte swap the original image if setSwapBytes(true) was called by sketch.
void TFT_eSPI::pushPixelsDMA(uint16_t* image, uint32_t len)
{
  if ((len == 0) || (!DMA_Enabled)) return;

  dmaWait();

  // Make sure any prior command/address bytes are fully sent and that the TFT is in
  // data mode before starting the DMA pixel stream.
  SPI_BUSY_CHECK;
  DC_D;

  // NOTE: Do not modify the source image buffer in-place.

  esp_err_t ret;
  static spi_transaction_t trans;

  memset(&trans, 0, sizeof(spi_transaction_t));

  trans.user = (void *)1;

  #if defined(TFT_DMA_BOUNCE_BUFFER) && __has_include("esp_heap_caps.h")
    if (dmaBouncePixels < len)
    {
      if (dmaBounce) { heap_caps_free(dmaBounce); dmaBounce = nullptr; dmaBouncePixels = 0; }
      dmaBounce = (uint16_t*)heap_caps_malloc(len * sizeof(uint16_t), MALLOC_CAP_DMA);
      if (!dmaBounce) {
        // Fallback to non-DMA path if we cannot allocate DMA-capable memory
        pushPixels(image, len);
        return;
      }
      dmaBouncePixels = len;
    }
    if (_swapBytes) {
      for (uint32_t i = 0; i < len; i++) dmaBounce[i] = (uint16_t)(image[i] << 8 | image[i] >> 8);
    } else {
      memcpy(dmaBounce, image, len * sizeof(uint16_t));
    }
    trans.tx_buffer = dmaBounce;
  #else
    trans.tx_buffer = image;
  #endif

  trans.length = len * 16;
  trans.flags = 0;

  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);

  spiBusyCheck++;
}


/***************************************************************************************
** Function name:           pushImageDMA
** Description:             Push image to a window (w*h must be less than 65536)
***************************************************************************************/
// Fixed const data assumed, will NOT clip or swap bytes
void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t const* image)
{
  if ((w == 0) || (h == 0) || (!DMA_Enabled)) return;

  uint32_t len = w*h;

  dmaWait();

  setAddrWindow(x, y, w, h);

  // Ensure window commands have completed before queuing DMA pixels.
  SPI_BUSY_CHECK;
  DC_D;

  esp_err_t ret;
  static spi_transaction_t trans;

  memset(&trans, 0, sizeof(spi_transaction_t));

  trans.user = (void *)1;
  trans.tx_buffer = image;
  trans.length = len * 16;
  trans.flags = 0;

  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);

  spiBusyCheck++;
}


/***************************************************************************************
** Function name:           pushImageDMA
** Description:             Push image to a window (w*h must be less than 65536)
***************************************************************************************/
// This will clip and also swap bytes if setSwapBytes(true) was called by sketch
void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* image, uint16_t* buffer)
{
  if ((x >= _vpW) || (y >= _vpH) || (!DMA_Enabled)) return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < _vpX) { dx = _vpX - x; dw -= dx; x = _vpX; }
  if (y < _vpY) { dy = _vpY - y; dh -= dy; y = _vpY; }

  if ((x + dw) > _vpW ) dw = _vpW - x;
  if ((y + dh) > _vpH ) dh = _vpH - y;

  if (dw < 1 || dh < 1) return;

  uint32_t len = dw*dh;

  if (buffer == nullptr) {
    buffer = image;
    dmaWait();
  }

  if ( (dw != w) || (dh != h) ) {
    if(_swapBytes) {
      for (int32_t yb = 0; yb < dh; yb++) {
        for (int32_t xb = 0; xb < dw; xb++) {
          uint32_t src = xb + dx + w * (yb + dy);
          (buffer[xb + yb * dw] = image[src] << 8 | image[src] >> 8);
        }
      }
    }
    else {
      for (int32_t yb = 0; yb < dh; yb++) {
        memcpy((uint8_t*) (buffer + yb * dw), (uint8_t*) (image + dx + w * (yb + dy)), dw << 1);
      }
    }
  }
  else if (buffer != image || _swapBytes) {
    if(_swapBytes) {
      for (uint32_t i = 0; i < len; i++) (buffer[i] = image[i] << 8 | image[i] >> 8);
    }
    else {
      memcpy(buffer, image, len*2);
    }
  }

  if (spiBusyCheck) dmaWait();

  setAddrWindow(x, y, dw, dh);

  // Ensure window commands have completed before queuing DMA pixels.
  SPI_BUSY_CHECK;
  DC_D;

  esp_err_t ret;
  static spi_transaction_t trans;

  memset(&trans, 0, sizeof(spi_transaction_t));

  trans.user = (void *)1;
  trans.tx_buffer = buffer;
  trans.length = len * 16;
  trans.flags = 0;

  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);

  spiBusyCheck++;
}

////////////////////////////////////////////////////////////////////////////////////////
// Processor specific DMA initialisation
////////////////////////////////////////////////////////////////////////////////////////

static void IRAM_ATTR dc_callback(spi_transaction_t *spi_tx)
{
  if ((bool)spi_tx->user) {DC_D;}
  else {DC_C;}
}


/***************************************************************************************
** Function name:           initDMA
** Description:             Initialise the DMA engine - returns true if init OK
***************************************************************************************/
bool TFT_eSPI::initDMA(bool ctrl_cs)
{
  if (DMA_Enabled) return false;

  esp_err_t ret;
  spi_bus_config_t buscfg = {
    .mosi_io_num = TFT_MOSI,
    .miso_io_num = TFT_MISO,
    .sclk_io_num = TFT_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 8,
    .flags = 0,
    .intr_flags = 0
  };

  int8_t pin = -1;
  if (ctrl_cs) pin = TFT_CS;

  spi_device_interface_config_t devcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = TFT_SPI_MODE,
    .duty_cycle_pos = 0,
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = SPI_FREQUENCY,
    .input_delay_ns = 0,
    .spics_io_num = pin,
    .flags = SPI_DEVICE_NO_DUMMY,
    .queue_size = 1,
    .pre_cb = 0,
    #if __has_include("soc/spi_reg.h")
      .post_cb = dma_end_callback
    #else
      .post_cb = 0
    #endif
  };

  // In Arduino-ESP32 3.x, SPIClass may have already initialised the bus.
  // Treat ESP_ERR_INVALID_STATE as success and avoid freeing the bus later.
  ret = spi_bus_initialize(spi_host, &buscfg, DMA_CHANNEL);
  if (ret == ESP_OK) {
    dmaBusOwned = true;
  }
  else if (ret == ESP_ERR_INVALID_STATE) {
    dmaBusOwned = false;
  }
  else {
    return false;
  }
  ret = spi_bus_add_device(spi_host, &devcfg, &dmaHAL);
  if (ret != ESP_OK) {
    if (dmaBusOwned) {
      spi_bus_free(spi_host);
      dmaBusOwned = false;
    }
    return false;
  }

  DMA_Enabled = true;
  spiBusyCheck = 0;
  return true;
}


/***************************************************************************************
** Function name:           deInitDMA
** Description:             Disconnect the DMA engine from SPI
***************************************************************************************/
void TFT_eSPI::deInitDMA(void)
{
  if (!DMA_Enabled) return;
  spi_bus_remove_device(dmaHAL);
  if (dmaBusOwned) {
    spi_bus_free(spi_host);
    dmaBusOwned = false;
  }
  DMA_Enabled = false;
}

////////////////////////////////////////////////////////////////////////////////////////
#endif // End of DMA FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////
