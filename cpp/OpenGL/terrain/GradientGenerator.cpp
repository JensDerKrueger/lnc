#include <bmp.h>

#include "GradientGenerator.h"

GradientGenerator::GradientGenerator(size_t texSize) :
    texSize(texSize)
{}

void GradientGenerator::addColor(float pos, const Vec4& color) {
    addColor(PosColor{pos, color});
}

void GradientGenerator::addColor(const PosColor& c) {
    colors.push_back(c);
}

GLTexture1D GradientGenerator::getTexture() const {
    GLTexture1D texture{GL_LINEAR, GL_LINEAR};
    texture.setData(get8BitVector(), texSize, 4);
    return texture;
}

std::vector<GLubyte> GradientGenerator::get8BitVector() const {
    std::vector<PosColor> sortedColors(colors);
    std::sort(sortedColors.begin(), sortedColors.end());
    
    std::vector<GLubyte> textureData(texSize*4);

    if (sortedColors.size() > 0) {
                
        size_t prevIndex = 0;
        size_t nextIndex = 0;
        for (size_t p = 0;p<texSize;++p) {
            const float normIndex = float(p)/float(texSize-1);
            
            if (normIndex > sortedColors[nextIndex].pos) {
                prevIndex = nextIndex;
                nextIndex = nextIndex+1;
            }
            if (normIndex >= sortedColors.back().pos) {
                prevIndex = sortedColors.size()-1;
                nextIndex = sortedColors.size()-1;
            }
                        
            const Vec4& prev = sortedColors[prevIndex].color;
            const Vec4& next = sortedColors[nextIndex].color;
            
                            
            float alpha = (prevIndex == nextIndex) ? 0.5f :  (normIndex-sortedColors[prevIndex].pos)/(sortedColors[nextIndex].pos-sortedColors[prevIndex].pos);
            
            const Vec4 curreColor{prev*(1-alpha)+next*alpha};
            textureData[p*4+0] = GLubyte(curreColor.r()*255);
            textureData[p*4+1] = GLubyte(curreColor.g()*255);
            textureData[p*4+2] = GLubyte(curreColor.b()*255);
            textureData[p*4+3] = GLubyte(curreColor.a()*255);
        }
    }
    
    return textureData;
}

void GradientGenerator::toFile(const std::string& filename) const {
    BMP::Image image{1, uint32_t(texSize), 4, get8BitVector()};
    BMP::save(filename, image);
}
