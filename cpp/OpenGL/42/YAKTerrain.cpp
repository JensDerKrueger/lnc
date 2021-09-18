#include "YAKTerrain.h"

#include <iostream>

YAKTerrain::YAKTerrain(const Vec2ui& size) :
size{size},
lastField{1,1},
generationThread{&YAKTerrain::computeBricks, this},
colorMapping{31,32,10,2,11,27,6,19,12,1,14,25,31,32,10,2,11,27,6,19,12,1,14,25,31,32,10,2,11,27,6,19,12,1,14,25}
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

void YAKTerrain::requestBricks(const Vec3i& offset) {
  if (status != YAKTerrainStatus::Idle) return;
  
  {
    std::lock_guard<std::mutex> lk(statusMutex);
    brickOffset = offset;
    status = YAKTerrainStatus::Computing;
  }
  cv.notify_one();
}

bool YAKTerrain::bricksReady() {
  std::unique_lock<std::mutex> lk(statusMutex);
  return status == YAKTerrainStatus::Ready;
}

std::pair<std::vector<ManagedYAK>, AABB> YAKTerrain::getBricks() {
  std::unique_lock<std::mutex> lk(statusMutex);
  if (status != YAKTerrainStatus::Ready) return {};
  status = YAKTerrainStatus::Idle;
  return culler.get();
}

void YAKTerrain::computeBricks() {
  do {
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
  }  while (true);
}

Grid2D YAKTerrain::generateHeightfield() const {
  Grid2D g = Grid2D::genRandom(size.x/20,size.y/10)*40+
             Grid2D::genRandom(size.x/10,size.y/5)*10+
             Grid2D::genRandom(size.x/4,size.y/2)*4;
    
  g.normalize();
  return g*colorMapping.size();
}

uint32_t YAKTerrain::sampleField(const Grid2D& field,
                                 const float normX, const float normY) const {
  return uint32_t(normY*lastField.sample(normX, normY/2.0f)+
                  (1-normY)*field.sample(normX, normY/2.0f+0.5f));
}

void YAKTerrain::generateBricksFromField(const Grid2D& field) {
  for (uint32_t y = 0; y < size.y; ++y) {
    
    const float centerY = float(y)/float(size.y-1);
    const float frontY  = (float(y)+1)/float(size.y-1);
    const float backY   = (float(y)-1)/float(size.y-1);
    
    for (uint32_t x = 0; x < size.x; ++x) {
      
      const float centerX = float(x)/(size.x-1);
      const float rightX  = (float(x)+1)/(size.x-1);
      const float leftX   = (float(x)-1)/(size.x-1);
            
      const uint32_t height = sampleField(field, centerX, centerY);
      
      const int32_t lh = abs(int32_t(height)-int32_t(sampleField(field, leftX, centerY)));
      const int32_t rh = abs(int32_t(height)-int32_t(sampleField(field, rightX, centerY)));
      const int32_t fh = abs(int32_t(height)-int32_t(sampleField(field, centerX, frontY)));
      const int32_t bh = abs(int32_t(height)-int32_t(sampleField(field, centerX, backY)));
      const int32_t maxHeightDiff = std::max(lh,std::max(rh,std::max(fh,bh)));
      const uint32_t brickCount = std::max<uint32_t>(1,uint32_t(maxHeightDiff));
      
      for (uint32_t i = 0;i<brickCount;++i) {

        const Vec3i relativeIntegerPos{
          (int32_t(x)-int32_t(size.x)/2)*brickSize.x,
          int32_t(height-i)*brickSize.z,
          (int32_t(y)-int32_t(size.y)/2)*brickSize.y
        };
        
        const Vec3i integerPos = brickOffset+relativeIntegerPos;
        
        auto brick = std::make_shared<SimpleYAK42>(brickSize.x,
                                                   brickSize.y,
                                                   brickSize.z,
                                                   colorMapping[height%colorMapping.size()],
                                                   integerPos);

        
        ManagedYAK managedBrick(brick);
        if (i > 0) managedBrick.hideAllStuds();
        culler.add(managedBrick);
      }
    }
  }
  
  lastField = field;
  culler.cull();
}
