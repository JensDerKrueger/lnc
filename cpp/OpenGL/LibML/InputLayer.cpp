#include "InputLayer.h"

InputLayer::InputLayer(std::ifstream& file) {
  load(file);
}

InputLayer::InputLayer(size_t width, size_t height, size_t channel) :
  width(width),
  height(height),
  channel(channel)
{
}

LayerData InputLayer::feedforward(const Vec& input) {
  return {input, input};
}

LayerData InputLayer::feedforward(const LayerData& input) {
  return input;
}

void InputLayer::save(std::ofstream& file) const {
  file << id() << std::endl;
  file << width << std::endl;
  file << height << std::endl;
  file << channel << std::endl;
}

void InputLayer::load(std::ifstream& file) {
  file >> width;
  file >> height;
  file >> channel;
}
