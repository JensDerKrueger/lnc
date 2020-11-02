#include <limits>

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

Grid2D::Grid2D(const Grid2D& other) :
    width(other.width),
    height(other.height),
    data(other.data)
{
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

MaxData Grid2D::maxValue() const {
    MaxData maxV{std::numeric_limits<float>::min(), Vec2{0,0}};
    for (size_t i = 0;i<data.size();++i) {
        if (maxV.value < data[i]){
            maxV.value = data[i];
            const float x = float(i % width)/(width-1);
            const float y = float(i / width)/(height-1);
            maxV.pos = Vec2{x,y};
        }
    }
    return maxV;
}

size_t Grid2D::index(size_t x, size_t y) const {
    return x + y * width;
}

std::ostream& operator<<(std::ostream &os, const Grid2D& v) {
    os << v.toString() ; return os;
}


Grid2D Grid2D::fromBMP(const std::string& filename) {
    Image bmp = BMP::load(filename);
    
    Grid2D g{bmp.width, bmp.width};
    
    size_t i = 0;
    for (size_t y = 0;y<g.height;++y) {
        for (size_t x = 0;x<g.width;++x) {
            g.data[i] = bmp.getValue(uint32_t(x),uint32_t(y),0)/255.0f;
            i++;
        }
    }

    return g;
}
