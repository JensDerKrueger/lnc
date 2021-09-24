#include "DeferredShader.h"

static const std::string vertexShader {R"(#version 410
layout (location = 0) in vec3 vPos;
out vec2 texCoords;
void main() {
    gl_Position = vec4(vPos, 1.0);
    texCoords = (vPos.xy+1.0)/2.0;
}
)"};

DeferredShader::DeferredShader(const std::string& fragmentShader,
                               const std::vector<MRTInfo>& offscreenTextures) :
width(0),
height(0),
program{GLProgram::createFromString(vertexShader, fragmentShader)},
fullScreenPosBuffer{GL_ARRAY_BUFFER},
offscreenTextures(offscreenTextures)
{
  std::vector<float> fullscreenTriangle{
    -1, -1, 0,
     3, -1, 0,
    -1,  3, 0,
  };
  fullScreenArray.bind();
  fullScreenPosBuffer.setData(fullscreenTriangle,3);
  fullScreenArray.connectVertexAttrib(fullScreenPosBuffer,program,"vPos",3);
}

void DeferredShader::resize(const uint32_t width, const uint32_t height) {
  this->width = width;
  this->height = height;
  for (auto& offscreenTexture : offscreenTextures) {
    offscreenTexture.resize(width, height);
  }
  offscreenDepth.setSize(width,height);
}

void DeferredShader::startFirstPass() {
  GL(glViewport(0, 0, GLsizei(width), GLsizei(height)));
    
  switch (offscreenTextures.size()) {
    case 1 :
      framebuffer.bind(*offscreenTextures[0].texture, offscreenDepth );
      break;
    case 2 :
      framebuffer.bind(*offscreenTextures[0].texture,
                       *offscreenTextures[1].texture, offscreenDepth );
      break;
    case 3 :
      framebuffer.bind(*offscreenTextures[0].texture,
                       *offscreenTextures[1].texture,
                       *offscreenTextures[2].texture, offscreenDepth );
      break;
    case 4 :
      framebuffer.bind(*offscreenTextures[0].texture,
                       *offscreenTextures[1].texture,
                       *offscreenTextures[2].texture,
                       *offscreenTextures[3].texture, offscreenDepth );
      break;
    default :
      throw GLException{"Invalid MRT count"};
  }
}

void DeferredShader::endFirstPass() {
  framebuffer.unbind2D();
  for (auto& offscreenTexture : offscreenTextures) {
    offscreenTexture.generateMipmap();
  }
}

void DeferredShader::startSecondPass(const uint32_t framebufferWidth,
                                     const uint32_t framebufferHeight) {
  GL(glViewport(0, 0, GLsizei(framebufferWidth), GLsizei(framebufferHeight)));
  program.enable();

  for (size_t i = 0;i<offscreenTextures.size();++i) {
    std::stringstream ss;
    ss << "offscreenTexture" << i;
    program.setTexture(ss.str(),*offscreenTextures[i].texture,GLuint(i+1));
  }
}

void DeferredShader::endSecondPass() {
  fullScreenArray.bind();
  GL(glDrawArrays(GL_TRIANGLES, 0, 3));
  
  for (size_t i = 0;i<offscreenTextures.size();++i) {
    program.unsetTexture2D(GLuint(i));
  }
}
