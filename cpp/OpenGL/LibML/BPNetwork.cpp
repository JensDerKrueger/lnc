#include "BPNetwork.h"


BPNetwork::BPNetwork(const std::vector<size_t>& structure) :
  structure(structure)
{
  randomInit();
}

BPNetwork::BPNetwork(const std::string& filename) {
  load(filename);
}

void BPNetwork::load(const std::string& filename) {
  std::ifstream file{filename};
  if (!file) throw FileException{std::string("Unable to read file ")+filename};
  
  uint64_t s;
  file >> s;
  structure.resize(s);
  for (size_t i = 0;i<structure.size();++i) {
    file >> structure[i];
  }
  
  file >> s;
  layers.clear();
  for (size_t i = 0;i<s;++i) {
    Vec v(0);
    Mat m(0,0);
    file >> v;
    file >> m;
    layers.emplace_back(v,m);
  }

  file.close();
}

void BPNetwork::save(const std::string& filename) const {
  std::ofstream file{filename};
  if (!file) throw FileException{std::string("Unable to write file ")+filename};
  
  file << structure.size() << std::endl;
  for (size_t i = 0;i<structure.size();++i) {
    file << structure[i] << std::endl;
  }

  file << layers.size() << std::endl;
  for (size_t i = 0;i<layers.size();++i) {
    file << layers[i].biases << std::endl;
    file << layers[i].weights << std::endl;
  }
  
  file.close();
}

void BPNetwork::randomInit() {
  layers.clear();
  for (size_t i = 1;i<structure.size();++i) {
    layers.emplace_back(Vec::random(structure[i],-1.0f,1.0f),
                        Mat::random(structure[i-1],structure[i],-1.0f,1.0f));
  }
}

Vec BPNetwork::feedforward(const Vec& input) {
  Vec activation{input};
  for (size_t i = 1;i<structure.size();++i) {
    activation = (layers[i-1].weights * activation + layers[i-1].biases).apply(sigmoid);
  }
  return activation;
}

Update BPNetwork::backpropagation(const Vec& input, const Vec& groundTruth) {
  Update update;
  for (size_t i = 1;i<structure.size();++i) {
    update.layers.emplace_back(Vec{structure[i]}, Mat{structure[i-1], structure[i]});
  }

  // feedforward
  Vec activation{input};
  std::vector<Vec> activations{activation};
  std::vector<Vec> zs;
  for (size_t i = 1;i<structure.size();++i) {
    zs.push_back(layers[i-1].weights * activation + layers[i-1].biases);
    activation = zs.back().apply(sigmoid);
    activations.push_back(activation);
  }
  
  // backprop last layer
  const size_t ls = layers.size();
  Vec delta = costPrime(activations[activations.size()-1], groundTruth) * zs[zs.size()-1].apply(sigmoidPrime);
  update.layers[ls-1].biases  = delta;
  update.layers[ls-1].weights = Mat::tensorProduct(activations[activations.size()-2],delta);

  // backprop remaining layers
  for (size_t l = 2;l<structure.size();++l) {
    delta = (layers[ls-l+1].weights.transpose() * delta) * zs[zs.size()-l].apply(sigmoidPrime);
    update.layers[ls-l].biases = delta;
    update.layers[ls-l].weights = Mat::tensorProduct(activations[activations.size()-l-1], delta);
  }

  return update;
}

void BPNetwork::applyUpdate(const Update& update, float eta) {
  for (size_t i = 0;i<update.layers.size();++i) {
    layers[i].biases  -= update.layers[i].biases*eta;
    layers[i].weights -= update.layers[i].weights*eta;
  }
}

Vec BPNetwork::costPrime(const Vec& activation, const Vec& groundTruth) {
  return activation-groundTruth;
}
