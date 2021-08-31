#include <array>

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>
#include <Tesselation.h>

const Vec3 brickScale{7.8f,9.6f,7.8f};
const float studHeight{1.8f};
const float studRadius{2.4f};
const float studSpacing = (brickScale.x-studRadius*2.0f) /2.0f;

class YAK42 {
public:
  YAK42(const Vec3ui& pos) :
  pos(pos) {}
    
  virtual ~YAK42() {};
  virtual void render(GLApp& app) = 0;
  virtual std::vector<Vec2t<uint16_t>> studsTop() = 0;
  virtual std::vector<Vec2t<uint16_t>> studsBottom() = 0;

  Vec3ui getPos() const {return pos;}
  
private:
  Vec3ui pos;
  
};

class SimpleYAK42 : public YAK42 {
public:
  SimpleYAK42(uint16_t width, uint16_t depth, uint16_t height,
              uint16_t colorCode, const Vec3ui& pos) :
  YAK42(pos),
  width(width),
  depth(depth),
  height(height),
  colorCode(colorCode)
  {
    generateGeometry();
  }
  
  virtual void render(GLApp& app) override {
    app.drawTriangles(geometry, TrisDrawType::LIST, false, true);
  }
    
  virtual std::vector<Vec2t<uint16_t>> studsTop() override {
    return studsBottom();
  }
  
  virtual std::vector<Vec2t<uint16_t>> studsBottom() override {
    std::vector<Vec2t<uint16_t>> studs(depth*width);
    size_t i = 0;
    for (uint16_t y = 0; y < depth; ++y) {
      for (uint16_t x = 0; x < width; ++x) {
        studs[i].x = x;
        studs[i++].y = y;
      }
    }
    return studs;
  }
  
private:
  uint16_t width;
  uint16_t depth;
  uint16_t height;
  uint16_t colorCode;
  
  std::vector<float> geometry;
  
  void generateGeometry() {
    geometry.clear();
    generateStuds();
    generateBase();
  }

  void generateStuds() {
    Tesselation studTesselation = Tesselation::genCylinder({},
                                                           studRadius,
                                                           studHeight,
                                                           false,
                                                           true,100);

  }
  
  void generateBase() {
    Tesselation baseTesselation = Tesselation::genBrick(Vec3(getPos())*brickScale,
                                                        Vec3(width,height/3,depth)*brickScale);

    
    const std::vector<float> vertices   = baseTesselation.getVertices();
    const std::vector<float> normals    = baseTesselation.getNormals();
    const std::vector<uint32_t> indices = baseTesselation.getIndices();
        
    for (const uint32_t index : indices) {
      geometry.push_back(vertices[index*3+0]);
      geometry.push_back(vertices[index*3+1]);
      geometry.push_back(vertices[index*3+2]);

      // TODO use color form palette
      geometry.push_back(1.0f);
      geometry.push_back(0.0f);
      geometry.push_back(0.0f);
      geometry.push_back(1.0f);

      geometry.push_back(normals[index*3+0]);
      geometry.push_back(normals[index*3+1]);
      geometry.push_back(normals[index*3+2]);
    }
    
  }

  
};





class YAK42App : public GLApp {
public:
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    glEnv.setTitle("Klemmspass");
    GL(glDisable(GL_BLEND));
    GL(glClearColor(0,0,0,0));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_FRONT));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override {
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
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    
    const Dimensions dim = glEnv.getFramebufferSize();
    
    const Mat4 rotationX = Mat4::rotationX(animationTime*42);
    const Mat4 rotationY = Mat4::rotationY(animationTime*100);
    const Mat4 projection{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 1000.0f)};
    const Mat4 view = Mat4::lookAt({0,0,3}, {0,0,0}, {0,1,0});
        
    Mat4 model = rotationX*rotationY*globalScale;
    glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height));
    setDrawProjection(projection);
    setDrawTransform(view*model);
    drawBricks();
  }
  
private:
  Vec2 mousePos;
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  float animationTime;
  
  Mat4 globalScale = Mat4::scaling(0.05f,0.05f,0.05f);
  
  SimpleYAK42 brick{4,2,3,0,{0,0,0}};
  
  void drawBricks() {
    brick.render(*this);
  }
  
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
