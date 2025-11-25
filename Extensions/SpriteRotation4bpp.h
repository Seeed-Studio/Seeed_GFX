#pragma once

#include <stdint.h>

// Helper utilities to keep the 4bpp sprite rotation logic separate from the core Sprite
// implementation. All functions live in this header so the translation unit that includes
// Sprite.cpp also gets their definitions.
class SpriteRotation4bpp
{
public:
  static inline uint8_t Normalize(uint8_t rotation)
  {
    return rotation & 0x03;
  }

  static inline bool IsActive(uint8_t rotation)
  {
    return Normalize(rotation) != 0;
  }

  static inline bool SwapsAxis(uint8_t rotation)
  {
    return (Normalize(rotation) & 0x01) != 0;
  }

  static inline bool Apply(uint8_t rotation, int32_t width, int32_t height, int32_t &x, int32_t &y)
  {
    rotation = Normalize(rotation);
    if (rotation == 0) {
      return inBounds(x, y, width, height);
    }

    int32_t tx = x;
    if (rotation == 1) {
      x = width - y - 1;
      y = tx;
    }
    else if (rotation == 2) {
      x = width - x - 1;
      y = height - y - 1;
    }
    else { // rotation == 3
      x = y;
      y = height - tx - 1;
    }

    return inBounds(x, y, width, height);
  }

private:
  static inline bool inBounds(int32_t x, int32_t y, int32_t width, int32_t height)
  {
    return (x >= 0) && (y >= 0) && (x < width) && (y < height);
  }
};
