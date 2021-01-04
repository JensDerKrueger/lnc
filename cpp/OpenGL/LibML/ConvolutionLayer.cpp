#include "ConvolutionLayer.h"

ConvolutionLayer::ConvolutionLayer(std::ifstream& file) {
  load(file);
}

ConvolutionLayer::ConvolutionLayer(size_t filterCount, size_t width, size_t height,
                                   size_t channelCount, size_t prevWidth, size_t prevHeight,
                                   Nonlinearity nonlinearity) :
filterCount(filterCount),
width(width),
height(height),
channelCount(channelCount),
prevWidth(prevWidth),
prevHeight(prevHeight),
outWidth{1+prevWidth-width},
outHeight{1+prevHeight-height},
nonlinearity{nonlinearity}
{
  randomInit();
}

LayerData ConvolutionLayer::feedforward(const LayerData& input) {
  this->input = input;
    
  Vec z{outWidth*outHeight*filterCount};
  for (size_t f = 0;f<filterCount;++f) {
    for (size_t y = 0;y<outHeight;++y) {
      for (size_t x = 0;x<outWidth;++x) {
        for (size_t c = 0;c<channelCount;++c) {
          for (size_t v = 0;v<height;++v) {
            for (size_t u = 0;u<width;++u) {
              z[x+outWidth*y+outWidth*outHeight*f] += weights[u+v*width+width*height*c+width*height*channelCount*f] * input.a[(x+u)+(y+v)*prevWidth+c*prevWidth*prevHeight];
            }
          }
        }
        z[x+outHeight*y+outWidth*outHeight*f] += biases[f];
      }
    }
  }
  
  switch (nonlinearity) {
    case Nonlinearity::Sigmoid :
      return LayerData{z.apply(sigmoid), z};
    case Nonlinearity::Tanh :
      return LayerData{z.apply(tanh), z};
    default :
      return LayerData{z.apply(reLU), z};
  }
}

float ConvolutionLayer::padded(const Vec& delta, size_t u, size_t v, size_t f) {
  const size_t padX{width-1};
  const size_t padY{height-1};
  
  if (u<padX || u-padX >= outWidth) return 0.0f;
  if (v<padY || v-padY >= outHeight) return 0.0f;
  
  return delta[(u-padX) + (v-padY) * outWidth + f*outWidth*outHeight];
}

LayerUpdate ConvolutionLayer::backprop(Vec& delta, bool updateDelta) {  
  // compute bias update
  Vec deltaBias{filterCount};
  for (size_t f = 0;f<filterCount;++f) {
    for (size_t j = 0;j<outWidth*outHeight;++j) {
      deltaBias[f] += delta[j+outWidth*outHeight*f];
    }
  }
    
  // compute weight update
  Mat deltaWeights{width*height,channelCount*filterCount};
  for (size_t f = 0;f<filterCount;++f) {
    for (size_t y = 0;y<height /* <- prevheight - (prevheight-height+1) + 1 */ ;++y) {
      for (size_t x = 0;x<width /* <- prevWidth - (prevWidth-width+1) + 1 */ ;++x) {
        for (size_t c = 0;c<channelCount;++c) {
          for (size_t v = 0;v<outHeight;++v) {
            for (size_t u = 0;u<outWidth;++u) {
              deltaWeights[x + y*width + c*width*height + f*width*height*channelCount] += delta[u+v*outWidth+outWidth*outHeight*f] * input.a[(x+u)+(y+v)*prevWidth + c*prevWidth*prevHeight];
            }
          }
        }
      }
    }
  }
  
  // compute new delta
  if (updateDelta) {
    Vec oldDelta = delta;
    
    delta = Vec(prevWidth*prevHeight*channelCount);

    // rotate Filter
    Mat rotWeights = weights;
    for (size_t f = 0;f<filterCount ;++f) {
      for (size_t c = 0;c<channelCount ;++c) {
        for (size_t y = 0;y<height ;++y) {
          for (size_t x = 0;x<width;++x) {
            rotWeights[x+y*width+width*height*c+width*height*channelCount*f] = weights[((width-1)-x) + width * ((height-1)-y) + width*height*c + width*height*channelCount*f];
          }
        }
      }
    }

    for (size_t c = 0;c<channelCount;++c) {
      for (size_t y = 0;y<prevHeight;++y) {
        for (size_t x = 0;x<prevWidth;++x) {
          for (size_t f = 0;f<filterCount;++f) {
            for (size_t v = 0;v<height;++v) {
              for (size_t u = 0;u<width;++u) {
                delta[x+y*prevWidth+c*prevWidth*prevWidth] += padded(oldDelta, x+u, y+v, f) * rotWeights[u+width*v + width*height*c + width*height*channelCount*f];
              }
            }
          }
        }
      }
    }
        
    switch (nonlinearity) {
      case Nonlinearity::Sigmoid :
        delta = delta * input.z.apply(sigmoidPrime);
        break;
      case Nonlinearity::Tanh :
        delta = delta * input.z.apply(tanhPrime);
        break;
      default :
        delta = delta * input.z.apply(reLUPrime);
        break;
    }
  }
  return {deltaBias, deltaWeights};
}
  
void ConvolutionLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << filterCount << std::endl;
  file << width << std::endl;
  file << height << std::endl;
  file << prevWidth << std::endl;
  file << prevHeight << std::endl;
  file << channelCount << std::endl;
  file << biases << std::endl;
  file << weights << std::endl;
  file << uint32_t(nonlinearity) << std::endl;
}

void ConvolutionLayer::load(std::ifstream& file) {
  file >> filterCount;
  file >> width;
  file >> height;
  file >> prevWidth;
  file >> prevHeight;
  file >> channelCount;
  file >> biases;
  file >> weights;
  
  uint32_t i;
  file >> i;
  nonlinearity = Nonlinearity(i);
  
  outWidth = 1+prevWidth-width;
  outHeight = 1+prevHeight-height;
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
  // zero init
  //biases = Vec{filterCount};
  
  // gaussian init
  biases = Vec::gaussian(filterCount,0.0f,1.0f);
  weights = Mat::gaussian(width*height,channelCount*filterCount,0.0f,1.0f) * (2.0f/sqrtf(width*height*channelCount));
}
