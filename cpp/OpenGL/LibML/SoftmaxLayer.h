#pragma once

#include "Layer.h"

class SoftmaxLayer : public Layer {
public:
  SoftmaxLayer(std::ifstream& file);
  SoftmaxLayer(size_t size, size_t prevSize);
  virtual ~SoftmaxLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override;

  static std::string id() {return "SoftmaxLayer";}
  
  virtual size_t outputWidth() const override {return biases.size();}
  virtual size_t outputHeight() const override {return 1;}
  virtual size_t outputChannels() const override {return 1;}

private:
  void load(std::ifstream& file);
  void randomInit(size_t size, size_t prevSize);

};
