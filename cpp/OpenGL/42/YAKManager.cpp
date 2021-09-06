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
  "    color = vInstanceColor;\n"
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
studArray{},
studInstanceBuffer{GL_ARRAY_BUFFER},
studPosBuffer{GL_ARRAY_BUFFER},
studNormalBuffer{GL_ARRAY_BUFFER},
studIndexBuffer{GL_ELEMENT_ARRAY_BUFFER},
baseShader{GLProgram::createFromString(baseVertexShaderString, fragmentShaderString)},
baseArray{},
baseInstanceBuffer{GL_ARRAY_BUFFER},
basePosBuffer{GL_ARRAY_BUFFER},
baseNormalBuffer{GL_ARRAY_BUFFER},
baseIndexBuffer{GL_ELEMENT_ARRAY_BUFFER}
{
  
}

void YAKManager::add(std::shared_ptr<YAK42> brick) {  
  mangedBricks.push_back({brick});
}

void YAKManager::compile() {
  flagInvisibleObjects();
  generateInstanceData();
  createCommonData();
}

void YAKManager::createCommonData() {
  // Studs
  Tesselation studTesselation = Tesselation::genCylinder({},
                                                         YAK42::studRadius,
                                                         YAK42::studHeight,
                                                         false,
                                                         true, 20);

  studShader.enable();
  studArray.bind();
  studPosBuffer.setData(studTesselation.getVertices(), 3);
  studNormalBuffer.setData(studTesselation.getNormals(), 3);
  studIndexBuffer.setData(studTesselation.getIndices());

  studArray.connectVertexAttrib(studPosBuffer, studShader, "vPos", 3);
  studArray.connectVertexAttrib(studNormalBuffer, studShader, "vNormal", 3);
  studArray.connectIndexBuffer(studIndexBuffer);

  studVertexCount = studTesselation.getIndices().size();
  
  // base
  Tesselation baseTesselation = Tesselation::genBrick({},
                                                      Vec3(1.0f,1.0f/3.0f,1.0f));

  
  baseShader.enable();
  baseArray.bind();
  basePosBuffer.setData(baseTesselation.getVertices(), 3);
  baseNormalBuffer.setData(baseTesselation.getNormals(), 3);
  baseIndexBuffer.setData(baseTesselation.getIndices());

  baseArray.connectVertexAttrib(basePosBuffer, baseShader, "vPos", 3);
  baseArray.connectVertexAttrib(baseNormalBuffer, baseShader, "vNormal", 3);
  baseArray.connectIndexBuffer(baseIndexBuffer);
  
  baseVertexCount = baseTesselation.getIndices().size();
}

void YAKManager::flagInvisibleObjects() {
  // TODO: Later
}

void YAKManager::generateInstanceData() {
  
  studInstanceCount = 0;
  baseInstanceCount = 0;
  std::vector<float> studsInstanceData;
  std::vector<float> baseInstanceData;
  
  
  for (const ManagedYAK& managedBrick : mangedBricks) {
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
          
          studInstanceCount++;
        }
      }
      
      // base geometry
      const Vec3 pos   = Vec3(brick->getPos()) * YAK42::brickScale;
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
      
      baseInstanceCount++;
    }
  }
  
  studShader.enable();
  studArray.bind();
  studInstanceBuffer.setData(studsInstanceData, 7);
  studArray.connectVertexAttrib(studInstanceBuffer, studShader, "vInstancePos",
                                3,0,1);
  studArray.connectVertexAttrib(studInstanceBuffer, studShader, "vInstanceColor",
                                4,3,1);

  
  baseShader.enable();
  baseArray.bind();
  baseInstanceBuffer.setData(baseInstanceData, 10);
  baseArray.connectVertexAttrib(baseInstanceBuffer, baseShader, "vInstancePos",
                                3,0,1);
  baseArray.connectVertexAttrib(baseInstanceBuffer, baseShader, "vInstanceScale",
                                3,3,1);
  baseArray.connectVertexAttrib(baseInstanceBuffer, baseShader, "vInstanceColor",
                                4,6,1);

}

void YAKManager::render() const {
  const Mat4 modelViewProjection = projection*modelView;
  const Mat4 modelViewInverseTranspose = Mat4::transpose(Mat4::inverse(modelView));
  
  studShader.enable();
  studArray.bind();
  studShader.setUniform("MVP", modelViewProjection);
  studShader.setUniform("MV", modelView);
  studShader.setUniform("MVit", modelViewInverseTranspose);
  GL(glDrawElementsInstanced(GL_TRIANGLES, GLsizei(studVertexCount),
                             GL_UNSIGNED_INT, (void*)0, GLsizei(studInstanceCount)));

  baseShader.enable();
  baseArray.bind();
  baseShader.setUniform("MVP", modelViewProjection);
  baseShader.setUniform("MV", modelView);
  baseShader.setUniform("MVit", modelViewInverseTranspose);
  GL(glDrawElementsInstanced(GL_TRIANGLES, GLsizei(baseVertexCount),
                             GL_UNSIGNED_INT, (void*)0, GLsizei(baseInstanceCount)));

}

