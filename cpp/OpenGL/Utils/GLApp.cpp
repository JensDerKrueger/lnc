#include "GLApp.h"


GLApp* GLApp::staticAppPtr = nullptr;

GLApp::GLApp(uint32_t w, uint32_t h, uint32_t s,
             const std::string& title,
             bool fpsCounter, bool sync) :
  glEnv{w,h,s,title,fpsCounter,sync,4,1,true},
  p{},
  mv{},
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
  simpleLightProg{GLProgram::createFromString(
     "#version 410\n"
     "uniform mat4 MVP;\n"
     "uniform mat4 MV;\n"
     "uniform mat4 MVit;\n"
     "layout (location = 0) in vec3 vPos;\n"
     "layout (location = 1) in vec4 vColor;\n"
     "layout (location = 2) in vec3 vNormal;\n"
     "out vec4 color;"
     "out vec3 normal;"
     "out vec3 pos;"
     "void main() {\n"
     "    gl_Position = MVP * vec4(vPos, 1.0);\n"
     "    pos = (MV * vec4(vPos, 1.0)).xyz;\n"
     "    color = vColor;\n"
     "    normal = (MVit * vec4(vNormal, 0.0)).xyz;\n"
     "}\n",
     "#version 410\n"
     "in vec4 color;"
     "in vec3 pos;"
     "in vec3 normal;"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    vec3 nnormal = normalize(normal);"
     "    vec3 nlightDir = normalize(vec3(0.0,0.0,0.0)-pos);"
     "    FragColor = color*abs(dot(nlightDir,nnormal));\n"
     "}\n")},
  simpleArray{},
  simpleVb{GL_ARRAY_BUFFER},
  resumeTime{0},
  animationActive{true}
{
  staticAppPtr = this;
  glEnv.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  glEnv.setKeyCallback(keyCallback);
  glEnv.setResizeCallback(sizeCallback);
  
  // setup a minimal shader and buffer
  shaderUpdate();
  
  glfwSetTime(0);
  Dimensions dim{ glEnv.getFramebufferSize() };
  glViewport(0, 0, dim.width, dim.height);
}

void GLApp::run() {
  init();
  Dimensions dim{ glEnv.getFramebufferSize() };
  resize(dim.width, dim.height);
  do {
    if (animationActive) {
      animate(glfwGetTime());
    }
    draw();
    glEnv.endOfFrame();
  } while (!glEnv.shouldClose());
}
 
void GLApp::resize(int width, int height) {
  Dimensions dim{ glEnv.getFramebufferSize() };
  GL(glViewport(0, 0, dim.width, dim.height));
}

void GLApp::drawLines(const std::vector<float>& data, LineDrawType t) {
  simpleProg.enable();
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

  switch (t) {
    case LineDrawType::LD_LIST :
      GL(glDrawArrays(GL_LINES, 0, GLsizei(data.size()/7)));
      break;
    case LineDrawType::LD_STRIP :
      GL(glDrawArrays(GL_LINE_STRIP, 0, GLsizei(data.size()/7)));
      break;
    case LineDrawType::LD_LOOP :
      GL(glDrawArrays(GL_LINE_LOOP, 0, GLsizei(data.size()/7)));
      break;
  }
}

void GLApp::drawPoints(const std::vector<float>& data, float pointSize) {
  simpleProg.enable();
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

  GL(glPointSize(pointSize));
  GL(glDrawArrays(GL_POINTS, 0, GLsizei(data.size()/7)));
}


void GLApp::drawTriangles(const std::vector<float>& data, TrisDrawType t, bool wireframe, bool lighting) {
  
  if (wireframe)
    GL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
  else
    GL(glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));
  
  size_t compCount = lighting ? 10 : 7;
  simpleVb.setData(data,compCount,GL_DYNAMIC_DRAW);

  if (lighting) {
    simpleLightProg.enable();
    simpleArray.bind();
    simpleArray.connectVertexAttrib(simpleVb, simpleLightProg, "vPos", 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleLightProg, "vColor", 4, 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleLightProg, "vNormal", 3, 7);
  } else {
    simpleProg.enable();
    simpleArray.bind();
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);
  }

  switch (t) {
    case TrisDrawType::TD_LIST :
      GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(data.size()/compCount)));
      break;
    case TrisDrawType::TD_STRIP :
      GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(data.size()/compCount)));
      break;
    case TrisDrawType::TD_FAN :
      GL(glDrawArrays(GL_TRIANGLE_FAN, 0, GLsizei(data.size()/compCount)));
      break;
  }
}

void GLApp::setDrawProjection(const Mat4& mat) {
  p = mat;
  shaderUpdate();
}

void GLApp::setDrawTransform(const Mat4& mat) {
  mv = mat;
  shaderUpdate();
}

void GLApp::shaderUpdate() {
  simpleProg.enable();
  simpleProg.setUniform("MVP", mv*p);
  
  simpleLightProg.enable();
  simpleLightProg.setUniform("MVP", mv*p);
  simpleLightProg.setUniform("MV", mv);
  simpleLightProg.setUniform("MVit", Mat4::inverse(mv), true);
}
