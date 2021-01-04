#pragma once

#include <fstream>
#include <vector>
#include <string>

#include <iostream>

#include "LA.h"

enum class Nonlinearity {Sigmoid, ReLU, Tanh};

struct LayerData {
  Vec a{0};
  Vec z{0};
};

struct LayerUpdate {
  LayerUpdate() : biases(0),weights(0,0){}
  LayerUpdate(const Vec& biases, const Mat& weights) : biases(biases),weights(weights){}
  Vec biases;
  Mat weights;
  
  LayerUpdate& operator+=(const LayerUpdate& rhs) {
    biases += rhs.biases;
    weights += rhs.weights;
    return *this;
  }
};

struct NetworkUpdate {
  std::vector<LayerUpdate> layers;
  
  NetworkUpdate& operator+=(const NetworkUpdate& rhs) {
    for (size_t i = 0;i<layers.size();++i) {
      layers[i] += rhs.layers[i];
    }
    return *this;
  }
  
  friend std::ostream& operator<<(std::ostream &os, const NetworkUpdate& v) {os << v.toString() ; return os;}
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

class Layer {
public:
  virtual LayerData feedforward(const LayerData& input) = 0;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) = 0;
  
  virtual void save(std::ofstream& file) const = 0;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) = 0;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) = 0;

  virtual size_t outputWidth() const = 0;
  virtual size_t outputHeight() const = 0;
  virtual size_t outputChannels() const = 0;

  LayerData input;
  Vec biases{0};
  Mat weights{0,0};

};
