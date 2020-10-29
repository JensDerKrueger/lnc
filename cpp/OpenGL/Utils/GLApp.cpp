#include <GLApp.h>


GLApp* GLApp::staticAppPtr = nullptr;

GLApp::GLApp(uint32_t w, uint32_t h, uint32_t s,
             const std::string& title,
             bool fpsCounter, bool sync, int major,
             int minor, bool core) :
  glEnv{w,h,s,title,fpsCounter,sync,major,minor,core},
  simpleProg{GLProgram::createFromString(
     "#version 410\n"
     "uniform mat4 MVP;\n"
     "layout (location = 0) in vec3 vPos;\n"
     "layout (location = 1) in vec4 vColor;\n"
     "out vec4 color;"
     "void main() {\n"
     "    gl_Position = MVP * vec4(vPos, 1.0);\n"
     "    color = vColor;\n"
     "}\n",
     "#version 410\n"
     "in vec4 color;"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    FragColor = color;\n"
     "}\n")},
  simpleArray{},
  simpleVb{GL_ARRAY_BUFFER},
  animationActive{true}
{
  staticAppPtr = this;
  glEnv.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  glEnv.setKeyCallback(keyCallback);
  glEnv.setResizeCallback(sizeCallback);
  
  // setup a minimal shader and buffer
  Mat4 mvp{};
  simpleProg.enable();
  simpleProg.setUniform("MVP", mvp);
  simpleArray.bind();

  std::vector<float> empty;
  simpleVb.setData(empty,7,GL_DYNAMIC_DRAW);

  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

  glfwSetTime(0);
  Dimensions dim{ glEnv.getFramebufferSize() };
  glViewport(0, 0, dim.width, dim.height);
}

void GLApp::run() {
  init();
  Dimensions dim{ glEnv.getFramebufferSize() };
  resize(dim.width, dim.height);
  do {
    if (animationActive) animate();
    draw();
    glEnv.endOfFrame();
  } while (!glEnv.shouldClose());
}
 
void GLApp::resize(int width, int height) {
  Dimensions dim{ glEnv.getFramebufferSize() };
  GL(glViewport(0, 0, dim.width, dim.height));
}

void GLApp::drawLines(const std::vector<float> data, LineDrawType t) {
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);

  switch (t) {
    case LineDrawType::LIST :
      GL(glDrawArrays(GL_LINES, 0, GLsizei(data.size()/7)));
      break;
    case LineDrawType::STRIP :
      GL(glDrawArrays(GL_LINE_STRIP, 0, GLsizei(data.size()/7)));
      break;
    case LineDrawType::LOOP :
      GL(glDrawArrays(GL_LINE_LOOP, 0, GLsizei(data.size()/7)));
      break;
  }
}

void GLApp::drawPoints(const std::vector<float> data, float pointSize) {
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
  GL(glPointSize(pointSize));
  GL(glDrawArrays(GL_POINTS, 0, GLsizei(data.size()/7)));
}
