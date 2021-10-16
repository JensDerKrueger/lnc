#include <GLApp.h>
#include <GLFramebuffer.h>
#include <MeshRenderer.h>

static const std::string sceneVertexShader {R"(#version 410
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 worldToShadow;
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
out vec3 normal;
out vec3 pos;
out vec4 shadowPos;
void main() {
    mat4 MV   = V*M;
    mat4 MVP  = P*MV;
    mat4 MVit = transpose(inverse(MV));
    gl_Position = MVP * vec4(vPos, 1.0);
    shadowPos = worldToShadow * M * vec4(vPos, 1.0);
    pos = (MV * vec4(vPos, 1.0)).xyz;
    normal = (MVit * vec4(vNormal, 0.0)).xyz;
})"};

static const std::string sceneFragmentShader {R"(#version 410
uniform vec3 color;
uniform float ambient=0.2;
uniform vec3 lightPosViewSpace;
uniform sampler2DShadow shadowMap;
uniform float depthBias = 0.01;
in vec3 pos;
in vec3 normal;
in vec4 shadowPos;
out vec4 FragColor;
void main() {
    vec3 nnormal = normalize(normal);
    vec3 nlightDir = normalize(lightPosViewSpace-pos);
    float light = clamp(clamp(dot(nlightDir,normal),0.0,1.0)+ambient,0.0,1.0);

    vec4 biasedShadow = shadowPos;
    biasedShadow.z -= depthBias;
    float shadowPercentage = textureProj(shadowMap,biasedShadow);

    vec4 lightColor = vec4(clamp(color.rgb*ambient,0.0,1.0), 1);
    vec4 shadowColor = vec4(color.rgb*light, 1);

    FragColor = mix(lightColor, shadowColor, shadowPercentage);
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

static const std::string shadowVertexShader {R"(#version 410
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
layout (location = 0) in vec3 vPos;
void main() {
    gl_Position = P*V*M * vec4(vPos, 1.0);
})"};

static const std::string shadowFragmentShader {R"(#version 410
void main() {
})"};

class ShadowMappingDemo : public GLApp {
public:
  ShadowMappingDemo() :
    GLApp(640,480,1,"Shadow Mapping Demo",true,false),
    sceneProgram{GLProgram::createFromString(sceneVertexShader,
                                             sceneFragmentShader)},
    lightProbeProgram{GLProgram::createFromString(lightProbeVertexShader,
                                                  lightProbeFragmentShader)},
    shadowProgram{GLProgram::createFromString(shadowVertexShader,
                                              shadowFragmentShader)}
  {
    shadowMap.setEmpty(1024,1024);
  }
  
  virtual void init() override {
    GL(glDisable(GL_BLEND));
    GL(glClearDepth(1.0f));
    GL(glClearColor(0,0,1,0));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));    
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));

    torus.init(Tesselation::genTorus({0,0,0}, 0.8f, 0.3f),true);
    torus.connect(sceneProgram);

    plane.init(Tesselation::genRectangle({0,0,-2}, 6, 6),true);
    plane.connect(sceneProgram);

    light.init(Tesselation::genSphere({0,0,0}, 0.1f, 10, 10));
    light.connect(lightProbeProgram);

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();
    projection = Mat4::perspective(fovY, aspect, zNear, zFar);
    view       = Mat4::lookAt({0,8,8}, {0,0,0}, {0,1,0});
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
  
  virtual void animate(double dAnimationTime) override {
    const float animationTime = float(dAnimationTime);
        
    torusModel = Mat4::rotationY(animationTime*80) *
                 Mat4::rotationX(animationTime*70);
    planeModel = Mat4::rotationX(90);

    torusColor = Vec3{
      fabs(sinf(animationTime*0.3f)),
      fabs(sinf(animationTime*0.8f)),
      fabs(sinf(animationTime*0.1f))
    };
    
    const Vec3 lightPos = Vec3{1,4,0};
    lightModel = Mat4::rotationY(animationTime*50) *
                 Mat4::translation(lightPos);
    
    lightProjection = Mat4::perspective(70.0f,
                                        float(shadowMap.getWidth())/
                                        float(shadowMap.getHeight()),
                                        1, 10);
    lightView = Mat4::lookAt(lightModel * Vec3{0,0,0}, {0,0,0}, {0,0,1});
  }
  
  void renderScene(const GLProgram& program, const Mat4& viewMatrix,
                   const Mat4& projectionMatrix, bool setLightParameter) {
    program.enable();
    if (setLightParameter)
      program.setUniform("lightPosViewSpace", view*lightModel*Vec3{0,0,0});
    
    program.setUniform("M", torusModel);
    program.setUniform("V", viewMatrix);
    program.setUniform("P", projectionMatrix);
    if (setLightParameter)
      program.setUniform("color", torusColor);
    torus.render();

    program.setUniform("M", planeModel);
    if (setLightParameter)
      program.setUniform("color", Vec3{0.5,0.5,0.5});
    plane.render();
  }
  
  virtual void draw() override {
    framebuffer.bind(shadowMap);
    GL(glViewport(0, 0, GLsizei(shadowMap.getWidth() ), GLsizei(shadowMap.getHeight())));
    GL(glClear(GL_DEPTH_BUFFER_BIT));
    renderScene(shadowProgram, lightView, lightProjection, false);
    framebuffer.unbind2D();
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
    sceneProgram.enable();
    sceneProgram.setUniform("worldToShadow", cliptToTextureMatrix*lightProjection*lightView);
    sceneProgram.setTexture("shadowMap", shadowMap);
    renderScene(sceneProgram, view, projection, true);

    lightProbeProgram.enable();
    lightProbeProgram.setUniform("MVP",  projection*view*lightModel);
    light.render();
  }
  
private:
  Mat4 view;
  Mat4 projection;
  
  GLProgram    sceneProgram;
  GLProgram    lightProbeProgram;

  Vec3         torusColor;
  Mat4         torusModel;
  MeshRenderer torus;

  Mat4         planeModel;
  MeshRenderer plane;

  Mat4         lightView;
  Mat4         lightProjection;
  Mat4         lightModel;
  MeshRenderer light;

  GLProgram shadowProgram;
  GLFramebuffer framebuffer;
  GLDepthTexture shadowMap;
  const Mat4 cliptToTextureMatrix {
    0.5f, 0.0f, 0.0f, 0.5f,
    0.0f, 0.5f, 0.0f, 0.5f,
    0.0f, 0.0f, 0.5f, 0.5f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

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
