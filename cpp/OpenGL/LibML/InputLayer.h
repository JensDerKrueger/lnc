#pragma once

#include "Layer.h"

class InputLayer : public Layer {
public:
  InputLayer(std::ifstream& file);
  InputLayer(size_t width, size_t height, size_t channel);
  virtual ~InputLayer() {}
  virtual LayerData feedforward(const Vec& input);
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override {return {};}
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override {}
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override {}

  static std::string id() {return "InputLayer";}
    
  virtual size_t outputWidth() const override {return width;}
  virtual size_t outputHeight() const override {return height;}
  virtual size_t outputChannels() const override {return channel;}
  
private:
  size_t width;
  size_t height;
  size_t channel;
  
  void load(std::ifstream& file);

};

