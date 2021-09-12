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
    
  const Grid2D heightfield = generateHeightfield();
  generateBricksFromField(heightfield);
    
  {
    std::unique_lock<std::mutex> lk(statusMutex);
    status = YAKTerrainStatus::Ready;
  }
}

Grid2D YAKTerrain::generateHeightfield() const {
  return Grid2D::genRandom(size.x/20,size.y/20)*40+
         Grid2D::genRandom(size.x/10,size.y/10)*10+
         Grid2D::genRandom(size.x/5,size.y/5)*5+
         Grid2D::genRandom(size.x,size.y);
}

void YAKTerrain::generateBricksFromField(const Grid2D& field) {
  const Vec3i brickSize{2,2,3};
  
  brickData.clear();
  for (uint32_t y = 0; y < size.y; ++y) {
    for (uint32_t x = 0; x < size.x; ++x) {
      const uint32_t height = uint32_t(field.sample(float(x)/size.x, float(y)/size.y));
      
      const int32_t lh = abs(int32_t(height)-int32_t(field.sample(float(x-1)/size.x, float(y)/size.y)));
      const int32_t rh = abs(int32_t(height)-int32_t(field.sample(float(x+1)/size.x, float(y)/size.y)));
      const int32_t th = abs(int32_t(height)-int32_t(field.sample(float(x)/size.x, float(y-1)/size.y)));
      const int32_t bh = abs(int32_t(height)-int32_t(field.sample(float(x)/size.x, float(y+1)/size.y)));
      const int32_t maxHeightDiff = std::max(lh,std::max(rh,std::max(th,bh)));
      
      const uint32_t brickCount = std::max<uint32_t>(1,uint32_t(maxHeightDiff));
      for (uint32_t i = 0;i<brickCount;++i) {
        brickData.push_back(std::make_shared<SimpleYAK42>(
                                                          brickSize.x,brickSize.y,brickSize.z,
                                                          height,
                                                          Vec3i{
                                                                (int32_t(x)-int32_t(size.x)/2)*brickSize.x,
                                                                int32_t(height-i)*brickSize.z,
                                                                (int32_t(y)-int32_t(size.x)/2)*brickSize.y
                                                              }));
      }
    }
  }
}
