#include "YAKManager.h"

#include <Tesselation.h>

static const std::string studVertexShaderString{
  "#version 410\n"
  "uniform mat4 MVP;\n"
  "uniform mat4 MV;\n"
  "uniform mat4 MVit;\n"
  "layout (location = 0) in vec3 vPos;\n"
  "layout (location = 1) in vec3 vNormal;\n"
  "layout (location = 2) in vec3 vInstancePos;\n"
  "layout (location = 3) in vec4 vInstanceColor;\n"
  "out vec4 color;\n"
  "out vec3 normal;\n"
  "out vec3 pos;\n"
  "void main() {\n"
  "    vec3 actualPos = vPos + vInstancePos;\n"
  "    gl_Position = MVP * vec4(actualPos, 1.0);\n"
  "    pos = (MV * vec4(actualPos, 1.0)).xyz;\n"
  "    color = vInstanceColor;\n"
  "    normal = (MVit * vec4(vNormal, 0.0)).xyz;\n"
  "}\n"
};

static const std::string baseVertexShaderString{
  "#version 410\n"
  "uniform mat4 MVP;\n"
  "uniform mat4 MV;\n"
  "uniform mat4 MVit;\n"
  "layout (location = 0) in vec3 vPos;\n"
  "layout (location = 1) in vec3 vNormal;\n"
  "layout (location = 2) in vec3 vInstancePos;\n"
  "layout (location = 3) in vec3 vInstanceScale;\n"
  "layout (location = 4) in vec4 vInstanceColor;\n"
  "out vec4 color;\n"
  "out vec3 normal;\n"
  "out vec3 pos;\n"
  "void main() {\n"
  "    vec3 actualPos = vPos*vInstanceScale + vInstancePos;\n"
  "    gl_Position = MVP * vec4(actualPos, 1.0);\n"
  "    pos = (MV * vec4(actualPos, 1.0)).xyz;\n"
  "    color = vec4(vInstanceColor.x, vInstanceColor.y, vInstanceColor.z, 0.1);\n"
  "    normal = (MVit * vec4(vNormal, 0.0)).xyz;\n"
  "}\n"
};

static const std::string fragmentShaderString{
  "#version 410\n"
  "in vec4 color;\n"
  "in vec3 pos;\n"
  "in vec3 normal;\n"
  "out vec4 FragColor;\n"
  "void main() {\n"
  "    vec3 nnormal = normalize(normal);"
  "    vec3 nlightDir = normalize(vec3(0.0,0.0,0.0)-pos);"
  "    FragColor = color*abs(dot(nlightDir,nnormal));\n"
  "}\n"
};


ManagedYAK::ManagedYAK(std::shared_ptr<YAK42> brick) :
  brick(brick),
  visible(true)
{
  studVisible.resize(brick->studsTop().size());
  std::fill(studVisible.begin(), studVisible.end(), true);
}



YAKManager::YAKManager() :
studShader{GLProgram::createFromString(studVertexShaderString, fragmentShaderString)},
studPosBuffer{GL_ARRAY_BUFFER},
studNormalBuffer{GL_ARRAY_BUFFER},
studIndexBuffer{GL_ELEMENT_ARRAY_BUFFER},
baseShader{GLProgram::createFromString(baseVertexShaderString, fragmentShaderString)},
basePosBuffer{GL_ARRAY_BUFFER},
baseNormalBuffer{GL_ARRAY_BUFFER},
baseIndexBuffer{GL_ELEMENT_ARRAY_BUFFER}
{
  createCommonData();
}

void YAKManager::push(const std::pair<std::vector<ManagedYAK>,AABB>& bricks) {
  generateInstanceData(bricks);
}

void YAKManager::pop() {
  if (!mangedBricks.empty()) mangedBricks.pop_front();
}

void YAKManager::createCommonData() {
  // Studs
  Tesselation studTesselation = Tesselation::genCylinder({},
                                                         YAK42::studRadius,
                                                         YAK42::studHeight,
                                                         false,
                                                         true, 20);

  studShader.enable();
  studPosBuffer.setData(studTesselation.getVertices(), 3);
  studNormalBuffer.setData(studTesselation.getNormals(), 3);
  studIndexBuffer.setData(studTesselation.getIndices());


  studVertexCount = studTesselation.getIndices().size();
  
  // base
  Tesselation baseTesselation = Tesselation::genBrick({},
                                                      Vec3(1.0f,1.0f/3.0f,1.0f));

  
  baseShader.enable();
  basePosBuffer.setData(baseTesselation.getVertices(), 3);
  baseNormalBuffer.setData(baseTesselation.getNormals(), 3);
  baseIndexBuffer.setData(baseTesselation.getIndices());

  baseVertexCount = baseTesselation.getIndices().size();
}

void YAKManager::generateInstanceData(const std::pair<std::vector<ManagedYAK>,AABB>& bricks) {
  mangedBricks.push_back(std::make_shared<InstanceData>());
  
  std::vector<float> studsInstanceData;
  std::vector<float> baseInstanceData;

  std::shared_ptr<InstanceData> newInstance = mangedBricks.back();
  newInstance->aabb = bricks.second;
  
  newInstance->studInstanceCount = 0;
  newInstance->baseInstanceCount = 0;  
    
  for (const ManagedYAK& managedBrick : bricks.first) {

    if (managedBrick.visible) {
      
      const auto brick = std::dynamic_pointer_cast<SimpleYAK42>(managedBrick.brick);
      
      if (!brick) {
        // TODO: handle other brick types, if the ever see the light of day
        continue;
      }
          
      // studs
      const Vec4 color = brick->getColor();
      for (size_t i = 0; i<managedBrick.studVisible.size(); ++i) {
        if (managedBrick.studVisible[i]) {
          const Vec3 pos = brick->computeGlobalStudPos(i);
          studsInstanceData.push_back(pos.x);
          studsInstanceData.push_back(pos.y);
          studsInstanceData.push_back(pos.z);
          
          studsInstanceData.push_back(color.r);
          studsInstanceData.push_back(color.g);
          studsInstanceData.push_back(color.b);
          studsInstanceData.push_back(color.a);
          
          newInstance->studInstanceCount++;
        }
      }
      
      // base geometry
      const Vec3 pos   = brick->getPos() * YAK42::brickScale;
      const Vec3 scale = Vec3(brick->getScale()) * YAK42::brickScale;

      baseInstanceData.push_back(pos.x);
      baseInstanceData.push_back(pos.y);
      baseInstanceData.push_back(pos.z);

      baseInstanceData.push_back(scale.x);
      baseInstanceData.push_back(scale.y);
      baseInstanceData.push_back(scale.z);

      baseInstanceData.push_back(color.r);
      baseInstanceData.push_back(color.g);
      baseInstanceData.push_back(color.b);
      baseInstanceData.push_back(color.a);
      
      newInstance->baseInstanceCount++;
    }
  }
  
  studShader.enable();
  newInstance->studArray.bind();
  

  newInstance->studInstanceBuffer.setData(studsInstanceData, 7);
  newInstance->studArray.connectVertexAttrib(newInstance->studInstanceBuffer, studShader, "vInstancePos",
                                3,0,1);
  newInstance->studArray.connectVertexAttrib(newInstance->studInstanceBuffer, studShader, "vInstanceColor",
                                4,3,1);

  newInstance->studArray.connectVertexAttrib(studPosBuffer, studShader, "vPos", 3);
  newInstance->studArray.connectVertexAttrib(studNormalBuffer, studShader, "vNormal", 3);
  newInstance->studArray.connectIndexBuffer(studIndexBuffer);

  baseShader.enable();
  newInstance->baseArray.bind();
  
  newInstance->baseInstanceBuffer.setData(baseInstanceData, 10);
  newInstance->baseArray.connectVertexAttrib(newInstance->baseInstanceBuffer, baseShader, "vInstancePos",
                                3,0,1);
  newInstance->baseArray.connectVertexAttrib(newInstance->baseInstanceBuffer, baseShader, "vInstanceScale",
                                3,3,1);
  newInstance->baseArray.connectVertexAttrib(newInstance->baseInstanceBuffer, baseShader, "vInstanceColor",
                                4,6,1);

  newInstance->baseArray.connectVertexAttrib(basePosBuffer, baseShader, "vPos", 3);
  newInstance->baseArray.connectVertexAttrib(baseNormalBuffer, baseShader, "vNormal", 3);
  newInstance->baseArray.connectIndexBuffer(baseIndexBuffer);
}

void YAKManager::render() const {
  const Mat4 modelViewProjection = projection*modelView;
  const Mat4 modelViewInverseTranspose = Mat4::transpose(Mat4::inverse(modelView));
  
  studShader.enable();
  studShader.setUniform("MVP", modelViewProjection);
  studShader.setUniform("MV", modelView);
  studShader.setUniform("MVit", modelViewInverseTranspose);

  for (const auto& instance : mangedBricks) {
    instance->studArray.bind();
    GL(glDrawElementsInstanced(GL_TRIANGLES, GLsizei(studVertexCount),
                               GL_UNSIGNED_INT, (void*)0, GLsizei(instance->studInstanceCount)));
  }
   
  baseShader.enable();
  baseShader.setUniform("MVP", modelViewProjection);
  baseShader.setUniform("MV", modelView);
  baseShader.setUniform("MVit", modelViewInverseTranspose);
  for (const auto& instance : mangedBricks) {
    instance->baseArray.bind();
    GL(glDrawElementsInstanced(GL_TRIANGLES, GLsizei(baseVertexCount),
                               GL_UNSIGNED_INT, (void*)0, GLsizei(instance->baseInstanceCount)));
  }
}

bool YAKManager::autoPop(const Mat4& modelView,
                         const std::array<Vec3, 8>& frustumPoints) {
  
  if (mangedBricks.empty()) return false;
  
  bool reject = false;
  const AABB& aabb = mangedBricks.front()->aabb;
  const std::array<Vec3, 2> arrayAABB = {aabb.maxVec, aabb.minVec};
  
  static const std::array<Vec3ui, 6> pointsToSide = {
    Vec3ui{4,5,0},
    Vec3ui{0,3,7},
    Vec3ui{0,1,2},
    Vec3ui{7,3,2},
    Vec3ui{2,1,5},
    Vec3ui{7,6,4}
  };
  
  for (size_t side = 0;side<6; ++side) {
    const Vec3 corner = arrayAABB[side/3];
    const Vec3 p0 = frustumPoints[pointsToSide[side][0]];
    const Vec3 p1 = frustumPoints[pointsToSide[side][1]];
    const Vec3 p2 = frustumPoints[pointsToSide[side][2]];
    const Vec3 normal = Vec3::cross(p0-p1,p0-p2);
    const float d = Vec3::dot(p0-corner,normal);
    if (d < 0) {
      reject = true;
      break;
    }
  }
    
  if (reject) pop();
  return reject;
}


void StaticYAKCuller::add(std::shared_ptr<YAK42> brick) {
  mangedBricks.push_back({brick});
}

void StaticYAKCuller::add(const ManagedYAK& brick) {
  mangedBricks.push_back(brick);
}

void StaticYAKCuller::cull() {
 // TODO: später mehr dazu
  
  aabb = mangedBricks.begin()->brick->computeAABB();
  for (const auto& managedBrick : mangedBricks) {
    aabb.merge(managedBrick.brick->computeAABB());
  }
}

std::pair<std::vector<ManagedYAK>,AABB> StaticYAKCuller::get() {
  std::vector<ManagedYAK> temp;
  std::swap(temp, mangedBricks);
  return std::make_pair(temp,aabb);
}
