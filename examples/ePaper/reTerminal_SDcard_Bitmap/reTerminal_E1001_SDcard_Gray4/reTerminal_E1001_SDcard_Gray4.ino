/*
 * reTerminal E1001 -- SD card image to 4-level grayscale e-paper (800x480, UC8179).
 *
 * UC8179 is a BW controller, but Seeed_GFX's waveform driver supports GRAY_LEVEL4
 * which gives 4 shades (black, dark gray, light gray, white) via initGrayMode(4).
 *
 * Pipeline mirrors online_img2bitmap_.html with screen=gray4:
 *   SD JPG/BMP/PNG -> RGB888 -> luma + gamma + Floyd-Steinberg gray4 dither
 *               -> 4bpp packed -> pushImage into the 4bpp Gray4 sprite.
 *
 * Hardware: reTerminal E1001 (XIAO ESP32-S3 + 7.5" UC8179 BW e-paper).
 *
 *           Function     ESP32-S3 GPIO
 *           SPI SCK      7   (shared with SD)
 *           SPI MOSI     9   (shared with SD)
 *           EPD CS       10
 *           EPD DC       11
 *           EPD RST      12
 *           EPD BUSY     13
 *           SD  CS       14
 *           SD  EN       16
 *           SD  DET      15
 *           SD  MISO     8   (carrier-wired but NOT in Setup520; we add it at runtime)
 *
 * Logging goes out on UART1 (GPIO43 TX, GPIO44 RX) -- the on-board USB-to-UART
 * bridge of the reTerminal carrier board is wired there. The default Arduino
 * `Serial` (USB CDC) only works when "USB CDC On Boot" is enabled, which is
 * unreliable for diagnostics on reTerminal.
 */

#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include "TFT_eSPI.h"

#include "dither.h"
#include "image_loader.h"

#ifndef EPAPER_ENABLE
#error "This example requires Setup520_Seeed_reTerminal_E1001 -- check driver.h selects BOARD_SCREEN_COMBO 520"
#endif

EPaper epaper;

// ----- pins ------------------------------------------------------------------
// SD pins are identical across all reTerminal E10xx carrier boards, except
// SD_EN: E1001/E1002/E1004 use GPIO16, E1003 uses GPIO39.
static constexpr int    PIN_SD_SCK   = 7;   // shared with e-paper SPI bus
static constexpr int    PIN_SD_MISO  = 8;   // NOT in Setup520 (TFT_MISO=-1) -- we add it for SD
static constexpr int    PIN_SD_MOSI  = 9;   // shared with e-paper SPI bus
static constexpr int    PIN_SD_CS    = 14;
static constexpr int    PIN_SD_EN    = 16;
static constexpr int    PIN_SD_DET   = 15;
static constexpr int    PIN_DBG_RX   = 44;
static constexpr int    PIN_DBG_TX   = 43;
#define LOG       Serial1
#define TAG       "[e1001-g4]"

// =============================================================================
// USER CONFIGURATION -- edit the constants below. Each option lists all the
// values you can pick from right above it.
// =============================================================================

// ----- image source ----------------------------------------------------------
// Path to the image on the SD card (must start with '/').
// Supported formats:
//   .jpg / .jpeg -- baseline JFIF (8-bit, YCbCr or grayscale)
//   .bmp         -- 24-bit BGR uncompressed, or 4-bit indexed (BI_RGB)
//   .png         -- any standard PNG (decoded via the bundled pngle library;
//                    RGBA images are alpha-composited over white).
// The actual format is sniffed from magic bytes -- a misleading extension is
// auto-corrected and a warning is printed.
static const char* IMAGE_PATH = "/img/demo.jpg";

// ----- dither ----------------------------------------------------------------
// Dithering algorithm. Same options as online_img2bitmap_.html. Pick one:
//
//   DITHER_NONE     -- nearest-color, no dithering (fastest, blocky)
//   DITHER_BAYER8   -- 8x8 ordered Bayer (no error buffer; safest on big panels)
//   DITHER_FS       -- Floyd-Steinberg (best quality/speed balance, default)
//   DITHER_JARVIS   -- Jarvis-Judice-Ninke (smoother, slowest)
//   DITHER_ATKINSON -- Atkinson (high contrast, classic Mac look)
//
static const DitherMethod DITHER_METHOD = DITHER_FS;

// Brightness gamma. 1.0 = neutral, >1.0 darkens, <1.0 brightens. Typical 0.8 - 1.6.
static const float DITHER_GAMMA = 1.0f;

// ----- layout: position -----------------------------------------------------
// Where on the panel the image snaps to. Pick one of these 9 anchors in a
// 3x3 grid:
//
//   ANCHOR_TOP_LEFT       ANCHOR_TOP_CENTER       ANCHOR_TOP_RIGHT
//   ANCHOR_MIDDLE_LEFT    ANCHOR_CENTER           ANCHOR_MIDDLE_RIGHT
//   ANCHOR_BOTTOM_LEFT    ANCHOR_BOTTOM_CENTER    ANCHOR_BOTTOM_RIGHT
//
enum DisplayAnchor {
  ANCHOR_TOP_LEFT,      ANCHOR_TOP_CENTER,      ANCHOR_TOP_RIGHT,
  ANCHOR_MIDDLE_LEFT,   ANCHOR_CENTER,          ANCHOR_MIDDLE_RIGHT,
  ANCHOR_BOTTOM_LEFT,   ANCHOR_BOTTOM_CENTER,   ANCHOR_BOTTOM_RIGHT,
};
static const DisplayAnchor DISPLAY_ANCHOR = ANCHOR_CENTER;

// ----- layout: size ---------------------------------------------------------
// How the image is sized relative to the panel. Pick one:
//
//   FIT_ORIGINAL -- keep source size as-is (always safe, recommended default)
//   FIT_CONTAIN  -- shrink to fit fully inside the panel; NEVER upscales
//                   (behaves like FIT_ORIGINAL if source is already smaller)
//   FIT_SCALE    -- multiply source size by DISPLAY_SCALE below
//
enum DisplayFit { FIT_ORIGINAL, FIT_CONTAIN, FIT_SCALE };
static const DisplayFit DISPLAY_FIT = FIT_SCALE;

// Scale factor -- only used when DISPLAY_FIT == FIT_SCALE. Examples:
//   0.25 -- quarter size       0.5  -- half size
//   1.0  -- original            2.0  -- 2x zoom (>1.0 may OOM on big panels)
static const float DISPLAY_SCALE = 0.7f;

// ----- diagnostics helpers ----------------------------------------------------
static void log_mem(const char* tag) {
  LOG.printf("[mem] %-22s heap=%lu kB  PSRAM free=%lu/%lu kB\n", tag,
             (unsigned long)(ESP.getFreeHeap() / 1024),
             (unsigned long)(ESP.getFreePsram() / 1024),
             (unsigned long)(ESP.getPsramSize() / 1024));
  LOG.flush();
}

static void list_sd_root(int max_entries = 32) {
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    LOG.println("[sd] cannot open '/' on the card");
    if (root) root.close();
    return;
  }
  LOG.println("[sd] contents of '/' :");
  int n = 0;
  File entry = root.openNextFile();
  while (entry) {
    if (entry.isDirectory()) LOG.printf("   <DIR>  %s\n", entry.name());
    else                     LOG.printf("   %7lu B  %s\n",
                                        (unsigned long)entry.size(), entry.name());
    entry.close();
    if (++n >= max_entries) { LOG.printf("   ... (truncated at %d entries)\n", max_entries); break; }
    entry = root.openNextFile();
  }
  if (n == 0) LOG.println("   (empty)");
  root.close();
  LOG.flush();
}

// ----- layout helpers ---------------------------------------------------------
static void compute_target_size(int src_w, int src_h, DisplayFit fit, float scale,
                                int panel_w, int panel_h,
                                int* out_w, int* out_h) {
  switch (fit) {
    case FIT_ORIGINAL:
      *out_w = src_w; *out_h = src_h;
      break;
    case FIT_CONTAIN: {
      double sx = (double)panel_w / src_w;
      double sy = (double)panel_h / src_h;
      double s  = (sx < sy) ? sx : sy;
      if (s > 1.0) s = 1.0;
      *out_w = (int)(src_w * s + 0.5);
      *out_h = (int)(src_h * s + 0.5);
      break;
    }
    case FIT_SCALE:
      *out_w = (int)(src_w * scale + 0.5);
      *out_h = (int)(src_h * scale + 0.5);
      break;
  }
  if (*out_w & 1) (*out_w)--;
  if (*out_h & 1) (*out_h)--;
  if (*out_w < 2) *out_w = 2;
  if (*out_h < 2) *out_h = 2;
}

static void compute_anchor_xy(int img_w, int img_h, DisplayAnchor a,
                              int panel_w, int panel_h,
                              int* out_x, int* out_y) {
  int x = 0, y = 0;
  switch (a) {
    case ANCHOR_TOP_LEFT:      x = 0;                      y = 0;                       break;
    case ANCHOR_TOP_CENTER:    x = (panel_w - img_w) / 2;  y = 0;                       break;
    case ANCHOR_TOP_RIGHT:     x = panel_w - img_w;        y = 0;                       break;
    case ANCHOR_MIDDLE_LEFT:   x = 0;                      y = (panel_h - img_h) / 2;   break;
    case ANCHOR_CENTER:        x = (panel_w - img_w) / 2;  y = (panel_h - img_h) / 2;   break;
    case ANCHOR_MIDDLE_RIGHT:  x = panel_w - img_w;        y = (panel_h - img_h) / 2;   break;
    case ANCHOR_BOTTOM_LEFT:   x = 0;                      y = panel_h - img_h;         break;
    case ANCHOR_BOTTOM_CENTER: x = (panel_w - img_w) / 2;  y = panel_h - img_h;         break;
    case ANCHOR_BOTTOM_RIGHT:  x = panel_w - img_w;        y = panel_h - img_h;         break;
  }
  if (x & 1) --x;   // keep 4bpp nibble alignment
  *out_x = x; *out_y = y;
}

static const char* fit_name(DisplayFit f) {
  switch (f) {
    case FIT_ORIGINAL: return "ORIGINAL";
    case FIT_CONTAIN:  return "CONTAIN";
    case FIT_SCALE:    return "SCALE";
  }
  return "?";
}
static const char* anchor_name(DisplayAnchor a) {
  switch (a) {
    case ANCHOR_TOP_LEFT:      return "TOP_LEFT";
    case ANCHOR_TOP_CENTER:    return "TOP_CENTER";
    case ANCHOR_TOP_RIGHT:     return "TOP_RIGHT";
    case ANCHOR_MIDDLE_LEFT:   return "MIDDLE_LEFT";
    case ANCHOR_CENTER:        return "CENTER";
    case ANCHOR_MIDDLE_RIGHT:  return "MIDDLE_RIGHT";
    case ANCHOR_BOTTOM_LEFT:   return "BOTTOM_LEFT";
    case ANCHOR_BOTTOM_CENTER: return "BOTTOM_CENTER";
    case ANCHOR_BOTTOM_RIGHT:  return "BOTTOM_RIGHT";
  }
  return "?";
}

// ----- main pipeline ----------------------------------------------------------
static void pack_4bpp_in_place(uint8_t* idx, int W, int H) {
  for (int y = 0; y < H; ++y) {
    const uint8_t* src = idx + (size_t)y * W;
    uint8_t* dst       = idx + (size_t)y * (W / 2);
    for (int x = 0; x < W; x += 2) {
      const uint8_t a = src[x]     & 0xF;
      const uint8_t b = src[x + 1] & 0xF;
      dst[x >> 1] = (uint8_t)((a << 4) | b);
    }
  }
}

static bool show_image_on_panel(RgbImage* img) {
  // 1) Decide the final on-panel size from the source size + fit mode.
  int W, H;
  compute_target_size(img->width, img->height, DISPLAY_FIT, DISPLAY_SCALE,
                      EPD_WIDTH, EPD_HEIGHT, &W, &H);
  int x, y;
  compute_anchor_xy(W, H, DISPLAY_ANCHOR, EPD_WIDTH, EPD_HEIGHT, &x, &y);
  LOG.printf("[layout] src=%dx%d  fit=%s  anchor=%s  scale=%.3f\n",
             img->width, img->height, fit_name(DISPLAY_FIT),
             anchor_name(DISPLAY_ANCHOR), DISPLAY_SCALE);
  LOG.printf("[layout] -> target=%dx%d  at (%d,%d)  panel=%dx%d\n",
             W, H, x, y, EPD_WIDTH, EPD_HEIGHT);
  if (W > EPD_WIDTH || H > EPD_HEIGHT) {
    LOG.printf("[layout] target exceeds panel by (%d,%d) px -- overflow will be cropped\n",
               W - EPD_WIDTH, H - EPD_HEIGHT);
  }

  // 2) Resize if needed. resize_image() allocates a new PSRAM buffer for the
  //    target and then frees the old one, so peak memory briefly holds both.
  if (W != img->width || H != img->height) {
    const size_t need_kb = (size_t)W * H * 3 / 1024;
    LOG.printf("[layout] resizing %dx%d -> %dx%d  (needs +%lu kB temporarily)\n",
               img->width, img->height, W, H, (unsigned long)need_kb);
    log_mem("before resize");
    if (!resize_image(img, W, H)) {
      LOG.println("[layout] resize failed (likely OOM) -- aborting");
      LOG.println("[layout]   try FIT_ORIGINAL, or a smaller DISPLAY_SCALE,");
      LOG.println("[layout]   or pre-shrink the image on the PC");
      return false;
    }
    log_mem("after resize");
  }

  if ((W & 1) || (H & 1)) {
    LOG.println(TAG " image size must be even on both axes for 4bpp pushImage");
    return false;
  }
  const size_t npx = (size_t)W * H;

  // 3) Dither into a 1-byte/pixel index buffer.
  log_mem("before idx malloc");
  LOG.printf(TAG " allocating index buf: %lu kB\n", (unsigned long)(npx / 1024));
  uint8_t* idx = (uint8_t*)ps_malloc(npx);
  if (!idx) idx = (uint8_t*)malloc(npx);
  if (!idx) { LOG.println(TAG " OOM idx -- aborting"); return false; }

  static const char* kDitherNames[] = {"NONE", "BAYER8", "FS", "JARVIS", "ATKINSON"};
  LOG.printf(TAG " dithering Gray4 with %s, gamma=%.2f ...\n",
             kDitherNames[(int)DITHER_METHOD], DITHER_GAMMA);
  const uint32_t t0 = millis();
  if (!dither_image(img->pixels, W, H, PAL_GRAY4, DITHER_METHOD,
                    DITHER_GAMMA, /*invert=*/false, idx)) {
    LOG.println(TAG " dither failed -- aborting");
    free(idx); return false;
  }
  LOG.printf(TAG " dither done in %lu ms\n", (unsigned long)(millis() - t0));

  // 4) Free the RGB888 source -- we don't need it anymore -- then pack to 4bpp.
  LOG.println(TAG " freeing RGB888 source");
  image_free(img);
  log_mem("after RGB888 freed");

  pack_4bpp_in_place(idx, W, H);

  // 5) Push at the computed anchor location. pushImage clips automatically when
  //    (x,y,W,H) extends outside the sprite.
  LOG.printf(TAG " pushImage %dx%d at (%d,%d) -> sprite\n", W, H, x, y);
  epaper.pushImage(x, y, W, H, (uint16_t*)idx);
  LOG.println(TAG " update() -- panel refresh starts");
  LOG.flush();
  const uint32_t t1 = millis();
  epaper.update();
  LOG.printf(TAG " update done in %lu ms\n", (unsigned long)(millis() - t1));
  free(idx);
  return true;
}

void setup() {
  LOG.begin(115200, SERIAL_8N1, PIN_DBG_RX, PIN_DBG_TX);
  delay(2500);
  LOG.println();
  LOG.println("==============================================");
  LOG.println("  reTerminal E1001 -- SD Bitmap (Gray4)");
  LOG.println("==============================================");
  LOG.printf("[sys] chip      : ESP32-S3 @ %lu MHz\n", (unsigned long)ESP.getCpuFreqMHz());
  LOG.printf("[sys] PSRAM size: %lu kB\n", (unsigned long)(ESP.getPsramSize() / 1024));
  if (ESP.getPsramSize() == 0) {
    LOG.println("[sys] !!! PSRAM is 0 kB -- enable Tools > PSRAM > OPI PSRAM in the IDE !!!");
  }
  LOG.printf("[sys] panel     : %d x %d (EPD_WIDTH x EPD_HEIGHT)\n", EPD_WIDTH, EPD_HEIGHT);
  LOG.printf("[sys] image     : '%s'\n", IMAGE_PATH);
  LOG.flush();

  pinMode(PIN_SD_EN, OUTPUT);
  digitalWrite(PIN_SD_EN, HIGH);
  pinMode(PIN_SD_DET, INPUT_PULLUP);
  delay(50);
  const int sd_det = digitalRead(PIN_SD_DET);
  LOG.printf("[sd] SD_DET (GPIO%d) reads %s -- %s\n", PIN_SD_DET,
             sd_det ? "HIGH" : "LOW",
             sd_det ? "no card detected (or DET pin floating)" : "card present");
  LOG.flush();

  LOG.println(TAG " EPaper begin ...");
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);
  epaper.update();
  LOG.println(TAG " EPaper cleared to white");
  log_mem("after epaper.begin");

  LOG.println(TAG " initGrayMode(4)");
  epaper.initGrayMode(GRAY_LEVEL4);
  epaper.fillSprite(TFT_GRAY_3);   // start from white
  log_mem("after initGrayMode");

  // The UC8179 BW controller is write-only, so Setup520 defines TFT_MISO=-1.
  // epaper.begin() therefore initialized the shared SPI bus WITHOUT routing MISO,
  // but SD cards are full-duplex and SD.begin() needs MISO to read card responses.
  // Re-init the same SPI bus with MISO=GPIO8 added; the display's CS/RST/DC/BUSY
  // are independent GPIOs and are untouched by this.
  SPIClass& spi = epaper.getSPIinstance();
  spi.end();
  spi.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, /*ss=*/-1);
  LOG.printf(TAG " SPI bus re-init: SCK=%d MISO=%d MOSI=%d (for SD)\n",
             PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI);

  LOG.println(TAG " SD.begin (shares HSPI with EPaper) ...");
  if (!SD.begin(PIN_SD_CS, spi)) {
    LOG.println(TAG " SD.begin FAILED -- aborting");
    LOG.println(TAG "   - Is the card inserted and formatted as FAT32 (or exFAT)?");
    LOG.printf (TAG "   - Is SD_EN (GPIO%d) actually wired to power the slot?\n", PIN_SD_EN);
    return;
  }
  LOG.printf("[sd] mounted; card size = %llu MB\n",
             (unsigned long long)(SD.cardSize() / (1024ULL * 1024ULL)));
  list_sd_root();

  if (!SD.exists(IMAGE_PATH)) {
    LOG.printf(TAG " image '%s' does NOT exist on SD -- aborting\n", IMAGE_PATH);
    LOG.println(TAG "   - check the file name above (case-sensitive)");
    LOG.println(TAG "   - path must start with '/' (e.g. \"/pic1.jpg\")");
    return;
  }
  {
    File probe = SD.open(IMAGE_PATH, FILE_READ);
    if (probe) {
      LOG.printf(TAG " image found: %s (%lu bytes)\n",
                 IMAGE_PATH, (unsigned long)probe.size());
      probe.close();
    }
  }

  log_mem("before image load");
  RgbImage img;
  if (!load_image_from_sd(IMAGE_PATH, /*target_w=*/0, /*target_h=*/0, &img)) {
    LOG.println(TAG " load failed -- aborting");
    LOG.println(TAG "   common issues:");
    LOG.println(TAG "    - source resolution too large (decodes to > 8 MB RGB888)");
    LOG.println(TAG "    - JPEG: non-baseline (progressive / 4:1:1 / CMYK not supported)");
    LOG.println(TAG "    - PNG : OOM at IHDR allocation (try a smaller image / FIT_CONTAIN)");
    return;
  }
  LOG.printf(TAG " image decoded: %dx%d  (%lu kB in PSRAM)\n",
             img.width, img.height,
             (unsigned long)((size_t)img.width * img.height * 3 / 1024));
  log_mem("after image decoded");

  if (show_image_on_panel(&img)) LOG.println(TAG " frame pushed OK");
  else                           LOG.println(TAG " show_image_on_panel failed");
  image_free(&img);

  LOG.println(TAG " done. Sleeping panel.");
  epaper.sleep();
  LOG.println("==============================================");
}

void loop() {
  // The e-paper holds the image without power. Press RESET to re-display.
  delay(1000);
}
