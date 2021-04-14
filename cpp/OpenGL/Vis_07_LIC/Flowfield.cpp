#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Flowfield.h"

static std::string getElem(std::ifstream& s, char sep) {
  std::string result;
  while (s.good()) {
    char c = s.get();
    if (c == sep) break;
    result += c;
  }
  return result;
}

Flowfield Flowfield::fromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::stringstream s;
    s << "Can't open file " << filename;
    throw std::runtime_error(s.str());
  }

  std::string elem{""};

  size_t dims = std::stoi(getElem(file,','));
  size_t sizeX{1}, sizeY{1}, sizeZ{1};
  
  if (dims < 1 || dims > 3) {
    std::stringstream s;
    s << "Invalid dimenion " << dims;
    throw std::runtime_error(s.str());
  }
  
  sizeX = std::stoi(getElem(file,','));
  if (dims == 2)
    sizeY = std::stoi(getElem(file,','));
  if (dims == 3)
    sizeZ = std::stoi(getElem(file,','));
  
  int timesteps = std::stoi(getElem(file,','));
  if (timesteps < 1) {
    std::stringstream s;
    s << "Invalid timesteps " << timesteps;
    throw std::runtime_error(s.str());
  }

  Flowfield f{sizeX,sizeY,sizeZ};
  size_t index{0};
  float x{0},y{0},z{0};

  while ( index < sizeX*sizeY*sizeZ) {
    x = std::stof(getElem(file,','));
    if (dims == 2)
      y = std::stof(getElem(file,','));
    if (dims == 3)
      z = std::stof(getElem(file,','));
    
    f.data[index++] = Vec3{x,y,z};
  }
  
  return f;
}

Flowfield Flowfield::genDemo(size_t size, DemoType d) {
  Flowfield f{size,size,size};
  
  switch (d) {
    case DemoType::DRAIN :
      {
        for (size_t z = 0;z<size;++z) {
          const float localZ = float(z)/size;
          for (size_t y = 0;y<size;++y) {
            const float localY = float(y)/size;
            for (size_t x = 0;x<size;++x) {
              const float localX = float(x)/size;
              f.data[x+y*size+z*size*size] = Vec3{(-localY+0.5f)+(0.5f-localX)/10.0f,(localX-0.5f)+(0.5f-localY)/10.0f,-localZ/10.0f};
            }
          }
        }
      }
      break;
    case DemoType::SATTLE :
      {
        for (size_t z = 0;z<size;++z) {
          const float localZ = float(z)/size;
          for (size_t y = 0;y<size;++y) {
            const float localY = float(y)/size;
            for (size_t x = 0;x<size;++x) {
              const float localX = float(x)/size;
              f.data[x+y*size+z*size*size] = Vec3{0.5f-localX,localY-0.5f,0.5f-localZ};
            }
          }
        }
      }
      break;
    case DemoType::CRITICAL :
      {
        for (size_t z = 0;z<size;++z) {
          const float localZ = float(z)/size;
          for (size_t y = 0;y<size;++y) {
            const float localY = float(y)/size;
            for (size_t x = 0;x<size;++x) {
              const float localX = float(x)/size;
              f.data[x+y*size+z*size*size] = Vec3{(localX-0.1f)*(localY-0.3f)*(localX-0.8f),(localY-0.7f)*(localZ-0.2f)*(localX-0.3f),(localZ-0.9f)*(localZ-0.6f)*(localX-0.5f)};
            }
          }
        }
      }
      break;
  }
  
  return f;
}

Flowfield::Flowfield(size_t sizeX, size_t sizeY, size_t sizeZ) :
sizeX(sizeX),
sizeY(sizeY),
sizeZ(sizeZ)
{
  data.resize(sizeX*sizeY*sizeZ);
}

Vec3 Flowfield::getData(size_t x, size_t y, size_t z) {
  return data[x+y*sizeY+z*sizeX*sizeY];
}

Vec3 Flowfield::linear(const Vec3& a, const Vec3& b, float alpha) {
  return a * (1.0f - alpha) + b * alpha;
}

Vec3 Flowfield::interpolate(const Vec3& pos) {
  const size_t fX = size_t(floor(pos.x() * (sizeX-1)));
  const size_t fY = size_t(floor(pos.y() * (sizeY-1)));
  const size_t fZ = size_t(floor(pos.z() * (sizeZ-1)));
  
  const size_t cX = size_t(ceil(pos.x() * (sizeX-1)));
  const size_t cY = size_t(ceil(pos.y() * (sizeY-1)));
  const size_t cZ = size_t(ceil(pos.z() * (sizeZ-1)));


  const std::array<Vec3, 8> values = {
    getData(fX,fY,fZ),
    getData(cX,fY,fZ),
    getData(fX,cY,fZ),
    getData(cX,cY,fZ),
    getData(fX,fY,cZ),
    getData(cX,fY,cZ),
    getData(fX,cY,cZ),
    getData(cX,cY,cZ)
  };
  
  const float alpha = pos.x() * (sizeX-1) - fX;
  const float beta  = pos.y() * (sizeY-1) - fY;
  const float gamma = pos.z() * (sizeZ-1) - fZ;
    
  return linear(linear(linear(values[0], values[1], alpha),
                       linear(values[2], values[3], alpha),
                       beta),
                linear(linear(values[4], values[5], alpha),
                       linear(values[6], values[7], alpha),
                       beta),
                gamma);
}
