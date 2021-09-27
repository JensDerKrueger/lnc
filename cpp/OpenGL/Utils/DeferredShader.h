#pragma once

#include <string>
#include <array>
#include <memory>

#include "GLProgram.h"
#include "GLFramebuffer.h"
#include "GLArray.h"
#include "GLBuffer.h"
#include "GLTexture2D.h"
#include "GLDepthBuffer.h"

struct MRTInfo {
  MRTInfo(uint32_t componentCount, GLDataType type, bool genMip) :
    componentCount(componentCount),
    type(type),
    genMip(genMip) {
      texture = std::make_shared<GLTexture2D>(GL_LINEAR,
                                              genMip ? GL_LINEAR_MIPMAP_LINEAR
                                                     : GL_LINEAR);
    }

  void resize(const uint32_t width, const uint32_t height) {
    texture->setEmpty(width, height, componentCount, type);
  }

  void generateMipmap() {
    if (genMip) texture->generateMipmap();
  }

  std::shared_ptr<GLTexture2D> texture;
  uint32_t componentCount;
  GLDataType type;
  bool genMip;
};

class DeferredShader {
public:
  DeferredShader(const std::string& fragmentShader,
                 const std::vector<MRTInfo>& offscreenTextures);

  void resize(const uint32_t width, const uint32_t height);

  void startFirstPass();
  void endFirstPass();

  void startSecondPass(const uint32_t framebufferWidth,
                       const uint32_t framebufferHeight);
  void endSecondPass();

  GLProgram& getProgram() {return program;}

private:
  uint32_t width;
  uint32_t height;

  GLProgram program;
  GLFramebuffer framebuffer;
  GLArray     fullScreenArray;
  GLBuffer    fullScreenPosBuffer;
  std::vector<MRTInfo> offscreenTextures;
  GLDepthBuffer offscreenDepth;

};
