#pragma once

#include "Layer.h"

class ConvolutionLayer : public Layer {
public:
  ConvolutionLayer(std::ifstream& file);
  ConvolutionLayer(size_t filterCount, size_t width, size_t height,
                   size_t channelCount, size_t prevWidth, size_t prevHeight,
                   Nonlinearity nonlinearity=Nonlinearity::ReLU);
  virtual ~ConvolutionLayer() {}
  virtual LayerData feedforward(const LayerData& input) override;
  virtual LayerUpdate backprop(Vec& delta, bool updateDelta) override;
  
  virtual void save(std::ofstream& file) const override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize) override;
  virtual void applyUpdate(const LayerUpdate& update, float eta, size_t bachSize, float lambda, size_t totalSize) override;

  static std::string id() {return "ConvolutionLayer";}
  
  virtual size_t outputWidth() const override {return outWidth;}
  virtual size_t outputHeight() const override {return outHeight;}
  virtual size_t outputChannels() const override {return filterCount;}
  
private:
  size_t filterCount;
  size_t width;
  size_t height;
  size_t channelCount;
  size_t prevWidth;
  size_t prevHeight;

  size_t outWidth;
  size_t outHeight;
  
  Nonlinearity nonlinearity;
  
  void load(std::ifstream& file);
  void randomInit();
  void genStructure();
  float padded(const Vec& delta, size_t u, size_t v, size_t f);
};
