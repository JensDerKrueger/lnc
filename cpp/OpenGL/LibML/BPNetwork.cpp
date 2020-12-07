#include "BPNetwork.h"


BPNetwork::BPNetwork(const std::vector<size_t>& structure) :
  structure(structure),
  layers(structure.size())
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
  biases.clear();
  for (size_t i = 0;i<s;++i) {
    Vec v(0);
    file >> v;
    biases.push_back(v);
  }

  file >> s;
  weights.clear();
  for (size_t i = 0;i<s;++i) {
    Mat m(0,0);
    file >> m;
    weights.push_back(m);
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

  file << biases.size() << std::endl;
  for (size_t i = 0;i<biases.size();++i) {
    file << biases[i] << std::endl;
  }
  
  file << weights.size() << std::endl;
  for (size_t i = 0;i<weights.size();++i) {
    file << weights[i] << std::endl;
  }

  file.close();
}

void BPNetwork::randomInit() {
  biases.clear();
  weights.clear();
  for (size_t i = 1;i<layers;++i) {
    biases.push_back(Vec::random(structure[i],-1.0f,1.0f));
    weights.push_back(Mat::random(structure[i-1],structure[i],-1.0f,1.0f));
  }
}

Vec BPNetwork::feedforward(const Vec& input) {
  Vec activation{input};
  for (size_t i = 1;i<layers;++i) {
    activation = (weights[i-1] * activation + biases[i-1]).apply(sigmoid);
  }
  return activation;
}

Update BPNetwork::backpropagation(const Vec& input, const Vec& groundTruth) {
  Update update;
  
  for (size_t i = 1;i<layers;++i) {
    update.biases.push_back(Vec{structure[i]});
    update.weights.push_back(Mat{structure[i-1],structure[i]});
  }

  // feedforward
  Vec activation{input};
  std::vector<Vec> activations{activation};
  std::vector<Vec> zs;
  for (size_t i = 1;i<layers;++i) {
    zs.push_back(weights[i-1] * activation + biases[i-1]);
    activation = zs.back().apply(sigmoid);
    activations.push_back(activation);
  }
  
  // backprop last layer
  Vec delta = costPrime(activations[activations.size()-1], groundTruth) * zs[zs.size()-1].apply(sigmoidPrime);
  update.biases[update.biases.size()-1]  = delta;
  update.weights[update.weights.size()-1] = Mat::tensorProduct(activations[activations.size()-2],delta);

  // backprop remaining layers
  for (size_t l = 2;l<layers;++l) {
    delta = (weights[weights.size()-l+1].transpose() * delta) * zs[zs.size()-l].apply(sigmoidPrime);
    update.biases[update.biases.size()-l] = delta;
    update.weights[update.weights.size()-l] = Mat::tensorProduct(activations[activations.size()-l-1], delta);
  }

  return update;
}

void BPNetwork::applyUpdate(const Update& update, float eta) {
  for (size_t i = 0;i<biases.size();++i) {
    biases[i] -= update.biases[i] * eta;
  }
  for (size_t i = 0;i<weights.size();++i) {
    weights[i] -= update.weights[i] * eta;
  }
}

Vec BPNetwork::costPrime(const Vec& activation, const Vec& groundTruth) {
  return activation-groundTruth;
}
