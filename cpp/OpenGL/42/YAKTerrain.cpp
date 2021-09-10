#include "YAKTerrain.h"

#include <iostream>

YAKTerrain::YAKTerrain(const Vec2ui& size) :
size{size},
generationThread{&YAKTerrain::computeBricks, this}
{
}

YAKTerrain::~YAKTerrain() {
  {
    std::lock_guard<std::mutex> lk(statusMutex);
    status = YAKTerrainStatus::Terminate;
  }
  cv.notify_one();

  generationThread.join();
}

void YAKTerrain::requestBricks() {
  if (status != YAKTerrainStatus::Idle) return;
  
  {
    std::lock_guard<std::mutex> lk(statusMutex);
    status = YAKTerrainStatus::Computing;
  }
  cv.notify_one();
}

bool YAKTerrain::bricksReady() {
  std::unique_lock<std::mutex> lk(statusMutex);
  return status == YAKTerrainStatus::Ready;
}

std::vector<std::shared_ptr<YAK42>> YAKTerrain::getBricks() {
  std::unique_lock<std::mutex> lk(statusMutex);
  if (status != YAKTerrainStatus::Ready) return {};
  status = YAKTerrainStatus::Idle;
  return std::move(brickData);
}

void YAKTerrain::computeBricks() {
  {
    std::unique_lock<std::mutex> lk(statusMutex);
    cv.wait(lk, [this]{return this->status == YAKTerrainStatus::Computing
                           || this->status == YAKTerrainStatus::Terminate;});
    if (this->status == YAKTerrainStatus::Terminate) return;
  }
    
  brickData.clear();  
  for (int32_t y = -int32_t(size.y)/2; y < int32_t(size.y)/2; ++y) {
    for (int32_t x = -int32_t(size.x)/2; x < int32_t(size.x)/2; ++x) {
      const int32_t height = 0; // TODO
      brickData.push_back(std::make_shared<SimpleYAK42>(2,2,1*3,
                                                        Rand::rand<uint16_t>(0,YAK42::colors.size()),
                                                        Vec3i{x*2,height,y*2}));
    }
  }
  
  {
    std::unique_lock<std::mutex> lk(statusMutex);
    status = YAKTerrainStatus::Ready;
  }
}
