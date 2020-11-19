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
     "out vec4 color;\n"
     "void main() {\n"
     "    gl_Position = MVP * vec4(vPos, 1.0);\n"
     "    color = vColor;\n"
     "}\n",
     "#version 410\n"
     "in vec4 color;\n"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    FragColor = color;\n"
     "}\n")},
  simpleTexProg{GLProgram::createFromString(
     "#version 410\n"
     "uniform mat4 MVP;\n"
     "layout (location = 0) in vec3 vPos;\n"
     "layout (location = 1) in vec2 vTexCoords;\n"
     "out vec4 color;\n"
     "out vec2 texCoords;\n"
     "void main() {\n"
     "    gl_Position = MVP * vec4(vPos, 1.0);\n"
     "    texCoords = vTexCoords;\n"
     "}\n",
     "#version 410\n"
     "uniform sampler2D raster;\n"
     "in vec2 texCoords;\n"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    FragColor = texture(raster, texCoords);\n"
     "}\n")},
  simpleLightProg{GLProgram::createFromString(
     "#version 410\n"
     "uniform mat4 MVP;\n"
     "uniform mat4 MV;\n"
     "uniform mat4 MVit;\n"
     "layout (location = 0) in vec3 vPos;\n"
     "layout (location = 1) in vec4 vColor;\n"
     "layout (location = 2) in vec3 vNormal;\n"
     "out vec4 color;\n"
     "out vec3 normal;\n"
     "out vec3 pos;\n"
     "void main() {\n"
     "    gl_Position = MVP * vec4(vPos, 1.0);\n"
     "    pos = (MV * vec4(vPos, 1.0)).xyz;\n"
     "    color = vColor;\n"
     "    normal = (MVit * vec4(vNormal, 0.0)).xyz;\n"
     "}\n",
     "#version 410\n"
     "in vec4 color;\n"
     "in vec3 pos;\n"
     "in vec3 normal;\n"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    vec3 nnormal = normalize(normal);"
     "    vec3 nlightDir = normalize(vec3(0.0,0.0,0.0)-pos);"
     "    FragColor = color*abs(dot(nlightDir,nnormal));\n"
     "}\n")},
  simpleArray{},
  simpleVb{GL_ARRAY_BUFFER},
  raster{GL_LINEAR, GL_LINEAR},
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
  shaderUpdate();
  
  simpleProg.enable();
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

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

void GLApp::drawPoints(const std::vector<float>& data, float pointSize) {
  shaderUpdate();
  
  simpleProg.enable();
  simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

  GL(glPointSize(pointSize));
  GL(glDrawArrays(GL_POINTS, 0, GLsizei(data.size()/7)));
}


void GLApp::drawTriangles(const std::vector<float>& data, TrisDrawType t, bool wireframe, bool lighting) {
  shaderUpdate();
  
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
    case TrisDrawType::LIST :
      GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(data.size()/compCount)));
      break;
    case TrisDrawType::STRIP :
      GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, GLsizei(data.size()/compCount)));
      break;
    case TrisDrawType::FAN :
      GL(glDrawArrays(GL_TRIANGLE_FAN, 0, GLsizei(data.size()/compCount)));
      break;
  }
}

void GLApp::setDrawProjection(const Mat4& mat) {
  p = mat;
}

void GLApp::setDrawTransform(const Mat4& mat) {
  mv = mat;
}

void GLApp::shaderUpdate() {
  simpleProg.enable();
  simpleProg.setUniform("MVP", mv*p);

  simpleTexProg.enable();
  simpleTexProg.setUniform("MVP", mv*p);

  simpleLightProg.enable();
  simpleLightProg.setUniform("MVP", mv*p);
  simpleLightProg.setUniform("MV", mv);
  simpleLightProg.setUniform("MVit", Mat4::inverse(mv), true);
}

void GLApp::drawImage(const Image& image, const Vec3& bl,
                      const Vec3& br, const Vec3& tl,
                      const Vec3& tr) {
  shaderUpdate();
  
  simpleTexProg.enable();
  
  std::vector<float> data = {
    tr[0], tr[1], tr[2], 1.0f, 1.0f,
    br[0], br[1], br[2], 1.0f, 0.0f,
    tl[0], tl[1], tl[2], 0.0f, 1.0f,
    tl[0], tl[1], tl[2], 0.0f, 1.0f,
    bl[0], bl[1], bl[2], 0.0f, 0.0f,
    br[0], br[1], br[2], 1.0f, 0.0f
  };
  
  simpleVb.setData(data,5,GL_DYNAMIC_DRAW);
    
  raster.setData(image.data, image.width, image.height, image.componentCount);
  
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleTexProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleTexProg, "vTexCoords", 2, 3);
  simpleTexProg.setTexture("raster",raster,0);

  GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(data.size()/5)));

}
