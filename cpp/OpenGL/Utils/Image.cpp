#include <sstream>
#include <iomanip>

#include "Image.h"

Image::Image(uint32_t width,
      uint32_t height,
      uint32_t componentCount) :
  width{width},
  height{height},
  componentCount{componentCount},
  data(size_t(width)*size_t(height)*size_t(componentCount))
{
}

Image::Image(uint32_t width,
      uint32_t height,
      uint32_t componentCount,
      std::vector<uint8_t> data) :
  width{width},
  height{height},
  componentCount{componentCount},
  data(data)
{
}

size_t Image::computeIndex(uint32_t x, uint32_t y, uint32_t component) const {
  return size_t(component)+(size_t(x)+size_t(y)* size_t(width))* size_t(componentCount);
}

uint8_t Image::getValue(uint32_t x, uint32_t y, uint32_t component) const {
  return data[computeIndex(x, y, component)];
}

void Image::setValue(uint32_t x, uint32_t y, uint32_t component, uint8_t value) {
  data[computeIndex(x, y, component)] = value;
}

void Image::setNormalizedValue(uint32_t x, uint32_t y, uint32_t component, float value) {
  const uint8_t iValue{uint8_t(std::max(0.0f, std::min(1.0f, value))*255)};
  data[computeIndex(x, y, component)] = iValue;
}

std::string Image::toCode(bool padding) const {
  std::stringstream ss;
  
  ss << "class MyImage : public Image {\n";
  ss << "public:\n";
  ss << "  MyImage() : \n";
  ss << "    Image("<< width << ",\n";
  ss << "          " << height << ",\n";
  ss << "          " << componentCount << ",\n";
  ss << "          {";

  for (size_t i = 0;i<data.size();++i) {
    if (i % 30 == 0) ss << "\n            ";
    if (padding) {
      ss << std::setfill (' ') << std::setw (3) << int(data[i]);
    } else {
      ss << int(data[i]);
    }
    if (i < data.size()-1)
      ss << ",";
    else
      ss << "\n";
  }
  
  ss << "          }\n";
  ss << "          ) {}\n";
  ss << "} myImage;\n";
  
  return ss.str();
}
