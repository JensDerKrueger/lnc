#include "YAKManager.h"

ManagedYAK::ManagedYAK(std::shared_ptr<YAK42> brick) :
  brick(brick),
  visible(true)
{
  studVisible.resize(brick->studsTop().size());
  std::fill(studVisible.begin(), studVisible.end(), true);
}

void YAKManager::add(std::shared_ptr<YAK42> brick) {  
  mangedBricks.push_back({brick});
}

void YAKManager::compile() {
  flagInvisibleObjects();
  generateInstanceData();
  createOpenGLData();
}

void YAKManager::createOpenGLData() {
  // TODO: Later
}

void YAKManager::flagInvisibleObjects() {
  // TODO: Later
}

void YAKManager::generateInstanceData() {
  for (const ManagedYAK& managedBrick : mangedBricks) {
    if (managedBrick.visible) {
      // studs
      const Vec4 color = managedBrick.brick->getColor();
      for (size_t i = 0; i<managedBrick.studVisible.size(); ++i) {
        if (managedBrick.studVisible[i]) {
          const Vec3 pos = managedBrick.brick->computeGlobalStudPos(i);
          // TODO: store pos and color in opengl-buffer
        }
      }
      
      // base geometry
      
      // TODO: store pos, size and color in opengl-buffer

    }
  }
}

void YAKManager::render(GLApp& app) const {
  for (const ManagedYAK& mangedBrick : mangedBricks) {
    if (mangedBrick.visible) {
      mangedBrick.brick->render(app);
    }
  }
}
