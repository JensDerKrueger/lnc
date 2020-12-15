#include "DenseLayer.h"

DenseLayer::DenseLayer(size_t size, size_t prevSize) {
  randomInit(size,prevSize);
}

void DenseLayer::randomInit(size_t size, size_t prevSize) {
  biases = Vec::gaussian(size,0.0f,1.0f);
  weights = Mat::gaussian(prevSize,size,0.0f,1.0f) * (1.0f/sqrtf(prevSize));
}
  
LayerData DenseLayer::feedforward(const LayerData& input) {
  this->input = input;
  Vec z = (weights * input.a + biases);
  return LayerData{z.apply(sigmoid), z};
}

LayerUpdate DenseLayer::backprop(Vec& delta, bool updateDelta) {
  LayerUpdate l{delta, Mat::tensorProduct(input.a, delta)};
  if (updateDelta)
    delta = (weights.transpose() * delta) * input.z.apply(sigmoidPrime);
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
}

void DenseLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << biases << std::endl;
  file << weights << std::endl;
}
