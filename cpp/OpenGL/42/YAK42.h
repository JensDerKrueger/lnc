#pragma once

#include <GLApp.h>

#include <Vec3.h>

class YAK42 {
public:
  YAK42(const Vec3i& pos) : pos(pos) {}
  virtual ~YAK42() {};
  
  virtual void render(GLApp& app) const = 0;
  virtual std::vector<Vec2t<uint16_t>> studsTop() = 0;
  virtual std::vector<Vec2t<uint16_t>> studsBottom() = 0;

  Vec3i getPos() const {return pos;}

  static const std::array<Vec4,111> colors;
 
private:
  Vec3i pos;
    
};

class SimpleYAK42 : public YAK42 {
public:
  SimpleYAK42(uint16_t width, uint16_t depth, uint16_t height,
              uint16_t colorCode, const Vec3i& pos);
  
  virtual ~SimpleYAK42() {};

  virtual void render(GLApp& app) const override;
  virtual std::vector<Vec2t<uint16_t>> studsTop() override;
  virtual std::vector<Vec2t<uint16_t>> studsBottom() override;
  
private:
  uint16_t width;
  uint16_t depth;
  uint16_t height;
  uint16_t colorCode;
  
  std::vector<float> geometry;
  
  void generateGeometry();
  void generateStuds();
  void generateBase();
  
};
