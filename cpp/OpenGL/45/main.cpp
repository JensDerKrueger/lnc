#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <sstream>
#include <vector>
#include <array>

#include <GLApp.h>
#include <DeferredShader.h>
#include <Rand.h>

#include "YAK42.h"
#include "YAKManager.h"

#include <Timer.h>


class VoxelGrid {
public:
  VoxelGrid(uint32_t w, uint32_t h, uint32_t d) :
  w{w},
  h{h},
  d{d},
  data(w*h*d)
  {

  }
  
  virtual ~VoxelGrid() {}
  virtual void fill() = 0;

  bool getVoxel(uint32_t x, uint32_t y, uint32_t z) const {
    return data[x+y*w+z*w*h];
  }

  void setVoxel(uint32_t x, uint32_t y, uint32_t z, bool value) {
    data[x+y*w+z*w*h] = value;
  }

  void clear(bool value) {
    std::fill(data.begin(), data.end(), value);
  }
  
  std::string toString() const {
    std::stringstream ss;
        
    size_t i = 0;
    for (uint32_t z = 0;z<d;++z) {
      for (uint32_t y = 0;y<h;++y) {
        for (uint32_t x = 0;x<w;++x) {
          ss << (data[i++] ? "X" : " ");
        }
        ss << "\n";
      }
      ss << "\n";
    }

    return ss.str();
  }
  
  Vec3ui getSize() const {
    return {w,h,d};
  }
  
 private:
  uint32_t w,h,d;
  std::vector<bool> data;
};

class CubicVoxelGrid : public VoxelGrid {
public:
  CubicVoxelGrid(uint32_t s) :
  VoxelGrid(s,s,s) {}
  
};
  
class MengerSponge : public CubicVoxelGrid {
public:
  MengerSponge(uint32_t depth) :
    CubicVoxelGrid(uint32_t(pow3(depth))),
    depth(depth)
  {
  }

  virtual ~MengerSponge() {}
  
  virtual void fill() override {
    construct(0,0,0,depth);
  }
  
private:
  uint32_t depth;
  
  uint32_t pow3(uint32_t exp) const {
    // TODO: improve me
    return uint32_t(pow(3,exp));
  }
 
  const std::array< Vec3ui ,20> offsets = {
    Vec3ui{0,0,0},Vec3ui{1,0,0},Vec3ui{2,0,0},
    Vec3ui{0,1,0},              Vec3ui{2,1,0},
    Vec3ui{0,2,0},Vec3ui{1,2,0},Vec3ui{2,2,0},

    Vec3ui{0,0,1},              Vec3ui{2,0,1},

    Vec3ui{0,2,1},              Vec3ui{2,2,1},

    Vec3ui{0,0,2},Vec3ui{1,0,2},Vec3ui{2,0,2},
    Vec3ui{0,1,2},              Vec3ui{2,1,2},
    Vec3ui{0,2,2},Vec3ui{1,2,2},Vec3ui{2,2,2}
  };

  void construct(uint32_t x, uint32_t y,uint32_t z, uint32_t depth) {
    if (depth == 0) {
      setVoxel(x,y,z,true);
    } else {
      for (const Vec3ui& p : offsets) {
        construct(x*3+p.x, y*3+p.y, z*3+p.z, depth-1);
      }
    }
  }
  
};


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


    vec3 nlightDir = normalize(vec3(0,0,1));

    FragColor = vec4(color.rgb*clamp(dot(nlightDir,normal),0,1), pos.a);
})"};

class YAK42App : public GLApp {
public:
  
  YAK42App(uint32_t spongeDepth) :
    GLApp(800,600,1,"YAK Fractal",true,false),
    deferredShader(dsFragmentShaderString, {
      {3,GLDataType::BYTE,true},
      {4,GLDataType::HALF,false},
      {3,GLDataType::HALF,true}
    }),
    sponge{spongeDepth},
    spongeDepth{spongeDepth}
  {
  }
  
  virtual void init() override {
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

    sponge.fill();
    
    const Vec3ui spongeSize = sponge.getSize();
    
    StaticYAKCuller culler;
    
    for (uint32_t z = 0;z<spongeSize.z;++z) {
      for (uint32_t y = 0;y<spongeSize.y;++y) {
        for (uint32_t x = 0;x<spongeSize.x;++x) {
          if (sponge.getVoxel(x, y, z)) {
            
            const int32_t sx = int32_t(x) - int32_t(spongeSize.x/2);
            const int32_t sy = int32_t(y) - int32_t(spongeSize.y/2);
            const int32_t sz = int32_t(z) - int32_t(spongeSize.z/2);
            
            auto brick = std::make_shared<SimpleYAK42>(1,
                                                       1,
                                                       3,
                                                       1,
                                                       Vec3i{sx,sy*3,sz});
            
            ManagedYAK managedBrick(brick);
            managedBrick.hideAllStuds();
            culler.add(managedBrick);
          }
        }
      }
    }
    culler.cull();
    manager.push(culler.get());
    
  }
  
    
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition,
                          double yPosition) override {
    focalDepth += float(y_offset);
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
    
    const Mat4 globalScale = Mat4::scaling(0.002f/spongeDepth);
    const Dimensions dim = glEnv.getFramebufferSize();

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();

    projection = Mat4::perspective(fovY, aspect, zNear, zFar);    
    view       = Mat4::lookAt({0,0,2},
                              {0,0,0},
                              {0,1,0});
    Mat4 rot   = Mat4::rotationY(float(animationTime)*50.0f);
    model      = globalScale*rot;
    
    manager.setProjection(projection);
    manager.setModelView(view*model);
  }
  
  virtual void draw() override {
    deferredShader.startFirstPass();
    GL(glDisable(GL_BLEND));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    manager.render();
    deferredShader.endFirstPass();

    const Dimensions dim = glEnv.getFramebufferSize();
    deferredShader.startSecondPass(dim.width, dim.height);
    GL(glClearColor(0.2f,0.2f,0.2f,0));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL(glEnable(GL_BLEND));
    deferredShader.getProgram().setUniform("focalDepth", focalDepth);
    
    deferredShader.endSecondPass();
  }
  
private:
  float animationTime;
  
  Mat4 view;
  Mat4 model;
  Mat4 projection;
  
  YAKManager manager;
  
  Vec3i brickOffset;
  
  
  DeferredShader deferredShader;
  float focalDepth{3.6f};

  MengerSponge sponge;
  uint32_t spongeDepth;

};


#ifdef _WIN32
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    YAK42App myApp{5};
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
