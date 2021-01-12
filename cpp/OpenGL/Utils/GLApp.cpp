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
  simpleSpriteProg{GLProgram::createFromString(
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
     "uniform sampler2D pointSprite;\n"
     "in vec4 color;\n"
     "out vec4 FragColor;\n"
     "void main() {\n"
     "    FragColor = color*texture(pointSprite, gl_PointCoord);\n"
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
  pointSprite{GL_LINEAR, GL_LINEAR},
  resumeTime{0},
  animationActive{true}
{
  staticAppPtr = this;
  glEnv.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  glEnv.setKeyCallback(keyCallback);
  glEnv.setResizeCallback(sizeCallback);
  
  resetPointTexture();

  
  // setup a minimal shader and buffer
  shaderUpdate();
  
  glfwSetTime(0);
  Dimensions dim{ glEnv.getFramebufferSize() };
  glViewport(0, 0, dim.width, dim.height);
}

void GLApp::setPointTexture(const std::vector<uint8_t>& shape, uint32_t x, uint32_t y, uint32_t components) {
  pointSprite.setData(shape, x, y, components);
}

void GLApp::resetPointTexture(uint32_t resolution) {
  std::vector<uint8_t> disk(resolution*resolution*4);
  const Vec2 center{0.0f,0.0f};
  for (size_t y = 0;y<resolution;++y) {
    for (size_t x = 0;x<resolution;++x) {
      const Vec2 normPos{2.0f*x/float(resolution)-1.0f, 2.0f*y/float(resolution)-1.0f};
      const uint8_t dist = uint8_t(std::max<int16_t>(0, int16_t((1.0f-(center - normPos).length()) * 255))); 
      disk[4*(x+y*resolution)+0] = 255;
      disk[4*(x+y*resolution)+1] = 255;
      disk[4*(x+y*resolution)+2] = 255;
      disk[4*(x+y*resolution)+3] = dist;
    }
  }
  setPointTexture(disk, resolution, resolution, 4);
}

void GLApp::run() {
  init();
  const Dimensions dim{ glEnv.getFramebufferSize() };
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
  const Dimensions dim{ glEnv.getFramebufferSize() };
  GL(glViewport(0, 0, dim.width, dim.height));
}


void GLApp::triangulate(const Vec3& p0,
                        const Vec3& p1, const Vec4& c1,
                        const Vec3& p2, const Vec4& c2,
                        const Vec3& p3,
                        float lineThickness,
                        std::vector<float>& trisData) {

  const Dimensions dim{ glEnv.getFramebufferSize() };
  const Vec3 scale{Vec3{2.0f/float(dim.width), 2.0f/float(dim.height), 1.0} * lineThickness};

  const Vec3 pDir = Vec3::normalize(p1-p0);
  const Vec3 cDir = Vec3::normalize(p2-p1);
  const Vec3 nDir = Vec3::normalize(p3-p2);
  
  const Vec3 viewDir = Vec3::normalize((mvi * Vec4{0,0,1,0}).xyz());
  
  const Vec3 pPerp = Vec3::cross(pDir, viewDir);
  const Vec3 cPerp = Vec3::cross(cDir, viewDir);
  const Vec3 nPerp = Vec3::cross(nDir, viewDir);
  
  Vec3 pSep = pPerp + cPerp;
  Vec3 nSep = nPerp + cPerp;
  
  pSep = (pSep / std::max(1.0f,Vec3::dot(pSep, cPerp))) * scale;
  nSep = (nSep / std::max(1.0f,Vec3::dot(nSep, cPerp))) * scale;

  trisData.push_back(p1[0]+pSep[0]); trisData.push_back(p1[1]+pSep[1]);trisData.push_back(p1[2]+pSep[2]);
  trisData.push_back(c1[0]);trisData.push_back(c1[1]); trisData.push_back(c1[2]); trisData.push_back(c1[3]);

  trisData.push_back(p2[0]+nSep[0]); trisData.push_back(p2[1]+nSep[1]);trisData.push_back(p2[2]+nSep[2]);
  trisData.push_back(c2[0]);trisData.push_back(c2[1]); trisData.push_back(c2[2]); trisData.push_back(c2[3]);

  trisData.push_back(p1[0]-pSep[0]); trisData.push_back(p1[1]-pSep[1]);trisData.push_back(p1[2]-pSep[2]);
  trisData.push_back(c1[0]);trisData.push_back(c1[1]); trisData.push_back(c1[2]); trisData.push_back(c1[3]);

  trisData.push_back(p2[0]+nSep[0]); trisData.push_back(p2[1]+nSep[1]);trisData.push_back(p2[2]+nSep[2]);
  trisData.push_back(c2[0]);trisData.push_back(c2[1]); trisData.push_back(c2[2]); trisData.push_back(c2[3]);

  trisData.push_back(p2[0]-nSep[0]); trisData.push_back(p2[1]-nSep[1]);trisData.push_back(p2[2]-nSep[2]);
  trisData.push_back(c2[0]);trisData.push_back(c2[1]); trisData.push_back(c2[2]); trisData.push_back(c2[3]);

  trisData.push_back(p1[0]-pSep[0]); trisData.push_back(p1[1]-pSep[1]);trisData.push_back(p1[2]-pSep[2]);
  trisData.push_back(c1[0]);trisData.push_back(c1[1]); trisData.push_back(c1[2]); trisData.push_back(c1[3]);
}


void GLApp::drawLines(const std::vector<float>& data, LineDrawType t, float lineThickness) {
  shaderUpdate();
  
  simpleProg.enable();
  simpleArray.bind();

  if (lineThickness > 1.0f) {
    std::vector<float> trisData;
    
    switch (t) {
      case LineDrawType::LIST :
        for (size_t i = 0;i<data.size()/7;i+=2) {
                    
          const size_t i1 = i;
          const size_t i2 = i+1;

          const Vec3 p1{data[i1*7+0],data[i1*7+1],data[i1*7+2]};
          const Vec4 c1{data[i1*7+3],data[i1*7+4],data[i1*7+5],data[i1*7+6]};
          const Vec3 p2{data[i2*7+0],data[i2*7+1],data[i2*7+2]};
          const Vec4 c2{data[i2*7+3],data[i2*7+4],data[i2*7+5],data[i2*7+6]};

          Vec3 p0{p1};
          Vec3 p3{p2};
          
          if (i1 >= 2 && p1[0] == data[(i1-1)*7+0] && p1[1] == data[(i1-1)*7+1] && p1[2] == data[(i1-1)*7+2]) {
            const size_t i0 = i-2;
            p0 = Vec3{data[i0*7+0],data[i0*7+1],data[i0*7+2]};
          }

          if (i2+2 < data.size()/7 && p2[0] == data[(i2+1)*7+0] && p2[1] == data[(i2+1)*7+1] && p2[2] == data[(i2+1)*7+2]) {
            const size_t i3 = i+2;
            p3 = Vec3{data[i3*7+0],data[i3*7+1],data[i3*7+2]};
          }

          triangulate(p0, p1, c1, p2, c2, p3, lineThickness, trisData);
        }
        break;
      case LineDrawType::STRIP :
        for (size_t i = 0;i<(data.size()/7)-1;++i) {
          
          const size_t i0 = (i==0) ? 0 : i-1;
          const size_t i1 = i;
          const size_t i2 = i+1;
          const size_t i3 = (i==(data.size()/7)-2) ? i2 : i2+1;
          
          const Vec3 p0{data[i0*7+0],data[i0*7+1],data[i0*7+2]};
          const Vec3 p1{data[i1*7+0],data[i1*7+1],data[i1*7+2]};
          const Vec4 c1{data[i1*7+3],data[i1*7+4],data[i1*7+5],data[i1*7+6]};
          const Vec3 p2{data[i2*7+0],data[i2*7+1],data[i2*7+2]};
          const Vec4 c2{data[i2*7+3],data[i2*7+4],data[i2*7+5],data[i2*7+6]};
          const Vec3 p3{data[i3*7+0],data[i3*7+1],data[i3*7+2]};

          triangulate(p0, p1, c1, p2, c2, p3, lineThickness, trisData);
        }
        break;
      case LineDrawType::LOOP :
        for (size_t i = 0;i<data.size()/7;++i) {

          const size_t i0 = (i==0) ? 0 : i-1;
          const size_t i1 = i;
          const size_t i2 = (i+1)%data.size();
          const size_t i3 = (i==(data.size()/7)-1) ? i2 : (i2+1)%data.size();
          
          const Vec3 p0{data[i0*7+0],data[i0*7+1],data[i0*7+2]};
          const Vec3 p1{data[i1*7+0],data[i1*7+1],data[i1*7+2]};
          const Vec4 c1{data[i1*7+3],data[i1*7+4],data[i1*7+5],data[i1*7+6]};
          const Vec3 p2{data[i2*7+0],data[i2*7+1],data[i2*7+2]};
          const Vec4 c2{data[i2*7+3],data[i2*7+4],data[i2*7+5],data[i2*7+6]};
          const Vec3 p3{data[i3*7+0],data[i3*7+1],data[i3*7+2]};

          triangulate(p0, p1, c1, p2, c2, p3, lineThickness, trisData);
        }
        break;
    }
    
    GL(glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));

    simpleVb.setData(trisData,7,GL_DYNAMIC_DRAW);
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);

    GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(trisData.size()/7)));
  } else {
    simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
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
}

void GLApp::drawPoints(const std::vector<float>& data, float pointSize, bool useTex) {
  shaderUpdate();
  
  if (useTex) {
    simpleSpriteProg.enable();
    simpleSpriteProg.setTexture("pointSprite", pointSprite, 0);
    simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
    simpleArray.bind();
    simpleArray.connectVertexAttrib(simpleVb, simpleSpriteProg, "vPos", 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleSpriteProg, "vColor", 4, 3);
  } else {
    simpleProg.enable();
    simpleVb.setData(data,7,GL_DYNAMIC_DRAW);
    simpleArray.bind();
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
    simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vColor", 4, 3);
  }


  GL(glPointSize(pointSize));
  GL(glDrawArrays(GL_POINTS, 0, GLsizei(data.size()/7)));
}

void GLApp::redrawTriangles(bool wireframe) {
  shaderUpdate();

  if (lastLighting) {
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
  
  if (wireframe)
    GL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
  else
    GL(glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));

  switch (lastTrisType) {
    case TrisDrawType::LIST :
      GL(glDrawArrays(GL_TRIANGLES, 0, lastTrisCount));
      break;
    case TrisDrawType::STRIP :
      GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, lastTrisCount));
      break;
    case TrisDrawType::FAN :
      GL(glDrawArrays(GL_TRIANGLE_FAN, 0, lastTrisCount));
      break;
  }
}


void GLApp::drawTriangles(const std::vector<float>& data, TrisDrawType t, bool wireframe, bool lighting) {
  shaderUpdate();
  
  size_t compCount = lighting ? 10 : 7;
  simpleVb.setData(data,compCount,GL_DYNAMIC_DRAW);

  lastLighting = lighting;
  lastTrisType = t;
  lastTrisCount = GLsizei(data.size()/compCount);

  redrawTriangles(wireframe);
}

void GLApp::setDrawProjection(const Mat4& mat) {
  p = mat;
}

void GLApp::setDrawTransform(const Mat4& mat) {
  mv = mat;
  mvi = Mat4::inverse(mv);
}

void GLApp::shaderUpdate() {
  simpleProg.enable();
  simpleProg.setUniform("MVP", mv*p);

  simpleSpriteProg.enable();
  simpleSpriteProg.setUniform("MVP", mv*p);

  simpleTexProg.enable();
  simpleTexProg.setUniform("MVP", mv*p);

  simpleLightProg.enable();
  simpleLightProg.setUniform("MVP", mv*p);
  simpleLightProg.setUniform("MV", mv);
  simpleLightProg.setUniform("MVit", mvi, true);
}

void GLApp::setImageFilter(GLint magFilter, GLint minFilter) {
  raster.setFilter(magFilter, minFilter);
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
