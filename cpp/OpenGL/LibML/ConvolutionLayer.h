#pragma once

#include "Layer.h"

class ConvolutionLayer : public Layer {
public:
  ConvolutionLayer(std::ifstream& file);
  ConvolutionLayer(size_t filterCount,
                   size_t width, size_t height, size_t prevFilterCount,
                   size_t prevWidth, size_t prevHeight);
  virtual ~ConvolutionLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override;

  static std::string id() {return "ConvolutionLayer";}
  
private:
  size_t filterCount;
  size_t width;
  size_t height;
  size_t prevFilterCount;
  size_t prevWidth;
  size_t prevHeight;
  
  void load(std::ifstream& file);
  void randomInit();
  void genStructure();
};
