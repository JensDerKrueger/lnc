#include <array>
#include <vector>

#include <GLApp.h>
#include <FontRenderer.h>
#include <DeferredShader.h>
#include <Rand.h>

#include "YAK42.h"
#include "YAKTerrain.h"
#include "YAKManager.h"

static const std::string dsFragmentShaderString {R"(#version 410
uniform sampler2D offscreenTexture0;
uniform sampler2D offscreenTexture1;
uniform sampler2D offscreenTexture2;
uniform float focalDepth;
in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

vec2 kernel[4] = vec2[4](vec2(1,0), vec2(-1,0), vec2(0,1), vec2(0,-1));

void main() {
    vec4 pos    = textureLod(offscreenTexture1, texCoords,0);
    float distToFocus = max(log(abs(focalDepth+pos.z)*10),0.0001f);

    vec2 offset = 1.0/textureSize(offscreenTexture0, int(distToFocus));

    vec3 color = textureLod(offscreenTexture0, texCoords, distToFocus).rgb;
    vec3 normal = textureLod(offscreenTexture2, texCoords, distToFocus).rgb;

    for (int i = 0;i<4;++i) {
      color += textureLod(offscreenTexture0, texCoords+kernel[i]*offset, distToFocus).rgb;
      normal += textureLod(offscreenTexture0, texCoords+kernel[i]*offset, distToFocus).rgb;
    }
    color /= 5;
    normal /= 5;
    vec3 nlightDir = normalize(vec3(0,1,4));

    FragColor = vec4(color.rgb*clamp(dot(nlightDir,normal)+0.3,0,1), pos.a);
})"};

class YAK42App : public GLApp {
public:
  
  YAK42App() :
    GLApp(640,480,1,"YAK42",true,false),
    deferredShader(dsFragmentShaderString, {
      {3,GLDataType::BYTE,true},
      {4,GLDataType::HALF,false},
      {3,GLDataType::HALF,true}
    })
  {
  }
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    GL(glDisable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glClearColor(0,0,0,0));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));

    const size_t tileCount = 4;
    for (size_t t = 0;t<tileCount;++t) {
      terrain.requestBricks(brickOffset);
      while (!terrain.bricksReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      manager.push(terrain.getBricks());
      brickOffset.z += int32_t(terrain.getSize().y)*terrain.getBrickSize().y;
    }
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width ||
        yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition,
                           double yPosition) override {
    if (state != GLFW_PRESS) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
    } else {
    }
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition,
                          double yPosition) override {
    
    focalDepth += y_offset;
  }
  
  
  virtual void resize(int width, int height) override {
    GL(glViewport(0, 0, GLsizei(width), GLsizei(height)));
    deferredShader.resize(uint32_t(width), uint32_t(height));
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
    
    const Mat4 globalScale = Mat4::scaling(0.005f);
    const Dimensions dim = glEnv.getFramebufferSize();

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();

    projection = Mat4::perspective(fovY, aspect, zNear, zFar);    
    view       = Mat4::lookAt({0,3,-4+this->animationTime/4}, {0,0.1f,this->animationTime/4}, {0,1,0});
    model      = globalScale;
         
    if (terrain.bricksReady()) manager.push(terrain.getBricks());
    
    manager.setProjection(projection);
    manager.setModelView(view*model);

    if (manager.autoPop()) {
      terrain.requestBricks(brickOffset);
      brickOffset.z += int32_t(terrain.getSize().y)*terrain.getBrickSize().y;
    }
  }
  
  virtual void draw() override {
    
    deferredShader.startFirstPass();
    GL(glDisable(GL_BLEND));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    manager.render();
    deferredShader.endFirstPass();

    const Dimensions dim = glEnv.getFramebufferSize();
    deferredShader.startSecondPass(dim.width, dim.height);
    GL(glClearColor(0.2f,0.2f,0.8f,0));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL(glEnable(GL_BLEND));
    deferredShader.getProgram().setUniform("focalDepth", focalDepth);
    
    deferredShader.endSecondPass();

    
  }
  
private:
  Vec2 mousePos;
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  float animationTime;
  
  Mat4 view;
  Mat4 model;
  Mat4 projection;
  
  YAKManager manager;
  
  Vec3i brickOffset;
  YAKTerrain terrain{{120,50}};
  
  DeferredShader deferredShader;
  float focalDepth{6};

};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    YAK42App myApp;
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



/*
 ivec2 kernel[4] = ivec2[4](ivec2(1,0), ivec2(-1,0), ivec2(0,1), ivec2(0,-1));

 
 float depthDist = 0;
 float normDist = 0;
 float count = 0;
 for (int j = 0;j<2;++j) {
   for (int i = 0;i<4;++i) {
     vec4 nPos = texelFetch(offscreenTexture1, ivec2(gl_FragCoord) + kernel[i]*j,0);
     depthDist += abs(pos.z-nPos.z);
     vec3 nNorm = texelFetch(offscreenTexture2, ivec2(gl_FragCoord) + kernel[i]*j,0).rgb;
     normDist += abs(dot(nNorm,normal));
     count++;
   }
 }
 bool border = depthDist/count > 0.005 || normDist/count < 0.9;

 vec3 quantColor = vec3(ivec3(color.rgb*dot(nlightDir,normal)*10)/10.0);

 
 float distToFocus = abs(6+pos.z)/5;


 FragColor = abs(vec4(distToFocus,distToFocus,distToFocus,pos.a));//  border ? vec4(0,0,0,1) : vec4(quantColor, pos.a);
 */
