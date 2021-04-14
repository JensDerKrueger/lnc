#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <Vec3.h>

struct Volume {
  Volume() :
    width{0}, height{0}, depth{0},
    scale{0.0f, 0.0f, 0.0f}
  {}

  size_t width;
  size_t height;
  size_t depth;
  size_t maxSize;
  Vec3 scale;

  std::vector<uint8_t> data;
  std::vector<Vec3> normals;
  
  void normalizeScale() {
    maxSize = std::max(width,std::max(height,depth));
    const Vec3 extend = scale*Vec3{float(width),float(height),float(depth)}/maxSize;
    const float m = std::max(extend.x(),std::max(extend.y(),extend.z()));
    scale = scale / m;
  }
  
  std::string toString() const {
    std::stringstream ss;
    ss << "width: " << width << "\n";
    ss << "height: " << height << "\n";
    ss << "depth: " << depth << "\n";
    ss << "dataseize: " << data.size() << "\n";
    ss << "scale: " << scale << "\n";
    
    for (size_t i = 0;i<data.size();++i) {
      if (i > 0 && i % width == 0) ss << "\n";
      if (i > 0 && i % (width*height) == 0) ss << "\n";
      ss << int(data[i])%10 << int(data[i])%10;
    }
    
    return ss.str();
  }
  
  void computeNormals() {
    normals.resize(data.size());
    for (size_t w = 1;w<depth-1;++w) {
      for (size_t v = 1;v<height-1;++v) {
        for (size_t u = 1;u<width-1;++u) {
          const size_t index = u + v * width + w * width * height;
          const Vec3 normal{
            float(data[index-1]) - float(data[index+1]),
            float(data[index-width]) - float(data[index+width]),
            float(data[index-width*height]) - float(data[index+width*height])
          };
          normals[index] = Vec3::normalize(normal);
        }
      }
    }
  }
};
