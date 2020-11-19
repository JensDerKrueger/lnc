#pragma once

struct Image {
  uint32_t width;
  uint32_t height;
  uint32_t componentCount;
  std::vector<uint8_t> data;

  Image(uint32_t width = 100,
        uint32_t height = 100,
        uint32_t componentCount = 4,
        std::vector<uint8_t> data=std::vector<uint8_t>{}) :
    width{width},
    height{height},
    componentCount{componentCount},
    data(width*height*componentCount)
  {
  }
  
  size_t computeIndex(uint32_t x, uint32_t y, uint32_t component) const {
    return component+(x+y*width)*componentCount;
  }
  
  uint8_t getValue(uint32_t x, uint32_t y, uint32_t component) const {
    return data[computeIndex(x, y, component)];
  }

  void setValue(uint32_t x, uint32_t y, uint32_t component, uint8_t value) {
    data[computeIndex(x, y, component)] = value;
  }

  void setNormalizedValue(uint32_t x, uint32_t y, uint32_t component, float value) {
    const uint8_t iValue{uint8_t(std::max(0.0f, std::min(1.0f, value))*255)};
    data[computeIndex(x, y, component)] = iValue;
  }
};
