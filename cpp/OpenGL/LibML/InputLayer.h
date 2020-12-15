#pragma once

#include "Layer.h"

class InputLayer : public Layer {
public:
  InputLayer(std::ifstream& file);
  InputLayer(size_t width, size_t height);
  virtual ~InputLayer() {}
  virtual LayerData feedforward(const Vec& input);
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override {return {};}
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override {}
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override {}

  static std::string id() {return "InputLayer";}
    
private:
  size_t width;
  size_t height;
  
  void load(std::ifstream& file);

};

