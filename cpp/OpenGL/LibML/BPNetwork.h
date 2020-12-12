#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <exception>

#include "LA.h"

struct LayerInfo {
  LayerInfo(const Vec& biases, const Mat& weights) : biases(biases),weights(weights){}
  Vec biases;
  Mat weights;
};

struct Update {
  std::vector<LayerInfo> layers;
  
  Update& operator+=(const Update& rhs) {
    for (size_t i = 0;i<layers.size();++i) {
      layers[i].biases += rhs.layers[i].biases;
      layers[i].weights += rhs.layers[i].weights;
    }
    return *this;
  }
  
  friend std::ostream& operator<<(std::ostream &os, const Update& v) {os << v.toString() ; return os;}
  const std::string toString() const {
    std::string result{"Biases:\n"};

    for (size_t i = 0;i<layers.size();++i) {
      result += layers[i].biases.toString() + "\n";
    }
    result += "weights:\n";
    for (size_t i = 0;i<layers.size();++i) {
      result += layers[i].weights.toString() + "\n";
    }
    return result;
  }
};


class BPNetwork {
public:
  enum class Initializer {UNIFORM, NORMALIZED};
  enum class CostModel {QUADRATIC, CROSS_ENTROPY};

  BPNetwork(const std::vector<size_t>& structure, CostModel model=CostModel::CROSS_ENTROPY, Initializer initializer=Initializer::NORMALIZED);
  BPNetwork(const std::string& filename);

  void load(const std::string& filename);
  void save(const std::string& filename) const;
  
  Vec feedforward(const Vec& input);
  Update backpropagation(const Vec& input, const Vec& target);
    
  void applyUpdate(const Update& update, float eta, size_t bachSize);
  void applyUpdate(const Update& update, float eta, size_t bachSize, float lambda, size_t totalSize);

private:
  std::vector<size_t> structure;
  std::vector<LayerInfo> layers;
  CostModel model;
  
  void randomInit(Initializer initializer);
  
  float cost(const Vec& activation, const Vec& groundTruth);
  Vec costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth);
};
