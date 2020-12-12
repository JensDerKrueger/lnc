#pragma once

#include "Layer.h"

class MaxPoolLayer : public Layer {
public:
  MaxPoolLayer(std::ifstream& file);
  MaxPoolLayer(size_t poolWidth, size_t poolHeight, size_t prevFilterCount, size_t prevWidth, size_t prevHeight);
  virtual ~MaxPoolLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override;

  static std::string id() {return "MaxPoolLayer";}
  
private:
  void load(std::ifstream& file);
  void randomInit(size_t poolWidth, size_t poolHeight, size_t prevFilterCount, size_t prevWidth, size_t prevHeight);

};
