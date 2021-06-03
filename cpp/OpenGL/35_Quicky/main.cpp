#include <array>

#include "Quicky.h"

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>


class MyGLApp : public GLApp {
public:
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    glEnv.setCursorMode(CursorMode::HIDDEN);
    glEnv.setTitle("Quicky Foto Editor");
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
      std::array<Vec2,2> cropBox = computeCropBox();
      quicky.crop(cropBox);
    }
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override {
    cropScale = std::max<float>(cropScale + float(y_offset), 1.0f);
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
        case GLFW_KEY_Q :
          quicky.rotate90();
          break;
        case GLFW_KEY_W :
          quicky.rotate180();
          break;
        case GLFW_KEY_E :
          quicky.rotate270();
          break;
        case GLFW_KEY_X :
          if (quicky.getEdited()) quicky.save();
          quicky.next();
          cropScale = 1.0f;
          break;
        case GLFW_KEY_Z :
          if (quicky.getEdited()) quicky.save();
          quicky.previous();
          cropScale = 1.0f;
          break;
        case GLFW_KEY_C :
          quicky.reset();
          break;
        case GLFW_KEY_P :
          quicky.erase();
          break;
        case GLFW_KEY_SPACE: {
          if (quicky.getEdited()) quicky.save();
          float diff;
          do {
            quicky.next();
            float imageAspect = float(quicky.getCurrentImage().width) /
                                float(quicky.getCurrentImage().height);

            const Vec2ui targetSize = quicky.getTargetImageSize();
            const float aspect = float(targetSize.x) / float(targetSize.y);
            diff = fabs(imageAspect-aspect);
                    
          } while (diff < 0.01f && quicky.getCurrentImageIndex() < quicky.getImageCount()-1);
        }
      }
    }
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    float imageAspect = float(quicky.getCurrentImage().width) /
                        float(quicky.getCurrentImage().height);
    float scale = imageAspect/glEnv.getFramebufferSize().aspect();
    if (scale < 1)
      setDrawTransform(Mat4::scaling(scale, 1, 1));
    else
      setDrawTransform(Mat4::scaling(1, 1/scale, 1));

    drawImage(quicky.getCurrentImage());
    drawMouseCursor();
    drawBBox();
    drawInfoText();
  }
  
private:
  Vec2 mousePos;
  Quicky quicky{"/Users/krueger/twitch/lnc/cpp/OpenGL/34_Mosaic/smallImages",{100,100},0};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  float cropScale{1.0f};
  
  std::array<Vec2,2> computeCropBox() const {
    float imageAspect = float(quicky.getCurrentImage().width) /
                        float(quicky.getCurrentImage().height);

    const Vec2ui targetSize = quicky.getTargetImageSize();
    const float aspect = float(targetSize.x) / float(targetSize.y)/imageAspect;

    const Vec2 a = Vec2(1-mousePos.x, 1-mousePos.y) / Vec2(aspect,1);
    const Vec2 b = Vec2(-mousePos.x, -mousePos.y) / Vec2(-aspect,-1);
    const float s = std::min(std::min(a.x, a.y), std::min(b.x, b.y));

    std::array<Vec2,2> cropBox;
    cropBox[0] = (mousePos + Vec2(-aspect,-1)*s/cropScale);
    cropBox[1] = (mousePos + Vec2( aspect, 1)*s/cropScale);
    return cropBox;
  }
  
  void drawBBox() {
    float imageAspect = float(quicky.getCurrentImage().width) /
                        float(quicky.getCurrentImage().height);

    const Vec2ui targetSize = quicky.getTargetImageSize();
    const float aspect = float(targetSize.x) / float(targetSize.y);

    const Vec4 color = (fabs(imageAspect-aspect) < 0.01f) ? Vec4{1,0,0,1} : Vec4{1,1,0,1};
      
    std::array<Vec2,2> cropBox = computeCropBox();
    
    std::array<Vec2,4> points;
    points[0] = cropBox[0]*2.0f-1.0f;
    points[1] = Vec2(cropBox[0].x, cropBox[1].y)*2.0f-1.0f;
    points[2] = cropBox[1]*2.0f-1.0f;
    points[3] = Vec2(cropBox[1].x, cropBox[0].y)*2.0f-1.0f;
    
    std::vector<float> glShape;
    for (size_t i = 0;i<4;++i) {
      glShape.push_back(points[i].x); glShape.push_back(points[i].y); glShape.push_back(0.0f);
      glShape.push_back(color.r); glShape.push_back(color.g); glShape.push_back(color.b);  glShape.push_back(color.a);
      glShape.push_back(points[(i+1)%4].x); glShape.push_back(points[(i+1)%4].y); glShape.push_back(0.0f);
      glShape.push_back(color.r); glShape.push_back(color.g); glShape.push_back(color.b);  glShape.push_back(color.a);
    }
    drawLines(glShape, LineDrawType::LIST, 2);

  }
  
  void drawMouseCursor() {
    std::vector<float> glShape;
    glShape.push_back(-1.0f); glShape.push_back(mousePos.y*2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(0.0f);  glShape.push_back(0.4f);
    glShape.push_back(1.0f); glShape.push_back(mousePos.y*2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(0.0f);  glShape.push_back(0.4f);
    glShape.push_back(mousePos.x*2.0f-1.0f); glShape.push_back(-1.0); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(0.0f);  glShape.push_back(0.4f);
    glShape.push_back(mousePos.x*2.0f-1.0f); glShape.push_back(1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(0.0f);  glShape.push_back(0.4f);
    drawLines(glShape, LineDrawType::LIST,2);
    
    glShape.clear();
    glShape.push_back(mousePos.x*2.0f-1.0f); glShape.push_back(mousePos.y*2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f);
    drawPoints(glShape, 30, true);
  }

  void drawInfoText() {
    std::stringstream ss;
    ss << quicky.getCurrentImageIndex()+1 << "/" << quicky.getImageCount();
    fe->render(ss.str(), glEnv.getFramebufferSize().aspect(), 0.05f, {0,-0.95f});
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
