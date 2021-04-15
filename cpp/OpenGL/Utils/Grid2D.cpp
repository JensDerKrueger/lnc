#include <limits>
#include <cmath>
#include <stdexcept>

#include "Rand.h"
#include "Vec2.h"
#include "bmp.h"

#include "Grid2D.h"

Grid2D::Grid2D(size_t width, size_t height) :
  width(width),
  height(height),
  data(width*height)
{
}

Grid2D::Grid2D(size_t width, size_t height, const std::vector<float> data) :
width(width),
height(height),
data(data)
{
  if (width*height != data.size())
    throw std::runtime_error("size mismatch");
}

Grid2D::Grid2D(const Grid2D& other) :
  width(other.width),
  height(other.height),
  data(other.data)
{
}

Grid2D::Grid2D(const Image& image) :
  width(image.width),
  height(image.height),
  data(image.data.size()/image.componentCount)
{
  for (size_t i = 0;i<data.size();++i) {
    data[i] = image.data[i*image.componentCount] / 255.0f;
  }
}
    
size_t Grid2D::getWidth() const {
  return width;
}

size_t Grid2D::getHeight() const {
  return height;
}

std::string Grid2D::toString() const {
    std::stringstream s;
    for (size_t i = 0;i<data.size();++i) {
        s << data[i];
        if (i%width == width-1 && i != 0)
            s << std::endl;
        else
            s << ", ";
    }
    return s.str();
}

std::vector<uint8_t> Grid2D::toByteArray() const {
    std::vector<uint8_t> uidata(data.size()*3);
    for (size_t i = 0;i<data.size();++i) {
        uidata[i*3+0] = uint8_t(data[i]*255);
        uidata[i*3+1] = uint8_t(data[i]*255);
        uidata[i*3+2] = uint8_t(data[i]*255);
    }
    return uidata;
}

void Grid2D::setValue(size_t x, size_t y, float value) {
    data[index(x,y)] = value;
}


float Grid2D::getValueNormalized(float x, float y) const {
    return data[index(size_t(x*width),size_t(y*height))];
}

float Grid2D::getValue(size_t x, size_t y) const {
    return data[index(x,y)];
}


float Grid2D::sample(const Vec2& pos) const  {
    return sample(pos.x(), pos.y());
}

float Grid2D::sample(float x, float y) const {
    x = std::max(std::min(x,1.0f), 0.0f);
    y = std::max(std::min(y,1.0f), 0.0f);
    
    float sx = x*(width-1);
    float sy = y*(height-1);
    
    float alpha = sx - floorf(sx);
    float beta  = sy - floorf(sy);
    
    Vec2ui a{uint32_t(floorf(sx)),uint32_t(floorf(sy))};
    Vec2ui b{uint32_t(ceilf(sx)),uint32_t(floorf(sy))};
    Vec2ui c{uint32_t(floorf(sx)),uint32_t(ceilf(sy))};
    Vec2ui d{uint32_t(ceilf(sx)),uint32_t(ceilf(sy))};
    
    float va = getValue(a.x(), a.y());
    float vb = getValue(b.x(), b.y());
    float vc = getValue(c.x(), c.y());
    float vd = getValue(d.x(), d.y());
    
    return (va * (1.0f-alpha) + vb * alpha) * (1.0f-beta) + (vc * (1.0f-alpha) + vd * alpha) * beta;
}


Vec3 Grid2D::normal(const Vec2& pos) const {
    return normal(pos.x(), pos.y());
}

Vec3 Grid2D::normal(float x, float y) const {
    x = std::max(std::min(x, 1.0f), 0.0f);
    y = std::max(std::min(y, 1.0f), 0.0f);

    float sx = x * (width - 1);
    float sy = y * (height - 1);
    
    Vec2ui a{ uint32_t(floorf(sx)),uint32_t(floorf(sy)) };
    Vec2ui b{ uint32_t(ceilf(sx)),uint32_t(floorf(sy)) };
    Vec2ui c{ uint32_t(floorf(sx)),uint32_t(ceilf(sy)) };
    Vec2ui d{ uint32_t(ceilf(sx)),uint32_t(ceilf(sy)) };

    float va = getValue(a.x(), a.y());
    float vb = getValue(b.x(), b.y());
    float vc = getValue(c.x(), c.y());
    float vd = getValue(d.x(), d.y());

    Vec3 n1 = Vec3::cross(Vec3(1.0f/width,(vb-va), 0.0f), Vec3(0.0f,(vc-va), 1.0f/height ));
    Vec3 n2 = Vec3::cross(Vec3(-1.0f/width,(vc - vd), 0.0f), Vec3(0.0f, (vb - vd), -1.0f/height));

    
    return Vec3::normalize((n1 + n2) / 2.0f);
}

Grid2D Grid2D::genRandom(size_t x, size_t y) {
    Grid2D result{x,y};
    for (size_t i = 0;i<result.data.size();++i) {
        result.data[i] = Rand::rand01();
    }
    return result;
}

Grid2D Grid2D::operator*(const float& value) const {
    Grid2D result{width,height};
    for (size_t i = 0;i<result.data.size();++i) {
        result.data[i] = data[i]*value;
    }
    return result;
}

Grid2D Grid2D::operator+(const float& value) const {
    Grid2D result{width,height};
    for (size_t i = 0;i<result.data.size();++i) {
        result.data[i] = data[i]+value;
    }
    return result;
}

Grid2D Grid2D::operator-(const float& value) const {
    Grid2D result{width,height};
    for (size_t i = 0;i<result.data.size();++i) {
        result.data[i] = data[i]-value;
    }
    return result;
}

Grid2D Grid2D::operator/(const float& value) const {
    return *this * (1.0f/value);
}

Grid2D Grid2D::operator-(const Grid2D& other) const {
    if (other.width > width) {
        Grid2D result{other.width,other.height};
        size_t i=0;
        for (size_t y = 0;y<other.height;++y) {
            for (size_t x = 0;x<other.width;++x) {
                result.data[i] = other.data[i] - sample(x/float(other.width-1.0f),y/float(other.height-1.0f));
                i++;
            }
        }
        return result;
    } else if (other.width < width) {
        Grid2D result{width,height};
        size_t i=0;
        for (size_t y = 0;y<height;++y) {
            for (size_t x = 0;x<width;++x) {
                result.data[i] = data[i] - other.sample(x/float(width-1.0f),y/float(height-1.0f));
                i++;
            }
        }
        return result;
    } else {
        Grid2D result{width,height};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = data[i]-other.data[i];
        }
        return result;
    }
}

Grid2D Grid2D::operator*(const Grid2D& other) const {
    if (other.width > width) {
        Grid2D result{other.width,other.height};
        size_t i=0;
        for (size_t y = 0;y<other.height;++y) {
            for (size_t x = 0;x<other.width;++x) {
                result.data[i] = other.data[i] * sample(x/float(other.width-1.0f),y/float(other.height-1.0f));
                i++;
            }
        }
        return result;
    } else if (other.width < width) {
        Grid2D result{width,height};
        size_t i=0;
        for (size_t y = 0;y<height;++y) {
            for (size_t x = 0;x<width;++x) {
                result.data[i] = data[i] * other.sample(x/float(width-1.0f),y/float(height-1.0f));
                i++;
            }
        }
        return result;
    } else {
        Grid2D result{width,height};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = data[i]*other.data[i];
        }
        return result;
    }
}

Grid2D Grid2D::operator/(const Grid2D& other) const {
    if (other.width > width) {
        Grid2D result{other.width,other.height};
        size_t i=0;
        for (size_t y = 0;y<other.height;++y) {
            for (size_t x = 0;x<other.width;++x) {
                result.data[i] = other.data[i] / sample(x/float(other.width-1.0f),y/float(other.height-1.0f));
                i++;
            }
        }
        return result;
    } else if (other.width < width) {
        Grid2D result{width,height};
        size_t i=0;
        for (size_t y = 0;y<height;++y) {
            for (size_t x = 0;x<width;++x) {
                result.data[i] = data[i] / other.sample(x/float(width-1.0f),y/float(height-1.0f));
                i++;
            }
        }
        return result;
    } else {
        Grid2D result{width,height};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = data[i]/other.data[i];
        }
        return result;
    }
}

Grid2D Grid2D::operator+(const Grid2D& other) const {
    if (other.width > width) {
        Grid2D result{other.width,other.height};
        size_t i=0;
        for (size_t y = 0;y<other.height;++y) {
            for (size_t x = 0;x<other.width;++x) {
                result.data[i] = other.data[i] + sample(x/float(other.width-1.0f),y/float(other.height-1.0f));
                i++;
            }
        }
        return result;
    } else if (other.width < width) {
        Grid2D result{width,height};
        size_t i=0;
        for (size_t y = 0;y<height;++y) {
            for (size_t x = 0;x<width;++x) {
                result.data[i] = data[i] + other.sample(x/float(width-1.0f),y/float(height-1.0f));
                i++;
            }
        }
        return result;
    } else {
        Grid2D result{width,height};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = data[i]+other.data[i];
        }
        return result;
    }
}
    
void Grid2D::normalize() {
    if (data.empty()) return;
    
    float minValue = data[0];
    float maxValue = data[0];
    for (size_t i = 0;i<data.size();++i) {
        minValue = std::min(minValue, data[i]);
        maxValue = std::max(maxValue, data[i]);
    }
    
    const float scale = 1.0f/(maxValue-minValue);
    for (size_t i = 0;i<data.size();++i) {
        data[i] = (data[i]-minValue) * scale;
    }
}

Vec2t<size_t> Grid2D::maxValue() const {
  float maxVal = std::numeric_limits<float>::min();
  Vec2t<size_t> maxV{0,0};
  for (size_t i = 0;i<data.size();++i) {
    if (maxVal < data[i]){
      maxVal = data[i];
      maxV = Vec2t<size_t>{size_t(i % width), size_t(i / width)};    }
  }
  return maxV;
}

Vec2t<size_t> Grid2D::minValue() const {
  float minVal = std::numeric_limits<float>::max();
  Vec2t<size_t> minV{0,0};
  for (size_t i = 0;i<data.size();++i) {
    if (minVal > data[i]){
      minVal = data[i];
      minV = Vec2t<size_t>{size_t(i % width), size_t(i / width)};
    }
  }
  return minV;
}

size_t Grid2D::index(size_t x, size_t y) const {
    return x + y * width;
}

std::ostream& operator<<(std::ostream &os, const Grid2D& v) {
    os << v.toString() ; return os;
}

Grid2D Grid2D::fromBMP(const std::string& filename) {
    Image bmp = BMP::load(filename);
    
    Grid2D g{bmp.width, bmp.height};
    
    size_t i = 0;
    for (size_t y = 0;y<g.height;++y) {
        for (size_t x = 0;x<g.width;++x) {
            g.data[i] = bmp.getValue(uint32_t(x),uint32_t(y),0)/255.0f;
            i++;
        }
    }

    return g;
}

void Grid2D::fill(float value) {
  std::fill(data.begin(), data.end(), value);
}

Grid2D::Grid2D(std::istream &is) {
  is.read((char*)&width, sizeof (width));
  is.read((char*)&height, sizeof (height));
  
  data.resize(width*height);
  is.read((char*)data.data(), sizeof(float) * width * height);
}

void Grid2D::save(std::ostream &os) const {
  os.write((char*)&width, sizeof (width));
  os.write((char*)&height, sizeof (height));
  os.write((char*)data.data(), sizeof(float) * width * height);
}

static const float d1{1.0f};
static const float d2{1.4142135624f};

static const float INV = std::numeric_limits<float>::max();
static const Vec2ui NO_POS{std::numeric_limits<uint32_t>::max(),
                    std::numeric_limits<uint32_t>::max()};
static float dist(size_t x, size_t y, const Vec2ui& p) {
  return sqrtf( (x-float(p.x()))*(x-float(p.x())) + 
                (y-float(p.y()))*(y-float(p.y())));
}

Grid2D Grid2D::toSignedDistance(float threshold) const {
  Grid2D r(width, height);
  
  std::vector<bool> I(width*height);
  std::vector<Vec2ui> p(width*height);
  
  for (size_t i = 0;i<I.size();++i) {
    I[i] = data[i] >= threshold;
  }

  for (size_t i = 0;i<I.size();++i) {
    r.data[i] = INV;
    p[i] = NO_POS;
  }
  
  for (size_t y = 1; y<height-1; y++ ) {
    for (size_t x = 1; x<width-1; x++ ) {
      const size_t i = index(x,y);
      if (I[index(x-1,y)] != I[i] ||
          I[index(x+1,y)] != I[i] ||
          I[index(x,y+1)] != I[i] ||
          I[index(x,y-1)] != I[i]) {
        r.data[i] = 0;
        p[i] = Vec2ui(uint32_t(x),uint32_t(y));
      }
    }
  }
 
  for (size_t y = 1; y<height-1; y++ ) {
    for (size_t x = 1; x<width-1; x++ ) {
      const size_t i = index(x,y);
      if (r.data[index(x-1,y-1)]+d2 < r.data[i]) {
        p[i] = p[index(x-1,y-1)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x,y-1)]+d1 < r.data[i]) {
        p[i] = p[index(x,y-1)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x+1,y-1)]+d2 < r.data[i]) {
        p[i] = p[index(x+1,y-1)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x-1,y)]+d1 < r.data[i]) {
        p[i] = p[index(x-1,y)];
        r.data[i] = dist(x, y, p[i]);
      }
    }
  }
  
  for (size_t y = height-2; y>=1; y-- ) {
    for (size_t x = width-2; x>=1; x--) {
      const size_t i = index(x,y);
      if (r.data[index(x+1,y)]+d1 < r.data[i]) {
        p[i] = p[index(x+1,y)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x-1,y+1)]+d2 < r.data[i]) {
        p[i] = p[index(x-1,y+1)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x,y+1)]+d1 < r.data[i]) {
        p[i] = p[index(x,y+1)];
        r.data[i] = dist(x, y, p[i]);
      }
      if (r.data[index(x+1,y+1)]+d2 < r.data[i]) {
        p[i] = p[index(x+1,y+1)];
        r.data[i] = dist(x, y, p[i]);
      }
    }
  }
  
  for (size_t i = 0;i<I.size();++i) {
    if (!I[i]) r.data[i] = -r.data[i];
  }
  
  return r;
}

GLTexture2D Grid2D::toTexture() const {
  GLTexture2D result;
  result.setData(data, uint32_t(width), uint32_t(height), 1);
  return result;
}
