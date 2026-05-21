// Image dithering / quantization for reTerminal e-paper devices.
//
// Ported from the in-browser tool `online_img2bitmap_.html` (BW / Gray4 / Gray16 / E4 / E6
// palettes with None / Bayer8 / Floyd-Steinberg / Jarvis / Atkinson dithering). The C++
// version reproduces the same pixel-by-pixel logic so previewing in the web tool and the
// on-device result match.
//
// Input: 24-bit RGB888 pixel buffer (row-major, no stride).
// Output: 1 byte per pixel, value depends on the chosen palette (see DitherPalette).
#pragma once

#include <Arduino.h>

enum DitherMethod {
  DITHER_NONE = 0,      // 直接最近色，无抖动
  DITHER_BAYER8,        // 有序 Bayer 8x8
  DITHER_FS,            // Floyd-Steinberg
  DITHER_JARVIS,        // Jarvis-Judice-Ninke
  DITHER_ATKINSON,      // Atkinson
};

enum DitherPalette {
  PAL_BW = 0,           // 1bpp: 0=black, 1=white
  PAL_GRAY4,            // 2bpp: 0..3 (0=black, 1=dark, 2=light, 3=white), luminance step +85
  PAL_GRAY16,           // 4bpp: 0..15 (0=black, 15=white), each step +17 in luminance
  PAL_E6,               // 4bpp E-Ink: 0x0=W, 0x2=G, 0x6=R, 0xB=Y, 0xD=B, 0xF=BK (raw codes)
};

// Returns true on success. On PSRAM allocation failure for the error-diffusion buffer the
// function transparently falls back to undithered nearest-color quantization and still
// returns true. out_index must be `width * height` bytes.
//
// BW mode honors `invert` (flip 0/1) and `gamma`.
// Gray4 / Gray16 / E6 modes ignore `invert`.
bool dither_image(const uint8_t* rgb888, int width, int height,
                  DitherPalette palette, DitherMethod method,
                  float gamma, bool invert,
                  uint8_t* out_index);

// 1bpp packing helper, MSB-first per byte (matches `pack1bpp()` in the HTML tool and
// the `drawBitmap()` API of TFT_eSPI).
// `bit_for_black` controls whether index 0 (black) becomes bit 0 or bit 1.
void pack_1bpp_msb(const uint8_t* bw_index, uint8_t* out_bits,
                   int width, int height, bool bit_for_black);
