#include "ColorConversion.h"

#include "Mat4.h"

Vec3 ColorConversion::rgbToHsv(const Vec3& other) {
  Vec3 hsv;
  
  const float minComp = std::min(other.r, std::min(other.g,other.b));
  const float maxComp = std::max(other.r, std::max(other.g,other.b));
  const float delta = maxComp - minComp;

  float h = 0;
  if (maxComp != minComp) {
    if (maxComp == other.r)
      h = fmod((60.0f * ((other.g - other.b) / delta) + 360.0f), 360.0f);
    else if (maxComp == other.g)
      h = fmod((60.0f * ((other.b - other.r) / delta) + 120.0f), 360.0f);
    else if (maxComp == other.b)
      h = fmod((60.0f * ((other.r - other.g) / delta) + 240.0f), 360.0f);
  }

  const float s = (maxComp == 0) ? 0 : (delta / maxComp);
  const float v = maxComp;

  return {h,s,v};
}

Vec3 ColorConversion::hsvToRgb(const Vec3& other) {
  const float h = float(int(other.x) % 360) / 60;
  const float s = std::max(0.0f, std::min(1.0f, other.y));
  const float v = std::max(0.0f, std::min(1.0f, other.z));

  if (s == 0) return {v,v,v}; // Achromatic (grey)

  const int i = int(floor(h));
  const float f = h - i;
  const float p = v * (1 - s);
  const float q = v * (1 - s * f);
  const float t = v * (1 - s * (1 - f));

  switch(i) {
    case 0: return {v,t,p};
    case 1: return {q,v,p};
    case 2: return {p,v,t};
    case 3: return {p,q,v};
    case 4: return {t,p,v};
    default: return {v,p,q};
  }
}

Vec3 ColorConversion::rgbToCmy(const Vec3& other) {
  return {1.0f-other.x,1.0f-other.y,1.0f-other.z};
}

Vec3 ColorConversion::cmyToRgb(const Vec3& other) {
  return {1.0f-other.x,1.0f-other.y,1.0f-other.z};
}

Vec4 ColorConversion::rgbToCmyk(const Vec3& other) {
  const Vec3 cmy = rgbToCmy(other);
  const float minVal = std::min(cmy.x,std::min(cmy.y,cmy.z));
  return {cmy-minVal,minVal};
}

Vec3 ColorConversion::cmykToRgb(const Vec4& other) {
  return {1.0f-(other.x+other.w),1.0f-(other.y+other.w),1.0f-(other.z+other.w)};
}

Vec3 ColorConversion::hslToHsv(const Vec3& other) {
  const float h = other.x;
  const float s = other.y;
  const float l = other.z;
  
  float v = s*std::min(l,1-l)+l;
  return {h,(v > 0) ?  2-2*l/v : 0, v};
}

Vec3 ColorConversion::hsvToHsl(const Vec3& other) {
  const float h = other.x;
  const float s = other.y;
  const float v = other.z;
  
  float l = v-v*s/2;
  float m = std::min(l,1-l);
  return {h,(m > 0) ? (v-l)/m : l, l};
}

Vec3 ColorConversion::rgbToYuv(const Vec3& other) {
  const Mat4 c{   0.299f,   0.587f,  0.114f,  0.0f,
                 -0.147f,  -0.289f,  0.436f,  0.0f,
                  0.615f,  -0.515f, -0.100f,  0.0f,
                    0.0f,     0.0f,    0.0f,  1.0f
  };
  return (c * Vec4(other,1.0f)).xyz();
}

Vec3 ColorConversion::yuvToRgb(const Vec3& other) {
  const Mat4 c{ 1.0f,      0.0f,    1.140f,  0.0f,
                1.0f,   -0.395f,   -0.581f,  0.0f,
                1.0f,    2.032f,      0.0f,  0.0f,
                0.0f,      0.0f,      0.0f,  1.0f
  };
  return (c * Vec4(other,1.0f)).xyz();
}
