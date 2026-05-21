// Minimal SD-card image loader for the reTerminal SD-card examples.
//
// Supported inputs:
//   - JPEG  (.jpg / .jpeg)   baseline 8-bit, YCbCr or grayscale, any chroma subsample
//   - BMP   (.bmp)           24-bit BGR uncompressed, or 4-bit indexed (palette + BI_RGB).
//                             4-bit indexed BMP is exactly what the online_img2bitmap_
//                             tool emits, so SD-prep <-> on-device display can stay
//                             byte-for-byte consistent.
//   - PNG   (.png)           all standard PNG types via the bundled pngle library
//                             (pngle.{h,c} + miniz.{h,c} in this folder). RGBA images
//                             are alpha-composited over a white background.
//
// The actual format is detected by magic bytes, so a file with a misleading
// extension (e.g. a JPEG saved as `.bmp`) is still decoded correctly and a
// warning is printed to Serial1.
//
// Output is always RGB888 in PSRAM, optionally resized to a target resolution with
// nearest-neighbor (good enough for e-paper, which is itself heavily quantized).
//
// On any failure the function returns false and prints a short reason to Serial1
// (UART1 on GPIO43/44, the reTerminal's hardware UART).
#pragma once

#include <Arduino.h>

struct RgbImage {
  uint8_t* pixels = nullptr;   // RGB888 row-major, owns the buffer (free with image_free)
  int width = 0;
  int height = 0;
};

// `target_w`/`target_h` == 0 means "keep source size".
// If both are non-zero and they differ from the decoded size, the loader resizes
// (nearest-neighbor) into a freshly-allocated PSRAM buffer.
bool load_image_from_sd(const char* path, int target_w, int target_h, RgbImage* out);

// Always safe to call (no-op when out is null/empty).
void image_free(RgbImage* out);

// Resize an already-loaded RGB888 image to (dst_w, dst_h) using nearest-neighbor.
// On success, replaces img->pixels with a fresh buffer (the old one is freed).
// No-op when the requested size already matches.
bool resize_image(RgbImage* img, int dst_w, int dst_h);
