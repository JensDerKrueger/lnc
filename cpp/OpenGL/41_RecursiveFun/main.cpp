#include <array>

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>


class MyGLApp : public GLApp {
public:
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    glEnv.setTitle("Recursive Fun");
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glEnable(GL_BLEND));
    GL(glClearColor(0,0,0,0));
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
    GL(glClear(GL_COLOR_BUFFER_BIT));
    float scale = 1/glEnv.getFramebufferSize().aspect();
    if (scale < 1)
      setDrawTransform(Mat4::scaling(scale, 1, 1));
    else
      setDrawTransform(Mat4::scaling(1, 1/scale, 1));

    drawFunyRecursiveStuff();
    drawInfoText();
  }
  
private:
  Vec2 mousePos;
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  std::vector<float> glShape;
  float animationTime;
  

  
  void drawFunyRecursiveStuff() {
    glShape.clear();
    rec(0);
    drawLines(glShape, LineDrawType::LIST, 3);
  }
  
  
  void addLine(const Vec2& from, const Vec2& to, const Vec4& color={1,1,1,1}) {
    glShape.push_back(from.x);
    glShape.push_back(from.y);
    glShape.push_back(0.0f);
    glShape.push_back(color.r);
    glShape.push_back(color.g);
    glShape.push_back(color.b);
    glShape.push_back(color.a);
    glShape.push_back(to.x);
    glShape.push_back(to.y);
    glShape.push_back(0.0f);
    glShape.push_back(color.r);
    glShape.push_back(color.g);
    glShape.push_back(color.b);
    glShape.push_back(color.a);
  }
  
  void rec(uint32_t n, const Vec2& pos = {0,-0.5},
           const Mat4& transformation={}) {
    const Mat4 aniRot = Mat4::rotationZ(std::tgammaf(cosf(animationTime/100.0f))*4);
    const Vec2& pos2 = transformation  * (pos + Vec2{0.0f,0.05f});
    addLine(pos, pos2, {n/10.0f,1.0f-n/10.0f,sinf(animationTime/1000),1.0f/n});
    if (n < 12) {
      rec(n+1, pos2, Mat4::rotationZ(cosf(animationTime/3)*4)*aniRot * transformation);
      rec(n+1, pos2, Mat4::rotationZ(-cosf(animationTime/3)*4)*aniRot * transformation);
    }
  }
    
  void drawInfoText() {
    std::stringstream ss;
    ss << "Relax with Recursion";
    fe->render(ss.str(), glEnv.getFramebufferSize().aspect(), 0.1f, {0,0.7f}, Alignment::Center, {1.0f,1.0f,1.0f,1.0f/animationTime});
  }
  
};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    MyGLApp myApp;
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
