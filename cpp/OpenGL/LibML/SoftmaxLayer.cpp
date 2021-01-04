#include "SoftmaxLayer.h"

#include <limits>

SoftmaxLayer::SoftmaxLayer(size_t size, size_t prevSize)
{
  randomInit(size,prevSize);
}

void SoftmaxLayer::randomInit(size_t size, size_t prevSize) {
  biases = Vec::gaussian(size,0.0f,1.0f);
  weights = Mat::gaussian(prevSize,size,0.0f,1.0f) * (1.0f/sqrtf(prevSize));
}
  
LayerData SoftmaxLayer::feedforward(const LayerData& input) {
  this->input = input;
  Vec z = (weights * input.a + biases);
  return LayerData{z.softmax(), z};
}

LayerUpdate SoftmaxLayer::backprop(Vec& delta, bool updateDelta) {
  LayerUpdate l{delta, Mat::tensorProduct(input.a, delta)};
  delta = input.z.softmaxPrime() * (weights.transpose() * delta);
  return l;
}

void SoftmaxLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) {
  const float normEta = eta/bachSize;
  biases  -= update.biases*normEta;
  weights -= update.weights*normEta;
}

void SoftmaxLayer::applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) {
  const float normEta = eta/bachSize;
  const float scale = 1.0f - eta * lambda/totalSize;
  biases  -= update.biases*normEta;
  weights  = weights * scale - update.weights*normEta;
}

SoftmaxLayer::SoftmaxLayer(std::ifstream& file) {
  file >> biases;
  file >> weights;
}

void SoftmaxLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << biases << std::endl;
  file << weights << std::endl;
}
