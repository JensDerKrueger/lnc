#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork(const std::vector<size_t>& structure, CostModel model) :
  structure(structure),
  model(model)
{
  randomInit();
}

NeuralNetwork::NeuralNetwork(const std::string& filename) {
  load(filename);
}

void NeuralNetwork::load(const std::string& filename) {
  std::ifstream file{filename};
  if (!file) throw FileException{std::string("Unable to read file ")+filename};
  
  uint64_t s;
  file >> s;
  structure.resize(s);
  for (size_t i = 0;i<structure.size();++i) {
    file >> structure[i];
  }
  
  file >> s;
  model = CostModel(s);
  
  file >> s;
  layers.clear();
  for (size_t i = 0;i<s;++i) {
    std::string layerId;
    file >> layerId;
    if (layerId == DenseLayer::id())
      layers.push_back(std::make_shared<DenseLayer>(file));
  }

  file.close();
}

void NeuralNetwork::save(const std::string& filename) const {
  std::ofstream file{filename};
  if (!file) throw FileException{std::string("Unable to write file ")+filename};
  
  file << structure.size() << std::endl;
  for (size_t i = 0;i<structure.size();++i) {
    file << structure[i] << std::endl;
  }
  
  file << uint64_t(model) << std::endl;

  file << layers.size() << std::endl;
  for (size_t i = 0;i<layers.size();++i) {
    layers[i]->save(file);
  }
  
  file.close();
}

void NeuralNetwork::randomInit() {
  layers.clear();

  for (size_t i = 1;i<structure.size();++i) {
    layers.push_back(std::make_shared<DenseLayer>(structure[i], structure[i-1]));
  }
}

LayerData NeuralNetwork::feedforwardInt(const Vec& input) {
  LayerData l{input, Vec{0}};
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
  update.layers.push_back(layers[ls-1]->backprop(delta));
  
  // backprop remaining layers
  for (size_t l = 0;l<layers.size()-1;++l) {
    update.layers.push_back(layers[ls-l-2]->backprop(delta));
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
  switch (model) {
    case CostModel::QUADRATIC :
      return 0.5f * Vec::dot((activation-groundTruth), (activation-groundTruth));
    case CostModel::CROSS_ENTROPY : {
      float sum = 0.0f;
      for (size_t i = 0;i<activation.size();++i) {
        sum -= groundTruth[i] * log(activation[i]) + (1-groundTruth[i]) * log(1-activation[i]);
      }
      return sum;
    }
    default :
      return 0;
  }
}

Vec NeuralNetwork::costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth) {
  switch (model) {
    case CostModel::QUADRATIC :
      return (activation - groundTruth) * input.apply(sigmoidPrime);
    case CostModel::CROSS_ENTROPY :
      return (activation - groundTruth);
    default :
      return Vec(activation.size());
  }
}
