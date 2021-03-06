#include <sstream>
#include <iomanip>

#include "Image.h"

Image::Image(const Vec4& color) :
  Image(1,1,4,{uint8_t(color.x()*255),
               uint8_t(color.y()*255),
               uint8_t(color.z()*255),
               uint8_t(color.w()*255)})
{
}

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

void Image::generateAlphaFromLuminance() {
  if (componentCount == 4) {
    for (size_t i = 0; i<data.size()/4;i++) {
      data[i*4+3] = uint8_t(0.299 * data[i*4+0] + 0.587 * data[i*4+1] + 0.114 * data[i*4+2]);
    }
  } else if (componentCount == 3) {
    std::vector<uint8_t> newData((data.size() / 3) * 4);
    
    for (size_t i = 0; i<data.size()/3;i++) {
      newData[i*4+0] = data[i*3+0];
      newData[i*4+1] = data[i*3+1];
      newData[i*4+2] = data[i*3+2];
      newData[i*4+3] = uint8_t(0.299 * data[i*3+0] + 0.587 * data[i*3+1] + 0.114 * data[i*3+2]);
    }
    
    data = newData;
    componentCount = 4;
  }
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

std::string Image::toCode(const std::string& varName, bool padding) const {
  std::stringstream ss;

  ss << "Image " << varName << " {"<< width << "," << height << ","<< componentCount << ",\n";
  ss << "              {";

  for (size_t i = 0;i<data.size();++i) {
    if (i % 30 == 0) ss << "\n              ";
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
  
  ss << "          }};\n";
  
  return ss.str();
}
