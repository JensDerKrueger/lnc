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

GLTexture1D GradientGenerator::getTexture() {
    std::sort(colors.begin(), colors.end());
    
    std::vector<GLubyte> textureData(texSize*4);

    if (colors.size() > 0) {
                
        size_t prevIndex = 0;
        size_t nextIndex = 0;
        for (size_t p = 0;p<texSize;++p) {
            const float normIndex = float(p)/float(texSize-1);
            
            if (normIndex > colors[nextIndex].pos) {
                prevIndex = nextIndex;
                nextIndex = nextIndex+1;
            }
            if (normIndex >= colors.back().pos) {
                prevIndex = colors.size()-1;
                nextIndex = colors.size()-1;
            }
                        
            const Vec4& prev = colors[prevIndex].color;
            const Vec4& next = colors[nextIndex].color;
            
                            
            float alpha = (prevIndex == nextIndex) ? 0.5f :  (normIndex-colors[prevIndex].pos)/(colors[nextIndex].pos-colors[prevIndex].pos);
            
            const Vec4 curreColor{prev*(1-alpha)+next*alpha};
            textureData[p*4+0] = GLubyte(curreColor.r()*255);
            textureData[p*4+1] = GLubyte(curreColor.g()*255);
            textureData[p*4+2] = GLubyte(curreColor.b()*255);
            textureData[p*4+3] = GLubyte(curreColor.a()*255);
        }
    }
    
    GLTexture1D texture{GL_LINEAR, GL_LINEAR};
    texture.setData(textureData, texSize, 4);
    return texture;
}
