// SD image loader. JPEG decoder is the minimal in-tree implementation originally written
// for reTerminal_local (Arduino/src/jpeg_decode.cpp) -- baseline JFIF only, no PMG/EXIF
// helpers, no DRI restart-marker handling beyond skipping them in the bitstream.
//
// PNG support is provided by pngle (MIT, https://github.com/kikuchan/pngle) which is
// vendored next to this file (pngle.{h,c}) together with its zlib/inflate backend
// (miniz.{h,c}). pngle streams the PNG line-by-line and calls our pngle_on_draw
// callback for each pixel, which we composite over white into an RGB888 buffer.
//
// We intentionally keep all three decoders in a single sketch folder (BMP + JPEG
// in this translation unit's anonymous namespace, PNG via the bundled pngle .c files)
// so each example folder is self-contained -- no shared library files between examples
// and no Arduino library installs required.
#include "image_loader.h"

#include <FS.h>
#include <SD.h>
#include <cmath>
#include <cstring>

extern "C" {
#include "pngle.h"
}

// Uncomment to dump every JPEG marker (APP/DHT/DQT/...) the parser walks past.
// Off by default -- only useful when debugging a JPEG that fails to decode.
// #define VERBOSE_JPEG_DECODE

namespace {

// =====================================================================================
// Tiny BMP reader: 24-bit BGR (BI_RGB) and 4-bit indexed (BI_RGB w/ palette).
// =====================================================================================
//
// BMP layout we accept:
//   File header  14 B    (BM + filesize + reserved + pixel-data offset)
//   DIB header   40 B    (BITMAPINFOHEADER, biSize=40)
//   Palette      4*N B   (BGRA), only present for <=8 bpp
//   Pixel data   row-padded to 4 bytes, bottom-up unless height is negative
static bool decode_bmp(File& f, RgbImage* out) {
  uint8_t hdr[14];
  if (f.read(hdr, 14) != 14)         { Serial1.println("[bmp] short header"); return false; }
  if (hdr[0] != 'B' || hdr[1] != 'M') {
    Serial1.printf("[bmp] bad magic (first 14 bytes: ");
    for (int i = 0; i < 14; ++i) Serial1.printf("%02X ", hdr[i]);
    Serial1.print(" | ascii: \"");
    for (int i = 0; i < 14; ++i) Serial1.printf("%c", (hdr[i] >= 0x20 && hdr[i] < 0x7F) ? hdr[i] : '.');
    Serial1.println("\")");
    return false;
  }
  const uint32_t pixel_off =
      (uint32_t)hdr[10] | ((uint32_t)hdr[11] << 8) |
      ((uint32_t)hdr[12] << 16) | ((uint32_t)hdr[13] << 24);

  uint8_t dib[40];
  if (f.read(dib, 40) != 40)          { Serial1.println("[bmp] short DIB"); return false; }
  const uint32_t dib_size = (uint32_t)dib[0] | ((uint32_t)dib[1] << 8) |
                            ((uint32_t)dib[2] << 16) | ((uint32_t)dib[3] << 24);
  if (dib_size < 40)                  { Serial1.println("[bmp] DIB too small"); return false; }
  const int32_t  w        = (int32_t)((uint32_t)dib[4]  | ((uint32_t)dib[5]  << 8) |
                                      ((uint32_t)dib[6]  << 16) | ((uint32_t)dib[7]  << 24));
  const int32_t  h_signed = (int32_t)((uint32_t)dib[8]  | ((uint32_t)dib[9]  << 8) |
                                      ((uint32_t)dib[10] << 16) | ((uint32_t)dib[11] << 24));
  const uint16_t bpp      = (uint16_t)dib[14] | ((uint16_t)dib[15] << 8);
  const uint32_t comp     = (uint32_t)dib[16] | ((uint32_t)dib[17] << 8) |
                            ((uint32_t)dib[18] << 16) | ((uint32_t)dib[19] << 24);

  if (comp != 0) { Serial1.printf("[bmp] only BI_RGB supported (got %u)\n", comp); return false; }
  if (bpp != 24 && bpp != 4) {
    Serial1.printf("[bmp] only 24bpp / 4bpp supported (got %u)\n", bpp);
    return false;
  }

  const bool top_down = (h_signed < 0);
  const int  h        = top_down ? -h_signed : h_signed;
  if (w <= 0 || h <= 0)               { Serial1.println("[bmp] zero dim"); return false; }

  // Read palette for indexed.
  uint8_t palette[16][3] = {{0}};
  if (bpp == 4) {
    // Skip rest of DIB if larger than 40 bytes.
    if (dib_size > 40) f.seek(14 + dib_size);
    uint8_t pal_entry[4];
    for (int i = 0; i < 16; ++i) {
      if (f.read(pal_entry, 4) != 4) { Serial1.println("[bmp] short palette"); return false; }
      palette[i][0] = pal_entry[2];  // R
      palette[i][1] = pal_entry[1];  // G
      palette[i][2] = pal_entry[0];  // B
    }
  }
  f.seek(pixel_off);

  const size_t rgb_size = (size_t)w * h * 3;
  uint8_t* rgb = (uint8_t*)ps_malloc(rgb_size);
  if (!rgb) rgb = (uint8_t*)malloc(rgb_size);
  if (!rgb) { Serial1.println("[bmp] OOM RGB"); return false; }

  const int  row_bytes = (bpp == 24) ? w * 3 : ((w + 1) / 2);
  const int  padded    = (row_bytes + 3) & ~3;
  uint8_t* line = (uint8_t*)malloc(padded);
  if (!line) { free(rgb); Serial1.println("[bmp] OOM line"); return false; }

  for (int row = 0; row < h; ++row) {
    if (f.read(line, padded) != padded) { free(line); free(rgb); Serial1.println("[bmp] short row"); return false; }
    const int dst_y = top_down ? row : (h - 1 - row);
    uint8_t* dst = rgb + (size_t)dst_y * w * 3;
    if (bpp == 24) {
      // BGR -> RGB
      for (int x = 0; x < w; ++x) {
        dst[x * 3 + 0] = line[x * 3 + 2];
        dst[x * 3 + 1] = line[x * 3 + 1];
        dst[x * 3 + 2] = line[x * 3 + 0];
      }
    } else {
      for (int x = 0; x < w; ++x) {
        const uint8_t b   = line[x >> 1];
        const uint8_t idx = (x & 1) ? (b & 0x0F) : (b >> 4);
        dst[x * 3 + 0] = palette[idx][0];
        dst[x * 3 + 1] = palette[idx][1];
        dst[x * 3 + 2] = palette[idx][2];
      }
    }
  }
  free(line);

  out->pixels = rgb;
  out->width  = w;
  out->height = h;
  return true;
}

// =====================================================================================
// Minimal baseline-JPEG decoder. Ported from reTerminal_local's jpeg_decode.cpp.
// =====================================================================================

struct JpegRgb { uint8_t* pixels = nullptr; int width = 0; int height = 0; };

static const uint8_t ZZ[64] = {
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

struct Huff {
  uint8_t nsym = 0;
  uint8_t sym[256] = {0};
  int mincode[17] = {0};
  int maxcode[17] = {0};
  int valptr [17] = {0};
};
struct Bs  { const uint8_t* data=nullptr; size_t len=0, pos=0; uint32_t buf=0; int cnt=0; bool err=false; };
struct Cmp { uint8_t id=0, hs=1, vs=1, qt=0, dcht=0, acht=0; int dcprev=0; };
struct Jd  {
  const uint8_t* data=nullptr; size_t len=0, pos=0;
  uint16_t width=0, height=0;
  uint8_t  ncomp=0;
  Cmp      comp[3];
  uint16_t qt[4][64] = {{0}};
  Huff     dcht[2], acht[2];
  uint16_t rst_int=0;
  uint8_t* rgb=nullptr;
};

static float COSTBL[8][8];

static inline uint8_t  jbyte(Jd* j){ return (j->pos < j->len) ? j->data[j->pos++] : 0; }
static inline uint16_t ju16 (Jd* j){ uint8_t h = jbyte(j), l = jbyte(j); return (uint16_t)((h << 8) | l); }

static void idct_init() {
  for (int u = 0; u < 8; ++u) {
    float cu = (u == 0) ? (1.0f / sqrtf(2.0f)) : 1.0f;
    for (int x = 0; x < 8; ++x) COSTBL[u][x] = cu * cosf((2.0f * x + 1.0f) * u * (float)M_PI / 16.0f);
  }
}
static void idct_1d(float* v) {
  float out[8];
  for (int x = 0; x < 8; ++x) {
    float s = 0.0f;
    for (int u = 0; u < 8; ++u) s += v[u] * COSTBL[u][x];
    out[x] = s * 0.5f;
  }
  memcpy(v, out, 8 * sizeof(float));
}
static void idct_2d(float block[64]) {
  for (int r = 0; r < 8; ++r) idct_1d(&block[r * 8]);
  float col[8];
  for (int c = 0; c < 8; ++c) {
    for (int r = 0; r < 8; ++r) col[r] = block[r * 8 + c];
    idct_1d(col);
    for (int r = 0; r < 8; ++r) block[r * 8 + c] = col[r];
  }
}
static void huff_build(Huff* h, const uint8_t bits[16], const uint8_t* vals) {
  h->nsym = 0;
  for (int i = 0; i < 16; ++i) h->nsym += bits[i];
  memcpy(h->sym, vals, h->nsym);
  int code = 0, si = 0;
  for (int l = 1; l <= 16; ++l) {
    if (bits[l - 1] == 0) {
      h->mincode[l] = 1;
      h->maxcode[l] = -1;
      h->valptr [l] = 0;
    } else {
      h->valptr [l] = si;
      h->mincode[l] = code;
      h->maxcode[l] = code + bits[l - 1] - 1;
      si += bits[l - 1];
    }
    code = (code + bits[l - 1]) << 1;
  }
}
static void bs_refill(Bs* b) {
  while (b->cnt <= 24 && b->pos < b->len) {
    uint8_t v = b->data[b->pos++];
    if (v == 0xFF) {
      if (b->pos >= b->len) break;
      uint8_t nx = b->data[b->pos];
      if (nx == 0x00) { b->pos++; }
      // RST marker or any other marker: stop refilling, let the caller handle it.
      // (For RST the MCU loop calls bs_restart() at the appropriate cadence; without
      // that the DC predictors would never be reset and decoding would corrupt itself.)
      else { b->pos--; break; }
    }
    b->buf = (b->buf << 8) | v;
    b->cnt += 8;
  }
}

// Called by the MCU loop every `restart_interval` MCUs. Drops any remaining bits in
// the bit buffer (byte-align), then advances past the upcoming FF Dx restart marker
// if it is the next thing in the stream. The MCU loop is responsible for also
// resetting all DC predictors to 0 after calling this.
static void bs_restart(Bs* b) {
  b->buf = 0;
  b->cnt = 0;
  // Hop over any padding 0xFF bytes (rare but legal), then the marker byte itself.
  while (b->pos < b->len && b->data[b->pos] == 0xFF) b->pos++;
  if (b->pos < b->len) {
    uint8_t m = b->data[b->pos];
    if (m >= 0xD0 && m <= 0xD7) b->pos++;
  }
}
static inline int bs_bits(Bs* b, int n) {
  if (n == 0) return 0;
  if (b->cnt < n) bs_refill(b);
  if (b->cnt < n) { b->err = true; return 0; }
  b->cnt -= n;
  return (int)((b->buf >> b->cnt) & ((1u << n) - 1));
}
static int huff_sym(Bs* b, const Huff* h) {
  int code = 0;
  for (int l = 1; l <= 16; ++l) {
    code = (code << 1) | bs_bits(b, 1);
    if (code <= h->maxcode[l] && h->maxcode[l] >= 0) return h->sym[h->valptr[l] + code - h->mincode[l]];
  }
  b->err = true;
  return 0;
}
static inline int receive(Bs* b, int n) {
  int v = bs_bits(b, n);
  if (n > 0 && v < (1 << (n - 1))) v -= (1 << n) - 1;
  return v;
}
static void decode_block(Bs* b, const Huff* dcht, const Huff* acht, const uint16_t* qt, int* dcprev, float out[64]) {
  int coef[64] = {0};
  int s = huff_sym(b, dcht);
  *dcprev += receive(b, s);
  coef[0] = *dcprev;
  for (int k = 1; k < 64;) {
    int sym = huff_sym(b, acht);
    if (sym == 0x00) break;
    if (sym == 0xF0) { k += 16; continue; }
    int run = (sym >> 4) & 0xF;
    int sz  = sym & 0xF;
    k += run;
    if (k >= 64) break;
    coef[k++] = receive(b, sz);
  }
  for (int i = 0; i < 64; ++i) out[ZZ[i]] = (float)(coef[i] * (int)qt[i]);
  idct_2d(out);
  for (int i = 0; i < 64; ++i) out[i] += 128.0f;
}
static inline uint8_t clamp8(float v) {
  int i = (int)(v + 0.5f);
  return (uint8_t)(i < 0 ? 0 : (i > 255 ? 255 : i));
}
static bool parse_sof0(Jd* j) {
  jbyte(j);
  j->height = ju16(j);
  j->width  = ju16(j);
  j->ncomp  = jbyte(j);
  if (j->ncomp > 3) return false;
  for (int i = 0; i < j->ncomp; ++i) {
    j->comp[i].id = jbyte(j);
    uint8_t sf = jbyte(j);
    j->comp[i].hs = (sf >> 4) & 0xF;
    j->comp[i].vs =  sf       & 0xF;
    j->comp[i].qt = jbyte(j) & 0x3;
  }
  return true;
}
static bool parse_dqt(Jd* j, int seglen) {
  int rem = seglen - 2;
  while (rem > 0) {
    uint8_t b = jbyte(j); rem--;
    int prec = (b >> 4) & 0xF;
    int id   =  b       & 0xF;
    if (id >= 4) return false;
    for (int k = 0; k < 64; ++k) {
      j->qt[id][k] = prec ? ju16(j) : jbyte(j);
      rem -= prec ? 2 : 1;
    }
  }
  return true;
}
static bool parse_dht(Jd* j, int seglen) {
  int rem = seglen - 2;
  while (rem > 0) {
    uint8_t tc = jbyte(j); rem--;
    int is_ac = (tc >> 4) & 0x1;
    int id    =  tc       & 0x1;
    uint8_t bits[16];
    int nsym = 0;
    for (int i = 0; i < 16; ++i) { bits[i] = jbyte(j); nsym += bits[i]; rem--; }
    uint8_t vals[256];
    for (int i = 0; i < nsym; ++i) { vals[i] = jbyte(j); rem--; }
    if (is_ac) huff_build(&j->acht[id], bits, vals);
    else       huff_build(&j->dcht[id], bits, vals);
  }
  return true;
}
static void parse_sos(Jd* j) {
  uint8_t n = jbyte(j);
  for (int i = 0; i < n && i < 3; ++i) {
    uint8_t cid = jbyte(j);
    uint8_t tb  = jbyte(j);
    for (int c = 0; c < j->ncomp; ++c) {
      if (j->comp[c].id == cid) {
        j->comp[c].dcht = (tb >> 4) & 0x1;
        j->comp[c].acht =  tb       & 0x1;
        break;
      }
    }
  }
  j->pos += 3;
}

// Decode a JPEG marker to a short human-readable name (for diagnostics).
static const char* jpeg_marker_name(uint8_t m) {
  switch (m) {
    case 0xC0: return "SOF0 (baseline)";
    case 0xC1: return "SOF1 (extended sequential)";
    case 0xC2: return "SOF2 (progressive)";
    case 0xC3: return "SOF3 (lossless)";
    case 0xC5: return "SOF5 (differential sequential)";
    case 0xC6: return "SOF6 (differential progressive)";
    case 0xC7: return "SOF7 (differential lossless)";
    case 0xC9: return "SOF9 (arithmetic)";
    case 0xCA: return "SOF10 (arithmetic progressive)";
    case 0xCB: return "SOF11 (arithmetic lossless)";
    case 0xC4: return "DHT";
    case 0xCC: return "DAC";
    case 0xDB: return "DQT";
    case 0xDD: return "DRI";
    case 0xDA: return "SOS";
    case 0xD9: return "EOI";
    case 0xE0: return "APP0/JFIF";
    case 0xE1: return "APP1 (EXIF)";
    case 0xE2: return "APP2";
    case 0xEE: return "APP14 (Adobe)";
    case 0xFE: return "COM";
    default:   return "?";
  }
}

static bool jpeg_decode_rgb(const uint8_t* data, size_t len, JpegRgb* out) {
  if (!data || !len || !out) {
    Serial1.println("[jpg]   bad args to decoder");
    return false;
  }
  idct_init();
  Jd* j = (Jd*)calloc(1, sizeof(Jd));
  if (!j) { Serial1.println("[jpg]   OOM Jd context"); return false; }
  j->data = data; j->len = len; j->pos = 0;
  if (jbyte(j) != 0xFF || jbyte(j) != 0xD8) {
    Serial1.println("[jpg]   no SOI (FF D8) -- not a JPEG file");
    free(j); return false;
  }

  bool got_sof = false;
  while (j->pos < j->len) {
    while (j->pos < j->len && jbyte(j) != 0xFF) {}
    uint8_t m = jbyte(j);
    if (m == 0xD9) break;
    if (m == 0x00 || (m >= 0xD0 && m <= 0xD7)) continue;
    int seglen = (int)ju16(j);
#ifdef VERBOSE_JPEG_DECODE
    Serial1.printf("[jpg]   marker FF%02X (%s) seglen=%d @0x%lX\n",
                   m, jpeg_marker_name(m), seglen,
                   (unsigned long)(j->pos - 2));
#endif
    switch (m) {
      case 0xDB:
        if (!parse_dqt(j, seglen)) {
          Serial1.println("[jpg]   parse_dqt failed");
          free(j); return false;
        }
        break;
      case 0xC0:
        if (!parse_sof0(j)) {
          Serial1.println("[jpg]   parse_sof0 failed");
          free(j); return false;
        }
        got_sof = true;
        Serial1.printf("[jpg]   SOF0 baseline %dx%d, %d components\n",
                       j->width, j->height, j->ncomp);
#ifdef VERBOSE_JPEG_DECODE
        for (int c = 0; c < j->ncomp; ++c) {
          Serial1.printf("[jpg]     comp[%d] id=%d hs=%d vs=%d qt=%d\n",
                         c, j->comp[c].id, j->comp[c].hs, j->comp[c].vs, j->comp[c].qt);
        }
#endif
        break;
      case 0xC2: case 0xC1: case 0xC3:
      case 0xC5: case 0xC6: case 0xC7:
      case 0xC9: case 0xCA: case 0xCB:
        Serial1.printf("[jpg]   unsupported SOF type FF%02X (%s)\n", m, jpeg_marker_name(m));
        Serial1.println("[jpg]   -> only baseline (SOF0 / FFC0) is supported.");
        Serial1.println("[jpg]      Re-export as a regular baseline JPEG, or convert to BMP.");
        free(j); return false;
      case 0xC4:
        if (!parse_dht(j, seglen)) {
          Serial1.println("[jpg]   parse_dht failed");
          free(j); return false;
        }
        break;
      case 0xDD: j->rst_int = ju16(j); break;
      case 0xDA: parse_sos(j); goto scan;
      default:   j->pos += seglen - 2; break;
    }
  }
scan:
  if (!got_sof) {
    Serial1.println("[jpg]   no SOF0 marker encountered before SOS");
    free(j); return false;
  }
  if (!j->width || !j->height) {
    Serial1.println("[jpg]   zero width/height in SOF0");
    free(j); return false;
  }

  size_t rgb_sz = (size_t)j->width * j->height * 3;
  Serial1.printf("[jpg]   allocating RGB888 buffer: %lu kB (%dx%d x3)\n",
                 (unsigned long)(rgb_sz / 1024), j->width, j->height);
  j->rgb = (uint8_t*)ps_malloc(rgb_sz);
  if (!j->rgb) j->rgb = (uint8_t*)malloc(rgb_sz);
  if (!j->rgb) {
    Serial1.printf("[jpg]   OOM RGB888 (%lu kB) -- image too large for PSRAM\n",
                   (unsigned long)(rgb_sz / 1024));
    free(j); return false;
  }
  memset(j->rgb, 0xFF, rgb_sz);

  int max_hs = 1, max_vs = 1;
  for (int c = 0; c < j->ncomp; ++c) {
    if (j->comp[c].hs > max_hs) max_hs = j->comp[c].hs;
    if (j->comp[c].vs > max_vs) max_vs = j->comp[c].vs;
  }
  int mcu_w = max_hs * 8, mcu_h = max_vs * 8;
  int mcu_cols = ((int)j->width  + mcu_w - 1) / mcu_w;
  int mcu_rows = ((int)j->height + mcu_h - 1) / mcu_h;
  Serial1.printf("[jpg]   MCU = %dx%d, grid = %dx%d (%d MCUs total), restart_interval=%u\n",
                 mcu_w, mcu_h, mcu_cols, mcu_rows, mcu_cols * mcu_rows,
                 (unsigned)j->rst_int);

  Bs bs;
  bs.data = j->data; bs.len = j->len; bs.pos = j->pos;

  float* cbuf[3] = {nullptr, nullptr, nullptr};
  for (int c = 0; c < j->ncomp; ++c) {
    int n = j->comp[c].hs * j->comp[c].vs * 64;
    cbuf[c] = (float*)malloc(n * sizeof(float));
    if (!cbuf[c]) {
      Serial1.printf("[jpg]   OOM cbuf[%d] (%d floats)\n", c, n);
      for (int k = 0; k < c; ++k) free(cbuf[k]);
      free(j->rgb); free(j); return false;
    }
  }

  // MCU counter for restart-marker handling. When rst_int > 0 the encoder inserts a
  // RSTn marker every `rst_int` MCUs in scan order; on the decoder side we must
  // discard any remaining bits in the bit buffer, hop over the 0xFF Dx marker,
  // and reset every component's DC predictor to 0 before continuing.
  uint32_t mcu_since_rst = 0;

  for (int my = 0; my < mcu_rows; ++my) {
    for (int mx = 0; mx < mcu_cols; ++mx) {
      for (int c = 0; c < j->ncomp; ++c) {
        int hs = j->comp[c].hs, vs = j->comp[c].vs;
        for (int by = 0; by < vs; ++by) {
          for (int bx = 0; bx < hs; ++bx) {
            float* dst = cbuf[c] + (by * hs + bx) * 64;
            decode_block(&bs, &j->dcht[j->comp[c].dcht], &j->acht[j->comp[c].acht],
                         j->qt[j->comp[c].qt], &j->comp[c].dcprev, dst);
          }
        }
      }
      if (bs.err) goto done;

      // Process a restart marker after every `rst_int` MCUs (if enabled).
      if (j->rst_int) {
        if (++mcu_since_rst == j->rst_int) {
          mcu_since_rst = 0;
          bs_restart(&bs);
          for (int c = 0; c < j->ncomp; ++c) j->comp[c].dcprev = 0;
        }
      }

      for (int py = 0; py < mcu_h; ++py) {
        int iy = my * mcu_h + py;
        if (iy >= (int)j->height) continue;
        for (int px = 0; px < mcu_w; ++px) {
          int ix = mx * mcu_w + px;
          if (ix >= (int)j->width) continue;
          float yy = 128.0f;
          {
            int hs = j->comp[0].hs, vs = j->comp[0].vs;
            int sx = px * hs / max_hs, sy = py * vs / max_vs;
            int bx = sx / 8, ox = sx % 8, by = sy / 8, oy = sy % 8;
            if (bx >= hs) bx = hs - 1;
            if (by >= vs) by = vs - 1;
            yy = cbuf[0][(by * hs + bx) * 64 + oy * 8 + ox];
          }
          float cb = 128.0f, cr = 128.0f;
          if (j->ncomp > 1) {
            int hs = j->comp[1].hs, vs = j->comp[1].vs;
            int sx = px * hs / max_hs, sy = py * vs / max_vs;
            int bx = sx / 8, ox = sx % 8, by = sy / 8, oy = sy % 8;
            if (bx >= hs) bx = hs - 1;
            if (by >= vs) by = vs - 1;
            cb = cbuf[1][(by * hs + bx) * 64 + oy * 8 + ox];
          }
          if (j->ncomp > 2) {
            int hs = j->comp[2].hs, vs = j->comp[2].vs;
            int sx = px * hs / max_hs, sy = py * vs / max_vs;
            int bx = sx / 8, ox = sx % 8, by = sy / 8, oy = sy % 8;
            if (bx >= hs) bx = hs - 1;
            if (by >= vs) by = vs - 1;
            cr = cbuf[2][(by * hs + bx) * 64 + oy * 8 + ox];
          }
          float cb0 = cb - 128.0f, cr0 = cr - 128.0f;
          size_t idx = ((size_t)iy * j->width + ix) * 3;
          j->rgb[idx + 0] = clamp8(yy + 1.40200f * cr0);
          j->rgb[idx + 1] = clamp8(yy - 0.34414f * cb0 - 0.71414f * cr0);
          j->rgb[idx + 2] = clamp8(yy + 1.77200f * cb0);
        }
      }
    }
  }
done:
  for (int c = 0; c < j->ncomp; ++c) free(cbuf[c]);
  if (bs.err) {
    Serial1.println("[jpg]   bitstream error during entropy decode -- file may be truncated or corrupt");
    free(j->rgb); free(j); return false;
  }
  out->pixels = j->rgb;
  out->width  = j->width;
  out->height = j->height;
  free(j);
  return true;
}

static bool decode_jpeg(File& f, RgbImage* out) {
  const size_t sz = f.size();
  uint8_t* raw = (uint8_t*)ps_malloc(sz);
  if (!raw) raw = (uint8_t*)malloc(sz);
  if (!raw) { Serial1.println("[jpg] OOM file"); return false; }
  size_t got = f.read(raw, sz);
  if (got != sz) { free(raw); Serial1.println("[jpg] short read"); return false; }

  JpegRgb tmp;
  bool ok = jpeg_decode_rgb(raw, sz, &tmp);
  free(raw);
  if (!ok) { Serial1.println("[jpg] decode failed"); return false; }
  out->pixels = tmp.pixels;
  out->width  = tmp.width;
  out->height = tmp.height;
  return true;
}

// =====================================================================================
// PNG (via pngle, MIT license, bundled in this folder as pngle.{h,c} + miniz.{h,c}).
// We allocate one RGB888 output buffer up-front in the init callback and stream the
// decoder line-by-line; RGBA inputs are alpha-composited over a white background since
// the e-paper panel is opaque.
// =====================================================================================
struct PngCtx {
  RgbImage* out;
  bool      oom;
};

static void pngle_on_init(pngle_t* p, uint32_t w, uint32_t h) {
  PngCtx* ctx = (PngCtx*)pngle_get_user_data(p);
  ctx->out->width  = (int)w;
  ctx->out->height = (int)h;
  const size_t n = (size_t)w * h * 3;
  Serial1.printf("[png]   IHDR %ux%u, allocating RGB888 buffer: %lu kB\n",
                 (unsigned)w, (unsigned)h, (unsigned long)(n / 1024));
  uint8_t* buf = (uint8_t*)ps_malloc(n);
  if (!buf) buf = (uint8_t*)malloc(n);
  if (!buf) {
    ctx->oom = true;
    Serial1.printf("[png]   OOM: need %lu kB but couldn't allocate\n",
                   (unsigned long)(n / 1024));
    return;
  }
  // Pre-fill with white so any out-of-bounds draw_cb or aborted decode still produces
  // a coherent image (rather than leaving the buffer uninitialized).
  memset(buf, 0xFF, n);
  ctx->out->pixels = buf;
}

static void pngle_on_draw(pngle_t* p, uint32_t x, uint32_t y,
                          uint32_t w, uint32_t h, const uint8_t rgba[4]) {
  PngCtx* ctx = (PngCtx*)pngle_get_user_data(p);
  if (ctx->oom || !ctx->out->pixels) return;
  // Composite over white when there's transparency.
  uint8_t r = rgba[0], g = rgba[1], b = rgba[2];
  if (rgba[3] != 0xFF) {
    const uint16_t a = rgba[3];
    const uint16_t ia = 255 - a;
    r = (uint8_t)((r * a + 255 * ia) / 255);
    g = (uint8_t)((g * a + 255 * ia) / 255);
    b = (uint8_t)((b * a + 255 * ia) / 255);
  }
  // pngle may call us with w,h>1 for interlaced PNGs (Adam7 sub-pass replicates).
  const int W = ctx->out->width;
  const int H = ctx->out->height;
  for (uint32_t dy = 0; dy < h; ++dy) {
    const int py = (int)(y + dy);
    if (py >= H) break;
    for (uint32_t dx = 0; dx < w; ++dx) {
      const int px = (int)(x + dx);
      if (px >= W) break;
      uint8_t* dst = ctx->out->pixels + ((size_t)py * W + px) * 3;
      dst[0] = r; dst[1] = g; dst[2] = b;
    }
  }
}

static bool decode_png(File& f, RgbImage* out) {
  pngle_t* png = pngle_new();
  if (!png) { Serial1.println("[png] pngle_new failed"); return false; }

  PngCtx ctx{ out, false };
  pngle_set_user_data(png, &ctx);
  pngle_set_init_callback(png, pngle_on_init);
  pngle_set_draw_callback(png, pngle_on_draw);

  constexpr size_t CHUNK = 4096;
  uint8_t buf[CHUNK];
  bool err = false;
  while (f.available()) {
    const int n = f.read(buf, CHUNK);
    if (n <= 0) break;
    int remaining = n;
    uint8_t* p = buf;
    while (remaining > 0) {
      const int eaten = pngle_feed(png, p, remaining);
      if (eaten < 0) {
        Serial1.printf("[png] decode error: %s\n", pngle_error(png));
        err = true;
        break;
      }
      if (eaten == 0) break;  // need more data
      p         += eaten;
      remaining -= eaten;
    }
    if (err) break;
  }

  const bool ok = !err && !ctx.oom && out->pixels != nullptr;
  pngle_destroy(png);
  if (!ok) {
    if (out->pixels) { free(out->pixels); out->pixels = nullptr; }
    out->width = out->height = 0;
    return false;
  }
  return true;
}

static bool ends_with_ci(const char* s, const char* suf) {
  const size_t ls = strlen(s), lf = strlen(suf);
  if (lf > ls) return false;
  for (size_t i = 0; i < lf; ++i) {
    char a = s[ls - lf + i];
    char b = suf[i];
    if (a >= 'A' && a <= 'Z') a += 32;
    if (b >= 'A' && b <= 'Z') b += 32;
    if (a != b) return false;
  }
  return true;
}

}  // namespace

// =====================================================================================
// public: nearest-neighbor RGB888 resize (replaces img->pixels with a new buffer)
// =====================================================================================
bool resize_image(RgbImage* img, int dst_w, int dst_h) {
  if (!img || !img->pixels || dst_w <= 0 || dst_h <= 0) return false;
  if (img->width == dst_w && img->height == dst_h) return true;

  const size_t sz = (size_t)dst_w * dst_h * 3;
  uint8_t* dst = (uint8_t*)ps_malloc(sz);
  if (!dst) dst = (uint8_t*)malloc(sz);
  if (!dst) { Serial1.println("[scale] OOM"); return false; }

  // Pre-compute x-source LUT to avoid the per-pixel divide.
  int* sx_lut = (int*)malloc(dst_w * sizeof(int));
  if (!sx_lut) { free(dst); Serial1.println("[scale] OOM lut"); return false; }
  for (int x = 0; x < dst_w; ++x) {
    int sx = (x * img->width) / dst_w;
    if (sx >= img->width) sx = img->width - 1;
    sx_lut[x] = sx;
  }
  for (int y = 0; y < dst_h; ++y) {
    int sy = (y * img->height) / dst_h;
    if (sy >= img->height) sy = img->height - 1;
    const uint8_t* srow = img->pixels + (size_t)sy * img->width * 3;
    uint8_t* drow = dst + (size_t)y * dst_w * 3;
    for (int x = 0; x < dst_w; ++x) {
      const int sx = sx_lut[x];
      drow[x * 3 + 0] = srow[sx * 3 + 0];
      drow[x * 3 + 1] = srow[sx * 3 + 1];
      drow[x * 3 + 2] = srow[sx * 3 + 2];
    }
  }
  free(sx_lut);
  free(img->pixels);
  img->pixels = dst;
  img->width  = dst_w;
  img->height = dst_h;
  return true;
}

void image_free(RgbImage* out) {
  if (!out) return;
  if (out->pixels) { free(out->pixels); out->pixels = nullptr; }
  out->width = 0;
  out->height = 0;
}

bool load_image_from_sd(const char* path, int target_w, int target_h, RgbImage* out) {
  if (!path || !out) return false;
  out->pixels = nullptr; out->width = 0; out->height = 0;

  File f = SD.open(path, FILE_READ);
  if (!f) { Serial1.printf("[img] open failed: %s\n", path); return false; }

  // Sniff the actual file format by reading the first 8 bytes -- many users get
  // tripped by files whose extension lies (e.g. a JPEG saved as .bmp by a web tool).
  // We override the extension-based decision when the magic disagrees.
  uint8_t sniff[8] = {0};
  const size_t n = f.read(sniff, 8);
  f.seek(0);
  enum { FMT_UNKNOWN, FMT_JPEG, FMT_BMP, FMT_PNG } fmt = FMT_UNKNOWN;
  if (n >= 2 && sniff[0] == 0xFF && sniff[1] == 0xD8) {
    fmt = FMT_JPEG;   // JPEG SOI
  } else if (n >= 2 && sniff[0] == 'B' && sniff[1] == 'M') {
    fmt = FMT_BMP;    // 'BM'
  } else if (n >= 8 && sniff[0] == 0x89 && sniff[1] == 'P' && sniff[2] == 'N' &&
             sniff[3] == 'G' && sniff[4] == 0x0D && sniff[5] == 0x0A &&
             sniff[6] == 0x1A && sniff[7] == 0x0A) {
    fmt = FMT_PNG;    // 89 PNG \r\n 1a \n
  }

  const bool ext_jpeg = ends_with_ci(path, ".jpg") || ends_with_ci(path, ".jpeg");
  const bool ext_bmp  = ends_with_ci(path, ".bmp");
  const bool ext_png  = ends_with_ci(path, ".png");
  const char* fmt_name = (fmt == FMT_JPEG) ? "JPEG"
                       : (fmt == FMT_BMP)  ? "BMP"
                       : (fmt == FMT_PNG)  ? "PNG"
                       : "UNKNOWN";

  const bool ext_disagrees =
      (ext_jpeg && fmt != FMT_JPEG && fmt != FMT_UNKNOWN) ||
      (ext_bmp  && fmt != FMT_BMP  && fmt != FMT_UNKNOWN) ||
      (ext_png  && fmt != FMT_PNG  && fmt != FMT_UNKNOWN);
  if (ext_disagrees) {
    Serial1.printf("[img] WARNING: file extension says %s but magic bytes are %02X %02X (-> %s). Using magic.\n",
                   ext_jpeg ? "JPEG" : ext_bmp ? "BMP" : "PNG",
                   sniff[0], sniff[1], fmt_name);
  }

  bool ok = false;
  if (fmt == FMT_JPEG || (fmt == FMT_UNKNOWN && ext_jpeg)) {
    ok = decode_jpeg(f, out);
  } else if (fmt == FMT_BMP || (fmt == FMT_UNKNOWN && ext_bmp)) {
    ok = decode_bmp(f, out);
  } else if (fmt == FMT_PNG || (fmt == FMT_UNKNOWN && ext_png)) {
    ok = decode_png(f, out);
  } else {
    Serial1.printf("[img] unsupported format: ext=%s magic=%02X %02X %02X %02X (only JPEG / BMP / PNG are supported)\n",
                   path, sniff[0], sniff[1], sniff[2], sniff[3]);
  }
  f.close();
  if (!ok) return false;

  Serial1.printf("[img] decoded %dx%d\n", out->width, out->height);

  if (target_w > 0 && target_h > 0 && (out->width != target_w || out->height != target_h)) {
    Serial1.printf("[img] resizing %dx%d -> %dx%d\n", out->width, out->height, target_w, target_h);
    if (!resize_image(out, target_w, target_h)) {
      image_free(out);
      return false;
    }
  }
  return true;
}
