#include <sstream>
#include <iomanip>

#include "Image.h"
#include "Grid2D.h"

Image::Image(const Vec4& color) :
  Image(1,1,4,{uint8_t(color.x*255),
               uint8_t(color.y*255),
               uint8_t(color.z*255),
               uint8_t(color.w*255)})
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

void Image::multiply(const Vec4& color) {
  if (componentCount == 4) {
    for (size_t i = 0; i<data.size()/4;i++) {
      data[i*4+0] = uint8_t(data[i*4+0] * color.r);
      data[i*4+1] = uint8_t(data[i*4+1] * color.g);
      data[i*4+2] = uint8_t(data[i*4+2] * color.b);
      data[i*4+3] = uint8_t(data[i*4+3] * color.a);
    }
  } else if (componentCount == 3) {
    std::vector<uint8_t> newData((data.size() / 3) * 4);
    
    for (size_t i = 0; i<data.size()/3;i++) {
      newData[i*4+0] = uint8_t(data[i*3+0] * color.r);
      newData[i*4+1] = uint8_t(data[i*3+1] * color.g);
      newData[i*4+2] = uint8_t(data[i*3+2] * color.b);
      newData[i*4+3] = uint8_t(255 * color.a);
    }
    
    data = newData;
    componentCount = 4;
  }
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

std::string Image::toACIIArt(bool bSmallTable) const {
  const std::string lut1{"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "};
  const std::string lut2{"@%#*+=-:. "};
  const std::string& lut = bSmallTable ? lut2 : lut1;

  std::stringstream ss;
  for (uint32_t y = 0;y<height;y+=4) {
    for (uint32_t x = 0;x<width;x+=4) {
      const uint8_t v = getLumiValue(x,height-1-y);
      ss << lut[(v*lut.length())/255] << lut[(v*lut.length())/255];
    }
    ss << "\n";
  }
  return ss.str();
}

uint8_t Image::getLumiValue(uint32_t x, uint32_t y) const {
  switch (componentCount) {
    case 1 : return getValue(x,y,0);
    case 2 : return uint8_t(getValue(x,y,0)*0.5f + getValue(x,y,1)*0.5f);
    case 3 :
    case 4 : return uint8_t(getValue(x,y,0)*0.299f + getValue(x,y,1)*0.587f + getValue(x,y,2)*0.114f);
    default : return 0;
  }
}

Image Image::filter(const Grid2D& filter) const {
  Image filteredImage{width, height, componentCount};
  
  const uint32_t hw = uint32_t(filter.getWidth()/2);
  const uint32_t hh = uint32_t(filter.getHeight()/2);
  
  for (uint32_t y = hh;y<height-hh;y+=1) {
    for (uint32_t x = hw;x<width-hw;x+=1) {
      for (uint32_t c = 0;c<componentCount;c+=1) {
        float conv = 0.0f;
        for (uint32_t u = 0;u<filter.getHeight();u+=1) {
          for (uint32_t v = 0;v<filter.getWidth();v+=1) {
            conv += float(getValue((x+u-hw),(y+v-hh),c)) * filter.getValue(u, v);
          }
        }
        filteredImage.setValue(x,y,c,uint8_t(fabs(conv)));
      }
    }
  }
  
  return filteredImage;
}

Image Image::toGrayscale() const {
  Image grayScaleImage{width,height,1};
  for (uint32_t y = 0;y<height;++y) {
    for (uint32_t x = 0;x<width;++x) {
      grayScaleImage.setValue(x,y,0,getLumiValue(x,y));
    }
  }
  return grayScaleImage;
}


Image Image::genTestImage(uint32_t width,
                          uint32_t height) {
  
  const uint32_t partY1 = height/3;
  const uint32_t partY2 = height*2/3;
  const uint32_t partX1 = width/3;
  const uint32_t partX2 = width*2/3;
  
  Image result{width,height,4};
  for (uint32_t y = 0;y<height;++y) {
    for (uint32_t x = 0;x<width;++x) {
      if (x<partX2) {
        uint8_t r = y < partY1;
        uint8_t g = y >= partY1 && y < partY2;
        uint8_t b = y >= partY2;
        if (x>=partX1) {
          r = 1-r;
          g = 1-g;
          b = 1-b;
        }
        result.setValue(x,y,0,r*255);
        result.setValue(x,y,1,g*255);
        result.setValue(x,y,2,b*255);
      } else {
        uint8_t l = uint8_t(255*((y >= partY1)*0.5 + (y >= partY2)*0.5));
        result.setValue(x,y,0,l);
        result.setValue(x,y,1,l);
        result.setValue(x,y,2,l);
      }
      result.setValue(x,y,3,255);

      
    }
  }

  return result;
}

uint8_t Image::linear(uint8_t a, uint8_t b, float alpha) const {
  return uint8_t(a * (1.0f - alpha) + b * alpha);
}

uint8_t Image::sample(float x, float y, uint32_t component) const {
  const uint32_t fX = size_t(floor(x * (width-1)));
  const uint32_t fY = size_t(floor(y * (height-1)));
  
  const uint32_t cX = size_t(ceil(x * (width-1)));
  const uint32_t cY = size_t(ceil(y * (height-1)));

  const std::array<uint8_t, 4> values = {
    getValue(fX,fY,component),
    getValue(cX,fY,component),
    getValue(fX,cY,component),
    getValue(cX,cY,component)
  };

  const float alpha = x * (width-1) - fX;
  const float beta  = y * (height-1) - fY;

  return linear(linear(values[0], values[1], alpha),
                linear(values[2], values[3], alpha),
                beta);
}

Image Image::resample(uint32_t newWidth) const {
  const uint32_t newHeight = uint32_t(newWidth * float(height)/float(width));
  Image result{newWidth, newHeight, componentCount};

  for (uint32_t y = 0;y<newHeight;++y) {
    for (uint32_t x = 0;x<newWidth;++x) {
      for (uint32_t c = 0;c<componentCount;++c) {
        result.setValue(x,y,c,sample(x/float(newWidth), y/float(newHeight), c));
      }
    }
  }
  return result;
}


Image Image::cropToAspectAndResample(uint32_t newWidth, uint32_t newHeight) const {
  if (newWidth == width && newHeight == height)
    return Image(width, height, componentCount, data);
  
  const float aspect    = float(width)/float(height);
  const float newAspect = float(newWidth)/float(newHeight);
  
  if (aspect == newAspect)
    return resample(newWidth);

  Image result{newWidth, newHeight, componentCount};

  const uint32_t startX = (aspect > newAspect) ? uint32_t(width*((1.0f-newAspect/(aspect))/2.0))  : 0;
  const uint32_t startY = (aspect < newAspect) ? uint32_t(height*((1.0f-aspect/(newAspect))/2.0)) : 0;
  
  const float relWidthInImage = (width-2.0f*startX) / width;
  const float relHeightInImage = (height-2.0f*startY) / height;
  
  const float offsetX = startX/float(width);
  const float offsetY = startY/float(height);

  for (uint32_t y = 0;y<newHeight;++y) {
    const float posY = offsetY + (y/float(newHeight)) * relHeightInImage;
    for (uint32_t x = 0;x<newWidth;++x) {
      const float posX = offsetX + (x/float(newWidth)) * relWidthInImage;
      
      for (uint32_t c = 0;c<componentCount;++c) {
        result.setValue(x,y,c,sample(posX, posY, c));
      }
    }
  }

  return result;
}


Image Image::crop(uint32_t blX, uint32_t blY, uint32_t trX, uint32_t trY) const {
  size_t i = 0;
  Image result{trX-blX, trY-blY, componentCount};
  for (uint32_t y = blY;y<trY;++y) {
    for (uint32_t x = blX;x<trX;++x) {
      for (uint32_t c = 0;c<componentCount;++c) {
        result.data[i++] = getValue(x,y,c);
      }
    }
  }
  return result;
}
