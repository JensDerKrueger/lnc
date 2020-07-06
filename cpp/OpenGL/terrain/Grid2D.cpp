#include <Rand.h>
#include <Vec2.h>

#include "Grid2D.h"

Grid2D::Grid2D(size_t width, size_t height) :
    width(width),
    height(height),
    data(width*height)
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

float Grid2D::getValue(size_t x, size_t y) const {
    return data[index(x,y)];
}

float Grid2D::sample(float x, float y) const {
    // TODO: check value range
    
    float sx = x*(width-1);
    float sy = y*(height-1);
    
    float alpha = sx - floor(sx);
    float beta  = sy - floor(sy);
    
    Vec2ui a{uint32_t(floor(sx)),uint32_t(floor(sy))};
    Vec2ui b{uint32_t(ceil(sx)),uint32_t(floor(sy))};
    Vec2ui c{uint32_t(floor(sx)),uint32_t(ceil(sy))};
    Vec2ui d{uint32_t(ceil(sx)),uint32_t(ceil(sy))};
    
    float va = getValue(a.x(), a.y());
    float vb = getValue(b.x(), b.y());
    float vc = getValue(c.x(), c.y());
    float vd = getValue(d.x(), d.y());
    
    return (va * (1.0f-alpha) + vb * alpha) * (1.0f-beta) + (vc * (1.0f-alpha) + vd * alpha) * beta;
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

Grid2D Grid2D::operator/(const float& value) const {
    return *this * (1.0f/value);
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
    
size_t Grid2D::index(size_t x, size_t y) const {
    return x + y * width;
}

std::ostream& operator<<(std::ostream &os, const Grid2D& v) {
    os << v.toString() ; return os;
}
