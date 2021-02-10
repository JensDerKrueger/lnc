#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include <Vec3.h>

enum class DemoType {
  DRAIN,
  SATTLE,
  CRITICAL
};

class Flowfield {
public:
  Flowfield(size_t sizeX, size_t sizeY, size_t sizeZ);
  Vec3 interpolate(const Vec3& pos);

  size_t getSizeX() const {return sizeX;}
  size_t getSizeY() const {return sizeY;}
  size_t getSizeZ() const {return sizeZ;}

  static Flowfield genDemo(size_t size, DemoType d);
  static Flowfield fromFile(const std::string& filename);
private:
  size_t sizeX;
  size_t sizeY;
  size_t sizeZ;
  std::vector<Vec3> data;
  Vec3 getData(size_t x, size_t y, size_t z);
  
  Vec3 linear(const Vec3& a, const Vec3& b, float alpha);
};
