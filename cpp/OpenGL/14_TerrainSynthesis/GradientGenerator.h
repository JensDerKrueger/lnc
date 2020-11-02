#pragma once

#include <vector>
#include <memory>
#include <exception>

#include <bmp.h>
#include <GLTexture1D.h>
#include <Vec4.h>


class GradientGeneratorException : public std::exception {
    public:
        GradientGeneratorException(const std::string& whatStr) : whatStr(whatStr) {}
        virtual const char* what() const throw() {
            return whatStr.c_str();
        }
    private:
        std::string whatStr;
};

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
    void addColor(float pos, const Vec4& color);
    void addColor(const PosColor& c);
        
    GLTexture1D getTexture(size_t texSize) const;
    
    void toFile(const std::string& filename, size_t texSize) const;
    
    static Image buildImage(const std::vector<std::shared_ptr<GradientGenerator>>& gens, size_t texSize);
    
private:
    std::vector<PosColor> colors;
    
    std::vector<GLubyte> get8BitVector(size_t texSize) const;
    
};
