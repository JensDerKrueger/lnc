#pragma once

#include <thread>
#include <vector>

#include "YAK42.h"

enum class YAKTerrainStatus {
  Idle,
  Computing,
  Ready,
  Terminate
};

class YAKTerrain {
public:
  YAKTerrain(const Vec2ui& size);
  ~YAKTerrain();
    
  void requestBricks();
  bool bricksReady();
  std::vector<std::shared_ptr<YAK42>> getBricks();
  
private:
  std::vector<std::shared_ptr<YAK42>> brickData;
  Vec2ui size;
  YAKTerrainStatus status{YAKTerrainStatus::Idle};  
  
  void computeBricks();
  
  std::mutex statusMutex;
  std::condition_variable cv;
  
  std::thread generationThread;
};
