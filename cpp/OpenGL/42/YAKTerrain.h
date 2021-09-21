#pragma once

#include <thread>
#include <vector>
#include <array>
#include <mutex>
#include <condition_variable>

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
  std::pair<std::vector<ManagedYAK>, AABB> getBricks();

  Vec2ui getSize() const {return size;}
  Vec3i getBrickSize() const {return brickSize;}

private:
  Vec2ui size;
  const Vec3i brickSize{2,2,3};
  Vec3i brickOffset;
  YAKTerrainStatus status{YAKTerrainStatus::Idle};
  Grid2D heightfield;

  std::mutex statusMutex;
  std::condition_variable cv;
  std::thread generationThread;
  float heightScale;
  std::vector<Vec2ui> previousRiverBed;
  float rPos;

  StaticYAKCuller culler;

  void computeBricks();
  Grid2D generateNewHeightfield() const;
  void initFirstPreviousHalfHeightField(size_t w, size_t h);
  void combineNewFieldWithPreviousHalf(Grid2D& newField) const;
  Grid2D generateHeightfield();
  void generateBricksFromField();
  uint32_t sampleField(float normX, float normY) const;
  uint16_t colorMapping(uint32_t height, uint32_t maxGradient) const;
};
