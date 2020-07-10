#pragma once

#include <vector>

#include <GLTexture1D.h>
#include <Vec4.h>

class PosColor {
public:
    float pos;
    Vec4 color;
    bool operator<(const PosColor& other) const {
        return pos < other.pos;
    }
};

class GradientGenerator {
public:
    GradientGenerator(size_t texSize);
    void addColor(float pos, const Vec4& color);
    void addColor(const PosColor& c);
    
    GLTexture1D getTexture();
    
private:
    size_t texSize;
    std::vector<PosColor> colors;
    
};
