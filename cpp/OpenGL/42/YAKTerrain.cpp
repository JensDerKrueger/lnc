#include "YAKTerrain.h"

#include <iostream>

YAKTerrain::YAKTerrain(const Vec2ui& size) :
size{size},
heightfield{1,1},
generationThread{&YAKTerrain::computeBricks, this},
heightScale(40.0f)
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

    heightfield = generateHeightfield();
    generateBricksFromField();
      
    {
      std::unique_lock<std::mutex> lk(statusMutex);
      status = YAKTerrainStatus::Ready;
    }
  }  while (true);
}


Grid2D YAKTerrain::generateNewHeightfield() const {
  Grid2D newField = Grid2D::genRandom(size.x/20,size.y/10)*40+
                    Grid2D::genRandom(size.x/10,size.y/5)*10+
                    Grid2D::genRandom(size.x/4 ,size.y/2)*4+
                    Grid2D::genRandom(size.x,size.y);
        
  newField.normalize(heightScale);
  return newField;
}

void YAKTerrain::initFirstPreviousHalfHeightField(size_t w, size_t h) {
  heightfield = Grid2D(w,h);
  rPos = w/2;
}

void YAKTerrain::combineNewFieldWithPreviousHalf(Grid2D& newField) const {
  for (size_t y = 0; y < newField.getHeight()/2 ; ++y) {
    const float alpha = float(y) / (newField.getHeight()/2-1);
    for (size_t x = 0; x < newField.getWidth(); ++x) {
      const float c = newField.getValue(x, y);
      const float l = heightfield.getValue(x, y + newField.getHeight()/2);
      newField.setValue(x, y, alpha*c + (1-alpha)*l);
    }
  }
}

void YAKTerrain::addRiver(Grid2D& newField) {
  for (size_t y = 0; y < newField.getHeight()/2+1 ; ++y) {
    const int32_t irPos = int32_t(rPos);
    for (int32_t w = -30; w < 30; ++w) {
      const int32_t curentPos = irPos+w;
      if (curentPos < 0) continue;
      if (curentPos > int32_t(newField.getWidth()-1)) break;
      const float val = newField.getValue(uint32_t(curentPos),y);
      const float height = fabs(sinf(w/15.0f)*15.0f);
      if (val < height) continue;
      newField.setValue(uint32_t(curentPos),y,height);
    }
    rPos = std::clamp<float>(float(rPos) + staticRand.rand11()*2,2,newField.getWidth()-3);
  }
}

Grid2D YAKTerrain::generateHeightfield() {
  Grid2D newField = generateNewHeightfield();
  
  if (heightfield.getWidth() !=newField.getWidth() ||
      heightfield.getHeight() !=newField.getHeight())
    initFirstPreviousHalfHeightField(newField.getWidth(), newField.getHeight());
  
  combineNewFieldWithPreviousHalf(newField);
  addRiver(newField);

  
  return newField;
}

uint32_t YAKTerrain::sampleField(float normX, float normY) const {
  return uint32_t(heightfield.sample(normX, normY/2.0f));
}

uint16_t YAKTerrain::colorMapping(uint32_t height, uint32_t maxGradient) const {
  static std::vector<uint16_t> colorMapping {
    10,10,10,29,  // water
    2,            // beach
    64,           // earth
    18,15,50,62,18,15,50,62, // forrest
    78,79,71,56,67,25,72,  // rocks
    91,91,91,93 // ice cap
  };

  static std::vector<uint16_t> colorMappingHill {
    10,10,10,29,  // water
    52,           // beach
    44,           // earth
    65,78,43,19,78,103,109,28, // forrest
    78,79,71,56,67,25,72,  // rocks
    91,91,91,93 // ice cap
  };

  
  const size_t normHeightIndex = std::min(colorMapping.size()-1, size_t((height/heightScale)*colorMapping.size()-1));

  if (maxGradient/3 > heightScale) {
    return colorMappingHill[normHeightIndex];
  } else {
    return colorMapping[normHeightIndex];
  }
}


void YAKTerrain::generateBricksFromField() {
  for (uint32_t y = 0; y < size.y; ++y) {
    
    const float centerY = float(y)/float(size.y-1);
    const float frontY  = (float(y)+1)/float(size.y-1);
    const float backY   = (float(y)-1)/float(size.y-1);
    
    for (uint32_t x = 0; x < size.x; ++x) {
      
      const float centerX = float(x)/(size.x-1);
      const float rightX  = (float(x)+1)/(size.x-1);
      const float leftX   = (float(x)-1)/(size.x-1);
            
      const uint32_t height = sampleField(centerX, centerY);
      
      const float leftHeight  = sampleField(leftX, centerY);
      const float rigthHeight = sampleField(rightX, centerY);
      const float frontHeight = sampleField(centerX, frontY);
      const float backHeight  = sampleField(centerX, backY);
            
      const uint32_t maxGradient = uint32_t(100*fabs(std::max((leftHeight-rigthHeight)/2.0f,
                                                          (frontHeight-backHeight)/2.0f)));
      
      const int32_t leftHeightBrickDiff  = abs(int32_t(height)-int32_t(leftHeight));
      const int32_t rightHeightBrickDiff = abs(int32_t(height)-int32_t(rigthHeight));
      const int32_t frontHeightBrickDiff = abs(int32_t(height)-int32_t(frontHeight));
      const int32_t backHeightBrickDiff  = abs(int32_t(height)-int32_t(backHeight));
      const int32_t maxHeightBrickDiff = std::max(leftHeightBrickDiff,std::max(rightHeightBrickDiff,std::max(frontHeightBrickDiff,backHeightBrickDiff)));
      const uint32_t brickCount = std::max<uint32_t>(1,uint32_t(maxHeightBrickDiff));
      
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
                                                   colorMapping(height,maxGradient),
                                                   integerPos);

        
        ManagedYAK managedBrick(brick);
        if (i > 0) managedBrick.hideAllStuds();
        culler.add(managedBrick);
      }
    }
  }
  culler.cull();
}
