#include <iostream>

#include "ArcBall.h"


ArcBall::ArcBall(const Vec2ui& winDim) :
  startDrag(),
  winDim{winDim},
  radius{1.0f}
{
}

void ArcBall::setWindowSize(const Vec2ui& winDim) {
  this->winDim = winDim;
}

void ArcBall::click(const Vec2ui& position) {
  startDrag = mapToSphere(position);
}

Mat4 ArcBall::drag(const Vec2ui& position) {
  // Map the point to the sphere
  const Vec3 current = mapToSphere(position);

  // compute the vector perpendicular to the begin and end vectors
  const Vec3 cross = Vec3::cross(current, startDrag);
  const float dot = Vec3::dot(current, startDrag);

  if (cross.length() > 1.0e-5f)
    return Quaternion(cross, dot).computeRotation();
  else
    return Quaternion(0,0,0,0).computeRotation();
}

Vec3 ArcBall::mapToSphere(const Vec2ui& position) const {
  // normalize position to [-1 ... 1]
  const Vec2 normPosition {
    -(position.x() / (float(winDim.x() - 1) / 2.0f)) - 1.0f,
     (position.y() / (float(winDim.y() - 1) / 2.0f)) - 1.0f
  };

  // compute the length of the vector to the point from the center
  const float length = normPosition.length();
  
  std::cout << "position " << position << std::endl;
  std::cout << "winDim " << winDim << std::endl;
  std::cout << "normPosition " << normPosition << std::endl;
  std::cout << "length " << length << std::endl;

  // if the point is mapped outside of the sphere... (length > radius)
  if (length > radius) {
    // compute a normalizing factor (radius / length)
    const float norm = float(radius / length);

    // return the "normalized" vector, a point on the sphere
    return {normPosition.x() * norm, normPosition.y() * norm,0.0f};
  } else {   // rlse it's inside
    // return a vector to a point mapped inside the sphere
    std::cout << "xxxx" << std::endl;
    
    
    return {normPosition.x(), normPosition.y(), length-radius};
  }
}

void ArcBall::setRadius(float radius) {
  this->radius = radius;
}
