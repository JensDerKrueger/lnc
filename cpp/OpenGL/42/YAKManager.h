#pragma once

#include <memory>
#include <vector>
#include <deque>

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
  
  void hideAllStuds() {
    std::fill(studVisible.begin(),studVisible.end(),false);
  }
};

class StaticYAKCuller {
public:
  void add(std::shared_ptr<YAK42> brick);
  void add(const ManagedYAK& brick);
  void cull();
  
  std::pair<std::vector<ManagedYAK>,AABB> get();
  
private:
  AABB aabb;
  std::vector<ManagedYAK> mangedBricks;
  
};

struct InstanceData {  
  GLArray studArray{};
  GLBuffer studInstanceBuffer{GL_ARRAY_BUFFER};
  GLArray baseArray{};
  GLBuffer baseInstanceBuffer{GL_ARRAY_BUFFER};
  
  size_t studInstanceCount;
  size_t baseInstanceCount;
  AABB aabb;
};

class YAKManager {
public:
  YAKManager();
  ~YAKManager() {}
  
  void push(const std::pair<std::vector<ManagedYAK>,AABB>& bricks);
  bool autoPop();
  
  void render() const;

  void setProjection(const Mat4& projection) {
    this->projection = projection;
  }
  
  void setModelView(const Mat4& modelView) {
    this->modelView = modelView;
  }
  
private:
  std::deque<std::shared_ptr<InstanceData>> mangedBricks;

  void generateInstanceData(const std::pair<std::vector<ManagedYAK>,AABB>& bricks);
  void createCommonData();
  void pop();

  GLProgram studShader;

  GLBuffer studPosBuffer;
  GLBuffer studNormalBuffer;
  GLBuffer studIndexBuffer;

  GLProgram baseShader;
  
  GLBuffer basePosBuffer;
  GLBuffer baseNormalBuffer;
  GLBuffer baseIndexBuffer;
    
  size_t studVertexCount;
  size_t baseVertexCount;

  Mat4 projection;
  Mat4 modelView;
  
};
