#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Quaternion.h"

class ArcBall {
public:
  ArcBall(const Vec2ui& winDim);

  void setWindowSize(const Vec2ui& winDim);
  void setRadius(float radius);
  void click(const Vec2ui& position);
  Mat4 drag(const Vec2ui& vPosition);
   
private:
  Vec3    startDrag;
  Vec2ui  winDim;
  float   radius;

  Vec3 mapToSphere(const Vec2ui& vPosition) const;
};
