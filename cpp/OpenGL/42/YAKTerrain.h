#pragma once

#include <thread>
#include <vector>

#include <Grid2D.h>

#include "YAK42.h"
#include "YAKManager.h"

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
    
  void requestBricks(const Vec3i& offset=Vec3i{0,0,0});
  bool bricksReady();
  std::vector<ManagedYAK> getBricks();
  
  Vec2ui getSize() const {return size;}
  Vec3i getBrickSize() const {return brickSize;}
  
private:
  Vec2ui size;
  const Vec3i brickSize{2,2,3};
  Vec3i brickOffset;
  YAKTerrainStatus status{YAKTerrainStatus::Idle};  
    
  std::mutex statusMutex;
  std::condition_variable cv;
  std::thread generationThread;
  
  StaticYAKCuller culler;
  
  void computeBricks();
  Grid2D generateHeightfield() const;
  void generateBricksFromField(const Grid2D& field);

};
