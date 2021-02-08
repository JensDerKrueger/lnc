#include "DenseLayer.h"

DenseLayer::DenseLayer(size_t size, size_t prevSize, Nonlinearity nonlinearity) :
  nonlinearity(nonlinearity)
{
  randomInit(size,prevSize);
}

void DenseLayer::randomInit(size_t size, size_t prevSize) {
  biases = Vec::gaussian(size,0.0f,1.0f);
  weights = Mat::gaussian(prevSize,size,0.0f,1.0f) * (1.0f/sqrtf(prevSize));
}
  
LayerData DenseLayer::feedforward(const LayerData& input) {
  this->input = input;
  Vec z = (weights * input.a + biases);
  
  switch (nonlinearity) {
    case Nonlinearity::Sigmoid :
      return LayerData{z.apply(sigmoid), z};
    case Nonlinearity::Tanh :
      return LayerData{z.apply(tanhf), z};
    default :
      return LayerData{z.apply(reLU), z};
  }
}

LayerUpdate DenseLayer::backprop(Vec& delta, bool updateDelta) {
  LayerUpdate l{delta, Mat::tensorProduct(input.a, delta)};
  
  if (updateDelta) {
    switch (nonlinearity) {
      case Nonlinearity::Sigmoid :
        delta = (weights.transpose() * delta) * input.z.apply(sigmoidPrime);
        break;
      case Nonlinearity::Tanh :
        delta = (weights.transpose() * delta) * input.z.apply(tanhPrime);
        break;
      default :
        delta = (weights.transpose() * delta) * input.z.apply(reLUPrime);
        break;
    }
  }
  
  return l;
}

void DenseLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) {
  const float normEta = eta/bachSize;
  biases  -= update.biases*normEta;
  weights -= update.weights*normEta;
}

void DenseLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) {
  const float normEta = eta/bachSize;
  const float scale = 1.0f - eta * lambda/totalSize;
  biases  -= update.biases*normEta;
  weights  = weights * scale - update.weights*normEta;
}

DenseLayer::DenseLayer(std::ifstream& file) {
  file >> biases;
  file >> weights;
  
  uint32_t i;
  file >> i;
  nonlinearity = Nonlinearity(i);
}

void DenseLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << biases << std::endl;
  file << weights << std::endl;
  file << uint32_t(nonlinearity) << std::endl;
}
