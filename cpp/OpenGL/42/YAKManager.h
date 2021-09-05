#pragma once

#include <memory>
#include <vector>

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
  YAKManager() {}
  ~YAKManager() {}
  
  void add(std::shared_ptr<YAK42> brick);
  
  void compile();
  void render(GLApp& app) const;

private:
  std::vector<ManagedYAK> mangedBricks;

  void flagInvisibleObjects();
  void generateInstanceData();
  void createOpenGLData();
  
};
