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

class mengerPredicate {
private:
  const size_t mengerDepth;
  std::vector<std::vector<uint8_t>> base3Digits;

public:
  mengerPredicate(const size_t mengerDepth) : mengerDepth(mengerDepth) {
    // precalculate the base 3 representation of all indices in the sponge
    // store only digit 1, so we can sum it up
    const size_t mengerSize = std::pow(3, mengerDepth);
    base3Digits.resize(mengerSize);
    for (size_t number = 0; number < mengerSize; ++number) {
      base3Digits[number].resize(mengerDepth);

      auto remains = number;
      for (size_t digit = 0; digit < mengerDepth; ++digit) {
        const auto divstep = std::div(remains, 3);
        if (divstep.rem == 1) {
          base3Digits[number][digit] = 1;
        }
        remains = divstep.quot;
      }
    }
  }

  bool contains(const size_t x, const size_t y, const size_t z) const {
    for (size_t digit = 0; digit < mengerDepth; ++digit) {
      // at every level there must be at most one "1" digit
      const uint8_t countDigitOne =
          base3Digits[x][digit] + base3Digits[y][digit] + base3Digits[z][digit];
      if (countDigitOne > 1) {
        return false;
      }
    }
    return true;
  }
};


class VoxelGrid {
public:
  VoxelGrid(size_t w, size_t h, size_t d) :
  w{w},
  h{h},
  d{d},
  data(w*h*d)
  {

  }
  
  virtual ~VoxelGrid() {}
  virtual void fill(uint8_t method) = 0;

  bool getVoxel(size_t x, size_t y, size_t z) const {
    return data[x+y*w+z*w*h];
  }

  void setVoxel(size_t x, size_t y, size_t z, bool value) {
    if (x < w && y < d && z < h)
      data[x+y*w+z*w*h] = value;
    else {
      std::cerr << "setVoxel index (" << x << ", " << y << ", " << z
                << ") out of range (" << w-1 << ", " << d-1 << ", " << h-1 << ")\n";
      exit(-1);
    }
  }

  void clear(bool value) {
    std::fill(data.begin(), data.end(), value);
  }
  
  std::string toString() const {
    std::stringstream ss;
    
    
    size_t i = 0;
    for (size_t z = 0;z<d;++z) {
      for (size_t y = 0;y<h;++y) {
        for (size_t x = 0;x<w;++x) {
          ss << (data[i++] ? "X" : " ");
        }
        ss << "\n";
      }
      ss << "\n";
    }

    return ss.str();
  }
    
  
 private:
  size_t w,h,d;
  std::vector<bool> data;
};

class CubicVoxelGrid : public VoxelGrid {
public:
  CubicVoxelGrid(size_t s) :
  VoxelGrid(s,s,s) {}
  
};
  
class MengerSponge : public CubicVoxelGrid {
public:
  MengerSponge(size_t depth) :
    CubicVoxelGrid(size_t(pow3(depth))),
    depth(depth)
  {
    
  }

  virtual ~MengerSponge() {}
  
  
  virtual void fill(uint8_t method) override {

    switch (method) {
      case 0 :
        construct(0,0,0,depth);
        break;
      case 1 :
        {
          clear(true);
          for (size_t currentDepth = 1;currentDepth<depth+1;++currentDepth) {
            for (size_t z = 0;z<pow3(currentDepth-1);++z) {
              for (size_t y = 0;y<pow3(currentDepth-1);++y) {
                for (size_t x = 0;x<pow3(currentDepth-1);++x) {
                  mengerIterration(x,y,z,pow3(depth-currentDepth));
                }
              }
            }
          }
        }
        break;
      case 2 :
        {
          const size_t mengerSize = std::pow(3, depth);
          mengerPredicate predicate(depth);

          for (size_t x = 0; x < mengerSize; ++x) {
            for (size_t y = 0; y < mengerSize; ++y) {
              for (size_t z = 0; z < mengerSize; ++z) {
                setVoxel(x, y, z, predicate.contains(x, y, z));
              }
            }
          }
        }
        break;
    }
    
  }
  
private:
  size_t depth;
  
  size_t pow3(size_t exp) const {
    // TODO: improve me
    return size_t(pow(3,exp));
  }
  
  const std::array< Vec3ui ,7> negativeOffsets = {
    Vec3ui{1,1,0},Vec3ui{1,0,1},
    Vec3ui{0,1,1},Vec3ui{1,1,1},Vec3ui{2,1,1},
    Vec3ui{1,2,1},Vec3ui{1,1,2}
  };
  
  void mengerIterration(size_t x, size_t y,size_t z, size_t delta) {
    for (const Vec3ui& p : negativeOffsets) {
      removeCube(x*3+p.x, y*3+p.y, z*3+p.z, delta);
    }
  }
  
  void removeCube(size_t x, size_t y,size_t z, size_t delta) {
    for (size_t dz = 0;dz<delta;++dz) {
      for (size_t dy = 0;dy<delta;++dy) {
        for (size_t dx = 0;dx<delta;++dx) {
          setVoxel(x*delta+dx, y*delta+dy, z*delta+dz, false);
        }
      }
    }
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

  void construct(size_t x, size_t y,size_t z, size_t depth) {
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
    vec3 nlightDir = normalize(vec3(0,1,4));

    FragColor = vec4(color.rgb*clamp(dot(nlightDir,normal)+0.3,0,1), pos.a);
})"};

class YAK42App : public GLApp {
public:
  
  YAK42App() :
    GLApp(640,480,1,"YAK Fractal",true,false),
    deferredShader(dsFragmentShaderString, {
      {3,GLDataType::BYTE,true},
      {4,GLDataType::HALF,false},
      {3,GLDataType::HALF,true}
    }),
    sponge{6}
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

    Timer t;
    sponge.fill(0);
    std::cout << t.stop() << std::endl;

    t.start();
    sponge.fill(1);
    std::cout << t.stop() << std::endl;

    t.start();
    sponge.fill(2);
    std::cout << t.stop() << std::endl;
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
    
    const Mat4 globalScale = Mat4::scaling(0.005f);
    const Dimensions dim = glEnv.getFramebufferSize();

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();

    projection = Mat4::perspective(fovY, aspect, zNear, zFar);    
    view       = Mat4::lookAt({0,2,2},
                              {0,0,0},
                              {0,1,0});
    Mat4 rot   = Mat4::rotationY(float(animationTime)*50.0f);
    model      = globalScale*rot;
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
  float animationTime;
  
  Mat4 view;
  Mat4 model;
  Mat4 projection;
  
  YAKManager manager;
  
  Vec3i brickOffset;
  
  
  DeferredShader deferredShader;
  float focalDepth{3.6f};

  MengerSponge sponge;

};


#ifdef _WIN32
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
