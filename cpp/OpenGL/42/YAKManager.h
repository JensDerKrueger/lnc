#pragma once

#include <memory>
#include <vector>

#include <Mat4.h>
#include <GLProgram.h>
#include <GLBuffer.h>
#include <GLArray.h>

#include "YAK42.h"

class ManagedYAK {
public:
  ManagedYAK(std::shared_ptr<YAK42> brick);
  
  std::shared_ptr<YAK42> brick;
  bool visible;
  std::vector<bool> studVisible;
};

class YAKManager {
public:
  YAKManager();
  ~YAKManager() {}
  
  void add(std::shared_ptr<YAK42> brick);
  
  void compile();
  void render() const;

  void setProjection(const Mat4& projection) {
    this->projection = projection;
  }
  
  void setModelView(const Mat4& modelView) {
    this->modelView = modelView;
  }
  
private:
  std::vector<ManagedYAK> mangedBricks;

  void flagInvisibleObjects();
  void generateInstanceData();
  void createCommonData();
  
  GLProgram studShader;
  GLArray studArray;
  GLBuffer studInstanceBuffer;

  GLBuffer studPosBuffer;
  GLBuffer studNormalBuffer;
  GLBuffer studIndexBuffer;

  GLProgram baseShader;
  GLArray baseArray;
  GLBuffer baseInstanceBuffer;
  
  GLBuffer basePosBuffer;
  GLBuffer baseNormalBuffer;
  GLBuffer baseIndexBuffer;
  
  size_t studVertexCount;
  size_t studInstanceCount;
  size_t baseVertexCount;
  size_t baseInstanceCount;
  
  Mat4 projection;
  Mat4 modelView;
  
};
