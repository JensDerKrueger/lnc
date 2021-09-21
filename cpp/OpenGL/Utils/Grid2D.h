#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Vec2.h"
#include "Vec3.h"
#include "Image.h"
#include "GLTexture2D.h"

class Grid2D {
public:
  Grid2D(size_t width, size_t height);
  Grid2D(size_t width, size_t height, const std::vector<float> data);
  
  Grid2D(const Grid2D& other);
  Grid2D(const Image& image);
  
  Grid2D(std::istream &is);
  void save(std::ostream &os) const;

  size_t getWidth() const;
  size_t getHeight() const;
  std::string toString() const;
  std::vector<uint8_t> toByteArray() const;
  Grid2D toSignedDistance(float threshold) const;
  GLTexture2D toTexture() const;

  void setValue(size_t x, size_t y, float value);
  float getValueNormalized(float x, float y) const;
  float getValue(size_t x, size_t y) const;
  float sample(float x, float y) const ;
  float sample(const Vec2& pos) const ;
  
  Vec3 normal(float x, float y) const;
  Vec3 normal(const Vec2& pos) const;

  static Grid2D genRandom(size_t x, size_t y);
  static Grid2D genRandom(size_t x, size_t y, uint32_t seed);
  Grid2D operator*(const float& value) const;
  Grid2D operator/(const float& value) const;
  Grid2D operator+(const float& value) const;
  Grid2D operator-(const float& value) const;

  Grid2D operator+(const Grid2D& other) const;
  Grid2D operator/(const Grid2D& other) const;
  Grid2D operator*(const Grid2D& other) const;
  Grid2D operator-(const Grid2D& other) const;
  void normalize(const float maxVal = 1);

  Vec2t<size_t> maxValue() const;
  Vec2t<size_t> minValue() const;
  
  void fill(float value);

  friend std::ostream& operator<<(std::ostream &os, const Grid2D& v);

  static Grid2D fromBMP(const std::string& filename);
    
private:
  size_t width;
  size_t height;
  std::vector<float> data{};
  size_t index(size_t x, size_t y) const;
  
  std::pair<size_t,size_t> findMaxSize(const Grid2D& other) const;
};
