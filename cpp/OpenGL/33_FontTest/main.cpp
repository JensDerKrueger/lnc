#include <GLApp.h>
#include <FontRenderer.h>

class MyGLApp : public GLApp {
public:
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  
  virtual void init() override {
    glEnv.setTitle("Font Test");
    fe = fr.generateFontEngine();
  }
      
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_S :
          fe->setRenderAsSignedDistanceField(!fe->getRenderAsSignedDistanceField());
          break;
      }
    }
  }

  virtual void draw() override {
    Dimensions dim{ glEnv.getFramebufferSize() };
    fe->render("Hallo", dim.aspect(), 0.5f, {0,0});
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
