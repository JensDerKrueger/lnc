#pragma once

#include "Layer.h"

class DenseLayer : public Layer {
public:
  DenseLayer(std::ifstream& file);
  DenseLayer(size_t size, size_t prevSize);
  virtual ~DenseLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override;

  static std::string id() {return "DenseLayer";}
  
private:
  void load(std::ifstream& file);
  void randomInit(size_t size, size_t prevSize);

};
