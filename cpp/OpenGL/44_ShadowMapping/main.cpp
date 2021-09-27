#include <array>

#include <GLApp.h>
#include <Tesselation.h>

static const std::string sceneVertexShader {R"(#version 410
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 MVit;
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
out vec3 normal;
out vec3 pos;
void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    pos = (MV * vec4(vPos, 1.0)).xyz;
    normal = (MVit * vec4(vNormal, 0.0)).xyz;
})"};

static const std::string sceneFragmentShader {R"(#version 410
uniform vec3 color;
uniform float ambient=0.2;
uniform vec3 lightPosViewSpace;
in vec3 pos;
in vec3 normal;
out vec4 FragColor;
void main() {
    vec3 nnormal = normalize(normal);
    vec3 nlightDir = normalize(lightPosViewSpace-pos);
    float light = clamp(clamp(dot(nlightDir,normal),0.0,1.0)+ambient,0.0,1.0);
    FragColor = vec4(color.rgb*light, 1);
})"};

static const std::string lightProbeVertexShader {R"(#version 410
uniform mat4 MVP;
layout (location = 0) in vec3 vPos;
void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
})"};

static const std::string lightProbeFragmentShader {R"(#version 410
out vec4 FragColor;
void main() {
    FragColor = vec4(1,1,1,1);
})"};

class ShadowMappingDemo : public GLApp {
public:
  ShadowMappingDemo() :
    GLApp(640,480,1,"Shadow Mapping Demo",true,false),
    sceneProgram{GLProgram::createFromString(sceneVertexShader,
                                             sceneFragmentShader)},
    lightProbeProgram{GLProgram::createFromString(lightProbeVertexShader,
                                                  lightProbeFragmentShader)},
    torusPosBuffer{GL_ARRAY_BUFFER},
    torusNormalBuffer{GL_ARRAY_BUFFER},
    torusIndexBuffer{GL_ELEMENT_ARRAY_BUFFER},
    planePosBuffer{GL_ARRAY_BUFFER},
    planeNormalBuffer{GL_ARRAY_BUFFER},
    planeIndexBuffer{GL_ELEMENT_ARRAY_BUFFER},
    lightPosBuffer{GL_ARRAY_BUFFER},
    lightIndexBuffer{GL_ELEMENT_ARRAY_BUFFER}
  {
  }
  
  virtual void init() override {
    GL(glDisable(GL_BLEND));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));    
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));
    
    const Tesselation torus = Tesselation::genTorus({0,0,0}, 0.8f, 0.3f);
    torusArray.bind();
    torusPosBuffer.setData(torus.getVertices(), 3);
    torusNormalBuffer.setData(torus.getNormals(), 3);
    torusIndexBuffer.setData(torus.getIndices());
    torusArray.connectVertexAttrib(torusPosBuffer, sceneProgram, "vPos",3);
    torusArray.connectVertexAttrib(torusNormalBuffer, sceneProgram, "vNormal",3);
    torusArray.connectIndexBuffer(torusIndexBuffer);
    torusVertexCount = GLsizei(torus.getIndices().size());
    
    const Tesselation plane = Tesselation::genRectangle({0,0,-2}, 6, 6);
    planeArray.bind();
    planePosBuffer.setData(plane.getVertices(), 3);
    planeNormalBuffer.setData(plane.getNormals(), 3);
    planeIndexBuffer.setData(plane.getIndices());
    planeArray.connectVertexAttrib(planePosBuffer, sceneProgram, "vPos",3);
    planeArray.connectVertexAttrib(planeNormalBuffer, sceneProgram, "vNormal",3);
    planeArray.connectIndexBuffer(planeIndexBuffer);
    planeVertexCount = GLsizei(plane.getIndices().size());

    const Tesselation light = Tesselation::genSphere({0,0,0}, 0.1f, 10, 10);
    lightArray.bind();
    lightPosBuffer.setData(light.getVertices(), 3);
    lightIndexBuffer.setData(light.getIndices());
    lightArray.connectVertexAttrib(lightPosBuffer, lightProbeProgram, "vPos",3);
    lightArray.connectIndexBuffer(lightIndexBuffer);
    lightVertexCount = GLsizei(light.getIndices().size());

  }
      
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition,
                          double yPosition) override {
  }
  
  virtual void resize(int width, int height) override {
    GL(glViewport(0, 0, GLsizei(width), GLsizei(height)));
  }
      
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
      }
    }
  }
  
  virtual void animate(double animationTime) override {
    this->animationTime = float(animationTime);
    
    const Dimensions dim = glEnv.getFramebufferSize();

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();

    projection = Mat4::perspective(fovY, aspect, zNear, zFar);    
    view       = Mat4::lookAt({0,8,8}, {0,0,0}, {0,1,0});
    torusModel = Mat4::rotationY(this->animationTime*80) *
                 Mat4::rotationX(this->animationTime*70);
    planeModel = Mat4::rotationX(90);

    torusColor = Vec3{
      fabs(sinf(this->animationTime*0.3f)),
      fabs(sinf(this->animationTime*0.8f)),
      fabs(sinf(this->animationTime*0.1f))
    };
    
    const Vec3 lightPos = Vec3{1,4,0};
    lightModel = Mat4::rotationY(this->animationTime*50) *
                 Mat4::translation(lightPos);
  }
  
  virtual void draw() override {
    GL(glDisable(GL_BLEND));
    GL(glClearColor(0,0,1,0));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    sceneProgram.enable();
    sceneProgram.setUniform("lightPosViewSpace", view*lightModel*Vec3{0,0,0});

    torusArray.bind();
    
    Mat4 modelView = view*torusModel;
    Mat4 modelViewProjection = projection*modelView;
    Mat4 modelViewInverseTranspose = Mat4::transpose(Mat4::inverse(modelView));
    
    sceneProgram.setUniform("MVP",  modelViewProjection);
    sceneProgram.setUniform("MV",   modelView);
    sceneProgram.setUniform("MVit", modelViewInverseTranspose);
    sceneProgram.setUniform("color", torusColor);
    GL(glDrawElements(GL_TRIANGLES, torusVertexCount, GL_UNSIGNED_INT,
                      (void*)0));

    planeArray.bind();
    modelView = view*planeModel;
    modelViewProjection = projection*modelView;
    modelViewInverseTranspose = Mat4::transpose(Mat4::inverse(modelView));
    
    sceneProgram.setUniform("MVP",  modelViewProjection);
    sceneProgram.setUniform("MV",   modelView);
    sceneProgram.setUniform("MVit", modelViewInverseTranspose);
    sceneProgram.setUniform("color", Vec3{0.5,0.5,0.5});
    GL(glDrawElements(GL_TRIANGLES, planeVertexCount, GL_UNSIGNED_INT,
                      (void*)0));

    lightProbeProgram.enable();
    lightArray.bind();
    lightProbeProgram.setUniform("MVP",  projection*view*lightModel);
    GL(glDrawElements(GL_TRIANGLES, lightVertexCount, GL_UNSIGNED_INT,
                      (void*)0));
  }
  
private:
  Vec2 mousePos;
  float animationTime;
  Mat4 view;
  Mat4 projection;
  
  GLProgram   sceneProgram;
  GLProgram   lightProbeProgram;

  Vec3        torusColor;
  Mat4        torusModel;
  GLArray     torusArray;
  GLBuffer    torusPosBuffer;
  GLBuffer    torusNormalBuffer;
  GLBuffer    torusIndexBuffer;
  GLsizei     torusVertexCount;

  Mat4        planeModel;
  GLArray     planeArray;
  GLBuffer    planePosBuffer;
  GLBuffer    planeNormalBuffer;
  GLBuffer    planeIndexBuffer;
  GLsizei     planeVertexCount;

  Mat4        lightModel;
  GLArray     lightArray;
  GLBuffer    lightPosBuffer;
  GLBuffer    lightIndexBuffer;
  GLsizei     lightVertexCount;

};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,
            INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    ShadowMappingDemo myApp;
    myApp.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
