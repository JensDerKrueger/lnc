#include "ConvolutionLayer.h"

ConvolutionLayer::ConvolutionLayer(std::ifstream& file) {
  load(file);
}

ConvolutionLayer::ConvolutionLayer(size_t filterCount,
                                   size_t width, size_t height,
                                   size_t prevWidth, size_t prevHeight) :
filterCount(filterCount),
width(width),
height(height),
prevWidth(prevWidth),
prevHeight(prevHeight)
{
  randomInit();
}

LayerData ConvolutionLayer::feedforward(const LayerData& input) {
  this->input = input;
  
  Vec z{(prevWidth-width)*(prevHeight-height)*filterCount};
  size_t i = 0;
  for (size_t f = 0;f<filterCount;++f) {
    const size_t weightOffset = width*height*f;
    for (size_t y = 0;y<prevHeight-height;++y) {
      for (size_t x = 0;x<prevWidth-width;++x) {
        for (size_t v = 0;v<height;++v) {
          for (size_t u = 0;u<width;++u) {
            const size_t index = (x+u)+(y+v)*prevWidth;
            z[i] += weights[weightOffset+u+v*width] * input.a[index];
          }
        }
        z[i] += biases[f];
        i++;
      }
    }
  }
  return LayerData{z.apply(reLU), z};
}

LayerUpdate ConvolutionLayer::backprop(Vec& delta) {
  Vec deltaBias{filterCount};
   
  size_t i = 0;
  for (size_t f = 0;f<filterCount;++f) {
    for (size_t j = 0;j<(prevHeight-height)*(prevWidth-width);++j) {
      deltaBias[f] += delta[i++];
    }
  }
 
  Mat deltaWeights{width*height,filterCount};
  
  i = 0;
  for (size_t f = 0;f<filterCount;++f) {
    const size_t weightOffset = (prevWidth-width)*(prevHeight-height)*f;
    for (size_t y = 0;y<height;++y) {
      for (size_t x = 0;x<width;++x) {
        
        for (size_t v = 0;v<prevHeight-height;++v) {
          for (size_t u = 0;u<prevWidth-width;++u) {
            const size_t index = (x+u)+(y+v)*prevWidth;
            deltaWeights[i] += delta[weightOffset+u+v*(prevWidth-width)] * input.a[index];
          }
        }
        i++;
      }
    }
  }
  
  LayerUpdate l{deltaBias, deltaWeights};

 
  // TODO
  // update delta
  
  return l;
}
  
void ConvolutionLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << filterCount << std::endl;
  file << width << std::endl;
  file << height << std::endl;
  file << prevWidth << std::endl;
  file << prevHeight << std::endl;
  file << biases << std::endl;
  file << weights << std::endl;
}

void ConvolutionLayer::load(std::ifstream& file) {
  file >> filterCount;
  file >> width;
  file >> height;
  file >> prevWidth;
  file >> prevHeight;
  file >> biases;
  file >> weights;
}

void ConvolutionLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) {
  const float normEta = eta/bachSize;
  biases  -= update.biases*normEta;
  weights -= update.weights*normEta;
}

void ConvolutionLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) {
  const float normEta = eta/bachSize;
  const float scale = 1.0f - eta * lambda/totalSize;
  biases  -= update.biases*normEta;
  weights  = weights * scale - update.weights*normEta;
}

void ConvolutionLayer::randomInit() {
  biases = Vec::gaussian(filterCount,0.0f,1.0f);
  weights = Mat::gaussian(width*height,filterCount,0.0f,1.0f) * (1.0f/sqrtf(width*height));
}
