#include "BPNetwork.h"


BPNetwork::BPNetwork(const std::vector<size_t>& structure, CostModel model, Initializer initializer) :
  structure(structure),
  model(model)
{
  randomInit(initializer);
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
  model = CostModel(s);
  
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
  
  file << uint64_t(model) << std::endl;

  file << layers.size() << std::endl;
  for (size_t i = 0;i<layers.size();++i) {
    file << layers[i].biases << std::endl;
    file << layers[i].weights << std::endl;
  }
  
  file.close();
}

void BPNetwork::randomInit(Initializer Initializer) {
  layers.clear();
  
  switch (Initializer) {
    case  Initializer:: UNIFORM :
      for (size_t i = 1;i<structure.size();++i) {
        layers.emplace_back(Vec::gaussian(structure[i],0.0f,1.0f),
                            Mat::gaussian(structure[i-1],structure[i],0.0f,1.0f));
      }
      break;
    case  Initializer:: NORMALIZED :
    default:
      for (size_t i = 1;i<structure.size();++i) {
        layers.emplace_back(Vec::gaussian(structure[i],0.0f,1.0f),
                            Mat::gaussian(structure[i-1],structure[i],0.0f,1.0f) * (1.0f/sqrt(structure[i-1])));
      }
      break;
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
  Vec delta = costDelta(zs[zs.size()-1], activations[activations.size()-1], groundTruth);
  
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

void BPNetwork::applyUpdate(const Update& update, float eta, size_t bachSize) {
  const float normEta = eta/bachSize;
  for (size_t i = 0;i<update.layers.size();++i) {
    layers[i].biases  -= update.layers[i].biases*normEta;
    layers[i].weights -= update.layers[i].weights*normEta;
  }
}

void BPNetwork::applyUpdate(const Update& update, float eta, size_t bachSize, float lambda, size_t totalSize) {
  const float normEta = eta/bachSize;
  const float scale = 1.0f - eta * lambda/totalSize;
  for (size_t i = 0;i<update.layers.size();++i) {
    layers[i].biases  -= update.layers[i].biases*normEta;
    layers[i].weights  = layers[i].weights * scale - update.layers[i].weights*normEta;
  }
}

float BPNetwork::cost(const Vec& activation, const Vec& groundTruth) {
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

Vec BPNetwork::costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth) {
  switch (model) {
    case CostModel::QUADRATIC :
      return (activation - groundTruth) * input.apply(sigmoidPrime);
    case CostModel::CROSS_ENTROPY :
      return (activation - groundTruth);
    default :
      return Vec(activation.size());
  }
}
