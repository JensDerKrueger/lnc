#pragma once

#include "Layer.h"

class MaxPoolLayer : public Layer {
public:
  MaxPoolLayer(std::ifstream& file);
  MaxPoolLayer(size_t poolWidth, size_t poolHeight, size_t channelCount, size_t prevWidth, size_t prevHeight);
  virtual ~MaxPoolLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override {}
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override {}

  static std::string id() {return "MaxPoolLayer";}
  
  virtual size_t outputWidth() const override {return prevWidth/poolWidth;}
  virtual size_t outputHeight() const override {return prevHeight/poolHeight;}
  virtual size_t outputChannels() const override {return channelCount;}
  
private:
  size_t poolWidth;
  size_t poolHeight;
  size_t channelCount;
  size_t prevWidth;
  size_t prevHeight;
  std::vector<size_t> maxIs;
  
  void load(std::ifstream& file);

};
