#include <array>

#include <GLApp.h>
#include <bmp.h>
#include <Tesselation.h>

static const std::string vertexShaderString {
  "#version 410\n"
  "uniform mat4 MVP;\n"
  "uniform mat4 MV;\n"
  "uniform mat4 MVit;\n"
  "layout (location = 0) in vec3 vPos;\n"
  "layout (location = 1) in vec3 vNormal;\n"
  "layout (location = 2) in vec2 vTexCoords;\n"
  "out vec3 normal;\n"
  "out vec3 pos;\n"
  "out vec2 texCoords;\n"
  "void main() {\n"
  "    gl_Position = MVP * vec4(vPos, 1.0);\n"
  "    pos = (MV * vec4(vPos, 1.0)).xyz;\n"
  "    texCoords = vTexCoords;\n"
  "    normal = (MVit * vec4(vNormal, 0.0)).xyz;\n"
  "}\n"
};

static const std::string fragmentShaderString {
  "#version 410\n"
  "uniform sampler2D torusTexture;"
  "uniform vec4 color;\n"
  "uniform vec3 lightPosViewSpace;\n"
  "in vec3 pos;\n"
  "in vec3 normal;\n"
  "in vec2 texCoords;\n"
  "out vec4 FragColor;\n"
  "void main() {\n"
  "    vec3 nnormal = normalize(normal);\n"
  "    vec3 nlightDir = normalize(lightPosViewSpace-pos);\n"
  "    vec4 texValue = texture(torusTexture, texCoords);\n"
  "    FragColor = color*texValue*dot(nlightDir,nnormal);\n"
  "}\n"
};

struct TorusParams {
  Mat4 model;
  Vec4 color;
};

class DeferredShadingApp : public GLApp {
public:
  
  DeferredShadingApp() :
    GLApp(640,480,1,"Deferred Shading Demo",true,false),
    torusShader{GLProgram::createFromString(vertexShaderString, fragmentShaderString)},
    torusPosBuffer{GL_ARRAY_BUFFER},
    torusNormalBuffer{GL_ARRAY_BUFFER},
    torusTexBuffer{GL_ARRAY_BUFFER},
    torusIndexBuffer{GL_ELEMENT_ARRAY_BUFFER},
    torusTexture{GL_LINEAR, GL_LINEAR}
  {
  }
  
  virtual void init() override {
    GL(glDisable(GL_BLEND));
    GL(glClearColor(0.2f,0.2f,0.8f,0));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));
    
    const Tesselation torus = Tesselation::genTorus({0,0,0}, 0.8f, 0.3f);
    
    torusPosBuffer.setData(torus.getVertices(), 3);
    torusNormalBuffer.setData(torus.getNormals(), 3);
    torusTexBuffer.setData(torus.getTexCoords(),2);
    torusIndexBuffer.setData(torus.getIndices());

    torusArray.bind();
    torusArray.connectVertexAttrib(torusPosBuffer, torusShader, "vPos",3);
    torusArray.connectVertexAttrib(torusNormalBuffer, torusShader, "vNormal",3);
    torusArray.connectVertexAttrib(torusTexBuffer, torusShader, "vTexCoords",3);
    torusArray.connectIndexBuffer(torusIndexBuffer);
    
    vertexCount = GLsizei(torus.getIndices().size());
    
    Image torusImage{BMP::load("Torus.bmp")};
    torusTexture.setData(torusImage.data, torusImage.width, torusImage.height, torusImage.componentCount);
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
    view       = Mat4::lookAt({0,0,-8}, {0,0,0}, {0,1,0});
        
    torusParams[0].model = Mat4::rotationY(this->animationTime*80) *
                           Mat4::rotationX(this->animationTime*70);
    torusParams[0].color = Vec4{
      fabs(sinf(this->animationTime*0.3f)),
      fabs(sinf(this->animationTime*0.8f)),
      fabs(sinf(this->animationTime*0.1f)),
      1.0f
    };
    
    torusParams[1].model = torusParams[0].model *
                           Mat4::rotationZ(this->animationTime*100) *
                           Mat4::translation({2,0,0}) * Mat4::scaling(0.2f);
    torusParams[1].color = Vec4{
      fabs(sinf(this->animationTime*0.4f)),
      fabs(sinf(this->animationTime*0.1f)),
      fabs(sinf(this->animationTime*0.9f)),
      1.0f
    };

    torusParams[2].model = torusParams[0].model *
                           Mat4::rotationZ(this->animationTime*100) *
                           Mat4::translation({-2,0,0}) * Mat4::scaling(0.2f);
    torusParams[2].color = Vec4{
      fabs(sinf(this->animationTime*0.1f)),
      fabs(sinf(this->animationTime*0.7f)),
      fabs(sinf(this->animationTime*0.4f)),
      1.0f
    };

    torusParams[3].model = torusParams[0].model *
                           Mat4::rotationZ(this->animationTime*100) *
                           Mat4::translation({0,2,0}) * Mat4::scaling(0.2f);
    torusParams[3].color = Vec4{
      fabs(sinf(this->animationTime*0.8f)),
      fabs(sinf(this->animationTime*0.3f)),
      fabs(sinf(this->animationTime*0.1f)),
      1.0f
    };

    torusParams[4].model = torusParams[0].model *
                           Mat4::rotationZ(this->animationTime*100) *
                           Mat4::translation({0,-2,0}) * Mat4::scaling(0.2f);
    torusParams[4].color = Vec4{
      fabs(sinf(this->animationTime*0.7f)),
      fabs(sinf(this->animationTime*0.8f)),
      fabs(sinf(this->animationTime*0.9f)),
      1.0f
    };
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    torusArray.bind();
    torusShader.enable();
    const Vec3 lightPosViewSpace = view * Vec3{0,3,-4};
    torusShader.setUniform("lightPosViewSpace", lightPosViewSpace);
    torusShader.setTexture("torusTexture",torusTexture,1);

    for (const auto& param : torusParams) {
      const Mat4 modelView = view*param.model;
      const Mat4 modelViewProjection = projection*modelView;
      const Mat4 modelViewInverseTranspose = Mat4::transpose(Mat4::inverse(modelView));
              
      torusShader.setUniform("MVP",  modelViewProjection);
      torusShader.setUniform("MV",   modelView);
      torusShader.setUniform("MVit", modelViewInverseTranspose);
      torusShader.setUniform("color", param.color);
      GL(glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void*)0));
    }

  }
  
private:
  Vec2 mousePos;
  float animationTime;
  Mat4 view;
  Mat4 projection;

  std::array<TorusParams,5> torusParams;

  GLProgram   torusShader;
  GLArray     torusArray;
  GLBuffer    torusPosBuffer;
  GLBuffer    torusNormalBuffer;
  GLBuffer    torusTexBuffer;
  GLBuffer    torusIndexBuffer;
  GLsizei     vertexCount;
  GLTexture2D torusTexture;

};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    DeferredShadingApp myApp;
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
