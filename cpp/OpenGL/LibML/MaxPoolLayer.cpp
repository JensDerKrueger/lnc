#include "MaxPoolLayer.h"

#include <limits>

MaxPoolLayer::MaxPoolLayer(std::ifstream& file) {
  load(file);
}

MaxPoolLayer::MaxPoolLayer(size_t poolWidth, size_t poolHeight, size_t channelCount, size_t prevWidth, size_t prevHeight) :
  poolWidth(poolWidth),
  poolHeight(poolHeight),
  channelCount(channelCount),
  prevWidth(prevWidth),
  prevHeight(prevHeight)
{
}

LayerData MaxPoolLayer::feedforward(const LayerData& input) {
  this->input = input;
  maxIs.clear();
  
  Vec z{outputWidth() * outputHeight() * channelCount};
  for (size_t c = 0;c<channelCount;++c) {
    for (size_t y = 0;y<outputHeight();++y) {
      for (size_t x = 0;x<outputWidth();++x) {
        const size_t zIndex = x + y*outputHeight() + outputWidth()*outputHeight()*c;
        z[zIndex] = std::numeric_limits<float>::lowest();
        size_t maxIndex;
        for (size_t v = 0;v<poolHeight;++v) {
          for (size_t u = 0;u<poolWidth;++u) {
            const size_t index = (x*poolWidth+u)+(y*poolHeight+v)*prevWidth + prevWidth*prevHeight*c;
            if (input.a[index] > z[zIndex]) {
              z[zIndex] = input.a[index];
              maxIndex = index;
            }
          }
        }
        maxIs.push_back(maxIndex);
      }
    }
  }
  return LayerData{z, z};
}

LayerUpdate MaxPoolLayer::backprop(Vec& delta, bool updateDelta) {
  if (updateDelta) {
    Vec oldDelta = delta;
    delta = Vec(delta.size()*poolHeight*poolWidth);
    for (size_t i = 0;i<maxIs.size();++i){
      delta[maxIs[i]] = oldDelta[i];
    }
  }
  return {Vec{0},Mat{0,0}};
}
  
void MaxPoolLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << poolWidth << std::endl;
  file << poolHeight << std::endl;
  file << channelCount << std::endl;
  file << prevWidth << std::endl;
  file << prevHeight << std::endl;
}

void MaxPoolLayer::load(std::ifstream& file) {
  file >> poolWidth;
  file >> poolHeight;
  file >> channelCount;
  file >> prevWidth;
  file >> prevHeight;
}
