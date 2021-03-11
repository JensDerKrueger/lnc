#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "Vec4.h"

class Image {
public:
  uint32_t width;
  uint32_t height;
  uint32_t componentCount;
  std::vector<uint8_t> data;

  Image(const Vec4& color);
  
  Image(uint32_t width = 100,
        uint32_t height = 100,
        uint32_t componentCount = 4);

  Image(uint32_t width,
        uint32_t height,
        uint32_t componentCount,
        std::vector<uint8_t> data);

  void multiply(const Vec4& color);
  void generateAlphaFromLuminance();
  size_t computeIndex(uint32_t x, uint32_t y, uint32_t component) const;
  uint8_t getValue(uint32_t x, uint32_t y, uint32_t component) const;
  void setValue(uint32_t x, uint32_t y, uint32_t component, uint8_t value);
  void setNormalizedValue(uint32_t x, uint32_t y, uint32_t component, float value);
  std::string toCode(const std::string& varName="myImage", bool padding=false) const;
};
