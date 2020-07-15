#include "GradientGenerator.h"

void GradientGenerator::addColor(float pos, const Vec4& color) {
    addColor(PosColor{pos, color});
}

void GradientGenerator::addColor(const PosColor& c) {
    colors.push_back(c);
}

GLTexture1D GradientGenerator::getTexture(size_t texSize) const {
    GLTexture1D texture{GL_LINEAR, GL_LINEAR};
    texture.setData(get8BitVector(texSize), texSize, 4);
    return texture;
}

std::vector<GLubyte> GradientGenerator::get8BitVector(size_t texSize) const {
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

void GradientGenerator::toFile(const std::string& filename, size_t texSize) const {
    BMP::Image image{1, uint32_t(texSize), 4, get8BitVector(texSize)};
    BMP::save(filename, image);
}

BMP::Image GradientGenerator::buildImage(const std::vector<std::shared_ptr<GradientGenerator>>& gens, size_t texSize) {
    if (gens.empty())
        throw GradientGeneratorException("Generator vector cannot be empty!");
    
    const size_t width = gens.size();
    const size_t height = texSize;
    
    std::vector<GLubyte> data(width*height*4);
    size_t genIndex = 0;
    for (const auto gen : gens) {
        std::vector<GLubyte> genData = gen->get8BitVector(texSize);
    
        for (size_t i = 0; i<genData.size()/4;++i) {
            data[(width*i+genIndex)*4+0] = genData[i*4+0];
            data[(width*i+genIndex)*4+1] = genData[i*4+1];
            data[(width*i+genIndex)*4+2] = genData[i*4+2];
            data[(width*i+genIndex)*4+3] = genData[i*4+3];
        }
        
        ++genIndex;
    }
    
    
    BMP::Image image{uint32_t(width), uint32_t(height), 4, data};
    return image;
}
