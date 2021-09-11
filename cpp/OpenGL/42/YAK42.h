#pragma once

#include <Vec3.h>
#include <Vec4.h>

class YAK42 {
public:
  YAK42(const Vec3i& pos,
        uint16_t colorCode) : pos(pos), colorCode(colorCode) {}
  virtual ~YAK42() {};
  
  virtual std::vector<Vec2t<uint16_t>> studsTop() const = 0;
  virtual std::vector<Vec2t<uint16_t>> studsBottom() const = 0;

  Vec3i getIntegerPos() const {return pos;}
  Vec3 getPos() const {return Vec3{pos}/Vec3{1,3,1};}

  virtual Vec3 computeGlobalStudPos(size_t i) const = 0;
  
  static const std::array<Vec4,111> colors;
 
  Vec4 getColor() const {
    return colors[colorCode];
  }
  
  static const Vec3 brickScale;
  static constexpr float studHeight{1.8f};
  static constexpr float studRadius{2.4f};
  static const float studSpacing;
  
private:
  Vec3i pos;
  uint16_t colorCode;

};

class SimpleYAK42 : public YAK42 {
public:
  void extracted(uint16_t depth, uint16_t width);
  
  SimpleYAK42(uint16_t width, uint16_t depth, uint16_t height,
              uint16_t colorCode, const Vec3i& pos);
  
  virtual ~SimpleYAK42() {};

  virtual std::vector<Vec2t<uint16_t>> studsTop() const override;
  virtual std::vector<Vec2t<uint16_t>> studsBottom() const override;
  
  virtual Vec3 computeGlobalStudPos(size_t i) const override;

  Vec3i getScale() const {return {width,height,depth};}

private:
  uint16_t width;
  uint16_t depth;
  uint16_t height;
  
  std::vector<float> geometry;
  std::vector<Vec2t<uint16_t>> studs;
  
  void generateStudPositions();
};
