#pragma once

#include <memory>
#include <vector>
#include <string>

#include "InputLayer.h"
#include "DenseLayer.h"
#include "MaxPoolLayer.h"
#include "ConvolutionLayer.h"
#include "SoftmaxLayer.h"

class NeuralNetwork {
public:
  NeuralNetwork(const std::shared_ptr<InputLayer> inputLayer, const std::vector<std::shared_ptr<Layer>>& layers);
  NeuralNetwork(const std::string& filename);

  void load(const std::string& filename);
  void save(const std::string& filename) const;
  
  Vec feedforward(const Vec& input) {return feedforwardInt(input).a;}
  NetworkUpdate backpropagation(const Vec& input, const Vec& target);
    
  void applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize);
  void applyUpdate(const NetworkUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize);

private:
  std::shared_ptr<InputLayer> inputLayer;
  std::vector<std::shared_ptr<Layer>> layers;
  
  void randomInit();
  
  LayerData feedforwardInt(const Vec& input);
  
  Vec costDelta(const Vec& input, const Vec& activation, const Vec& groundTruth);
};
