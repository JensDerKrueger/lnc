#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork(const std::shared_ptr<InputLayer> inputLayer, const std::vector<std::shared_ptr<Layer>>& layers) :
  inputLayer(inputLayer),
  layers(layers)
{
}

NeuralNetwork::NeuralNetwork(const std::string& filename) {
  load(filename);
}

void NeuralNetwork::load(const std::string& filename) {
  std::ifstream file{filename};
  if (!file) throw FileException{std::string("Unable to read file ")+filename};
  

  std::string layerId;
  file >> layerId;
  if (InputLayer::id() != layerId)
    throw FileException{std::string("Invalid file ")+filename+std::string(". First Layer must be an input layer.")};
  inputLayer = std::make_shared<InputLayer>(file);
  
  uint64_t s;
  file >> s;
  layers.clear();
  for (size_t i = 0;i<s;++i) {
    file >> layerId;
    if (layerId == ConvolutionLayer::id())
      layers.push_back(std::make_shared<ConvolutionLayer>(file));
    if (layerId == MaxPoolLayer::id())
      layers.push_back(std::make_shared<MaxPoolLayer>(file));
    if (layerId == DenseLayer::id())
      layers.push_back(std::make_shared<DenseLayer>(file));
  }

  file.close();
}

void NeuralNetwork::save(const std::string& filename) const {
  std::ofstream file{filename};
  if (!file) throw FileException{std::string("Unable to write file ")+filename};
  
  inputLayer->save(file);

  file << layers.size() << std::endl;
  for (size_t i = 0;i<layers.size();++i) {
    layers[i]->save(file);
  }
  
  file.close();
}

LayerData NeuralNetwork::feedforwardInt(const Vec& input) {
  LayerData l = inputLayer->feedforward(input);
  for (size_t i = 0;i<layers.size();++i) {
    l = layers[i]->feedforward(l);
  }
  return l;
}

NetworkUpdate NeuralNetwork::backpropagation(const Vec& input, const Vec& groundTruth) {
  NetworkUpdate update;

  // feed forward
  LayerData l = feedforwardInt(input);
  
  // backprop last layer
  const size_t ls = layers.size();
  Vec delta = costDelta(l.z, l.a, groundTruth);
  update.layers.push_back(layers[ls-1]->backprop(delta, true));
  
  // backprop remaining layers
  for (size_t l = 0;l<layers.size()-1;++l) {
    update.layers.push_back(layers[ls-l-2]->backprop(delta, l > 0));
  }
  return update;
}

void NeuralNetwork::applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize) {
  const size_t ls = layers.size();
  for (size_t i = 0;i<ls;++i) {
    layers[i]->applyUpdate(update.layers[ls-i-1], eta, bachSize);
  }
}

void NeuralNetwork::applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) {
  const size_t ls = layers.size();
  for (size_t i = 0;i<ls;++i) {
    layers[i]->applyUpdate(update.layers[ls-i-1], eta, bachSize, lambda, totalSize);
  }
}

float NeuralNetwork::cost(const Vec& activation, const Vec& groundTruth) {
  // cross entropy cost model
  float sum = 0.0f;
  for (size_t i = 0;i<activation.size();++i) {
    sum -= groundTruth[i] * log(activation[i]) + (1-groundTruth[i]) * log(1-activation[i]);
  }
  return sum;
}

Vec NeuralNetwork::costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth) {
  return (activation - groundTruth);
}
