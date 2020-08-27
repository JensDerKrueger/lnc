#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Quaternion.h"

class ArcBall {
public:
  ArcBall(const Vec2i& winDim);

  void setWindowSize(const Vec2i& winDim);
  void setRadius(float radius);
  void click(const Vec2i& position);  
  Mat4 drag(const Vec2i& vPosition);
   
private:
  Vec3  startDrag;
  Vec2i winDim;
  float radius;

  Vec3 mapToSphere(const Vec2i& vPosition) const;
};
