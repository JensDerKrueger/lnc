#include "MaxPoolLayer.h"

#include <limits>

MaxPoolLayer::MaxPoolLayer(std::ifstream& file) {
  load(file);
}

MaxPoolLayer::MaxPoolLayer(size_t poolWidth, size_t poolHeight, size_t prevFilterCount, size_t prevWidth, size_t prevHeight) :
  poolWidth(poolWidth),
  poolHeight(poolHeight),
  prevFilterCount(prevFilterCount),
  prevWidth(prevWidth),
  prevHeight(prevHeight)
{
}

LayerData MaxPoolLayer::feedforward(const LayerData& input) {
  this->input = input;
  maxIs.clear();
  
  Vec z{prevWidth/poolWidth * prevHeight/poolHeight * prevFilterCount};
  size_t i = 0;
  for (size_t f = 0;f<prevFilterCount;++f) {
    const size_t filterOffset = prevWidth*prevHeight*f;
    for (size_t y = 0;y<prevHeight/poolHeight;++y) {
      for (size_t x = 0;x<prevWidth/poolWidth;++x) {
        z[i] = std::numeric_limits<float>::lowest();
        size_t maxIndex;
        for (size_t v = 0;v<poolHeight;++v) {
          for (size_t u = 0;u<poolWidth;++u) {
            const size_t index = filterOffset+(x*poolWidth+u)+(y*poolHeight+v)*prevWidth;
            if (input.a[index] > z[i]) {
              z[i] = input.a[index];
              maxIndex = index;
            }
          }
        }
        maxIs.push_back(maxIndex);
        i++;
      }
    }
  }
  return LayerData{z, z};
}

LayerUpdate MaxPoolLayer::backprop(Vec& delta) {
  Vec oldDelta = delta;
  delta = Vec(delta.size()*poolHeight*poolWidth);
  for (size_t i = 0;i<maxIs.size();++i){
    delta[maxIs[i]] = oldDelta[i];
  }
  return {Vec{0},Mat{0,0}};
}
  
void MaxPoolLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << poolWidth << std::endl;
  file << poolHeight << std::endl;
  file << prevFilterCount << std::endl;
  file << prevWidth << std::endl;
  file << prevHeight << std::endl;
}

void MaxPoolLayer::load(std::ifstream& file) {
  file >> poolWidth;
  file >> poolHeight;
  file >> prevFilterCount;
  file >> prevWidth;
  file >> prevHeight;
}
