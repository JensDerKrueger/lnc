#pragma once

#include "DenseLayer.h"
#include "MaxPoolLayer.h"
#include "ConvolutionLayer.h"

class NeuralNetwork {
public:
  enum class CostModel {QUADRATIC, CROSS_ENTROPY};

  NeuralNetwork(const std::vector<size_t>& structure, CostModel model=CostModel::CROSS_ENTROPY);
  NeuralNetwork(const std::string& filename);

  void load(const std::string& filename);
  void save(const std::string& filename) const;
  
  Vec feedforward(const Vec& input) {return feedforwardInt(input).a;}
  NetworkUpdate backpropagation(const Vec& input, const Vec& target);
    
  void applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize);
  void applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize);

private:
  std::vector<size_t> structure;
  std::vector<std::shared_ptr<Layer>> layers;
  CostModel model;
  
  void randomInit();
  
  LayerData feedforwardInt(const Vec& input);
  
  float cost(const Vec& activation, const Vec& groundTruth);
  Vec costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth);
};
