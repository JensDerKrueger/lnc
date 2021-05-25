#pragma once

#include "Vec3.h"
#include "Vec4.h"

namespace ColorConversion {
  Vec3 rgbToHsv(const Vec3& other);
  Vec3 hsvToRgb(const Vec3& other);

  Vec3 hslToHsv(const Vec3& other);
  Vec3 hsvToHsl(const Vec3& other);

  Vec3 rgbToCmy(const Vec3& other);
  Vec4t<float> rgbToCmyk(const Vec3& other);

  Vec3 cmyToRgb(const Vec3& other);
  Vec3 cmykToRgb(const Vec4& other);
  
  Vec3 rgbToYuv(const Vec3& other);
  Vec3 yuvToRgb(const Vec3& other);
}
