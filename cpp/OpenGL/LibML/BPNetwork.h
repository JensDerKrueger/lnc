#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <exception>

#include "LA.h"

class FileException : std::exception {
public:
  FileException(const std::string& desc) : desc(desc) {}
  const char* what() const noexcept {return desc.c_str();}
private:
  std::string desc;
};

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
  BPNetwork(const std::vector<size_t>& structure);
  BPNetwork(const std::string& filename);

  void load(const std::string& filename);
  void save(const std::string& filename) const;
  
  Vec feedforward(const Vec& input);
  Update backpropagation(const Vec& input, const Vec& target);
  void applyUpdate(const Update& update, float eta);
    
private:
  std::vector<size_t> structure;
  std::vector<LayerInfo> layers;
  
  void randomInit();
  
  static Vec costPrime(const Vec& activation, const Vec& groundTruth);
};
