#include <iostream>
#include <cmath>

#include "OpenGLRenderer.h"

#include <bmp.h>
#include <FontRenderer.h>


OpenGLRenderer::OpenGLRenderer(uint32_t width, uint32_t height) :
  Renderer(width,height),
  brick{Tesselation::genBrick({0.0f,0.0f,0.0f}, {0.9f,0.9f,0.9f})},
  vbBrickPos{GL_ARRAY_BUFFER},
  vbBrickNorm{GL_ARRAY_BUFFER},
  vbBrickTan{GL_ARRAY_BUFFER},
  vbBrickTc{GL_ARRAY_BUFFER},
  ibBrick{GL_ELEMENT_ARRAY_BUFFER},
  brickArray{},
  brickAlbedo{GL_LINEAR, GL_LINEAR},
  brickNormalMap{GL_LINEAR, GL_LINEAR},
  background{Tesselation::genBrick({0,0,0}, {float(width),float(height),1}, {1,1,1})},
  vbBackgroundPos{GL_ARRAY_BUFFER},
  vbBackgroundNorm{GL_ARRAY_BUFFER},
  vbBackgroundTan{GL_ARRAY_BUFFER},
  vbBackgroundTc{GL_ARRAY_BUFFER},
  ibBackground{GL_ELEMENT_ARRAY_BUFFER},
  backgroundArray{},
  backgroundAlbedo{GL_LINEAR, GL_LINEAR},
  backgroundNormalMap{GL_LINEAR, GL_LINEAR},
  pillar{Tesselation::genBrick({0,0,0}, {1,float(height),2}, {1.0f/10.f,float(height/10),1.0f/10})},
  vbPillarPos{GL_ARRAY_BUFFER},
  vbPillarNorm{GL_ARRAY_BUFFER},
  vbPillarTan{GL_ARRAY_BUFFER},
  vbPillarTc{GL_ARRAY_BUFFER},
  ibPillar{GL_ELEMENT_ARRAY_BUFFER},
  pillarArray{},
  pillarAlbedo{GL_LINEAR, GL_LINEAR},
  pillarNormalMap{GL_LINEAR, GL_LINEAR},
  progBrick{GLProgram::createFromFile("brickVertex.glsl", "brickFragment.glsl")},
  mvpLocationBrick{progBrick.getUniformLocation("MVP")},
  mLocationBrick{progBrick.getUniformLocation("M")},
  mitLocationBrick{progBrick.getUniformLocation("Mit")},
  invVLocationBrick{progBrick.getUniformLocation("invV")},
  colorBrickLocation{progBrick.getUniformLocation("color")},
  opacityBrickLocation{progBrick.getUniformLocation("opacity")},
  fractalAnimationLocation{progBrick.getUniformLocation("animation")},
  progNormalMap{GLProgram::createFromFile("backgroundVertex.glsl", "backgroundFragment.glsl")},
  mvpLocationNormalMap{progNormalMap.getUniformLocation("MVP")},
  mLocationNormalMap{progNormalMap.getUniformLocation("M")},
  mitLocationNormalMap{progNormalMap.getUniformLocation("Mit")},
  invVLocationNormalMap{progNormalMap.getUniformLocation("invV")},
  lpLocationNormalMap{progNormalMap.getUniformLocation("vLightPos")},
  texLocationNormalMap{progNormalMap.getUniformLocation("textureSampler")},
  normMapLocationNormalMap{progNormalMap.getUniformLocation("normalSampler")},
  colorLocation{progNormalMap.getUniformLocation("color")},
  opacityLocation{progNormalMap.getUniformLocation("opacity")},

  progBackground{GLProgram::createFromFile("fractalVertex.glsl", "fractalFragment.glsl")},
  mvpLocationBackground{progBackground.getUniformLocation("MVP")},
  mLocationBackground{progBackground.getUniformLocation("M")},
  mitLocationBackground{progBackground.getUniformLocation("Mit")},
  invVLocationBackground{progBackground.getUniformLocation("invV")},
  lpLocationBackground{progBackground.getUniformLocation("vLightPos")},
  animationBackgroundLocation{progBackground.getUniformLocation("animation")},
  fractalParamBackgroundLocation{progBackground.getUniformLocation("fractParam")},
  
  starter(std::make_shared<BrickStart>(Vec3{0,0,0},Vec3{0,0,0})),
  particleSystem{8000, starter, {-10,-10,50}, {10,10,55}, {0,0,0}, {-100.0f,-100.0f,-100.0f}, {100.0f,100.0f,100.0f}, 5.0f, 80.0f, RAINBOW_COLOR, false},
    particleBitmap(std::make_shared<Bitmap>("start.bmp", 64)),
    scoreParticleSystem(6000, particleBitmap, {-0.2f,-0.2f,0.0f}, {0.2f,0.2f,0.0f}, {0.0f,0.0f,0.0f}, 10.0f, 80.0f, {1.0f,1.0f,1.0f}, true),
  viewerPos{0.0f,0.0f,5.0f},
  animationStartTime{0.0},
  currentTime{0.0},
    gameOver{false}
{
    setBackgroundParam(0.8f);
    
  vbBrickPos.setData(brick.getVertices(),3);
  vbBrickNorm.setData(brick.getNormals(),3);
  vbBrickTan.setData(brick.getTangents(),3);
  vbBrickTc.setData(brick.getTexCoords(),2);
  ibBrick.setData(brick.getIndices());
  
  brickArray.bind();
  brickArray.connectVertexAttrib(vbBrickPos,progBrick,"vPos",3);
  brickArray.connectVertexAttrib(vbBrickNorm,progBrick,"vNorm",3);
  brickArray.connectVertexAttrib(vbBrickTc,progBrick,"vTc",2);
  brickArray.connectIndexBuffer(ibBrick);

  vbBackgroundPos.setData(background.getVertices(),3);
  vbBackgroundNorm.setData(background.getNormals(),3);
  vbBackgroundTan.setData(background.getTangents(),3);
  vbBackgroundTc.setData(background.getTexCoords(),2);
  ibBackground.setData(background.getIndices());
  Image backgroundAlbedoImage{BMP::load("BackgroundAlbedo.bmp")};
  backgroundAlbedo.setData(backgroundAlbedoImage.data, backgroundAlbedoImage.width, backgroundAlbedoImage.height, backgroundAlbedoImage.componentCount);
  Image backgroundNormalImage{BMP::load("BackgroundNormal.bmp")};
  backgroundNormalMap.setData(backgroundNormalImage.data, backgroundNormalImage.width, backgroundNormalImage.height, backgroundNormalImage.componentCount);
  
  backgroundArray.bind();
  backgroundArray.connectVertexAttrib(vbBackgroundPos,progBackground,"vPos",3);
  backgroundArray.connectVertexAttrib(vbBackgroundNorm,progBackground,"vNorm",3);
  backgroundArray.connectVertexAttrib(vbBackgroundTc,progBackground,"vTc",2);
  backgroundArray.connectIndexBuffer(ibBackground);

  vbPillarPos.setData(pillar.getVertices(),3);
  vbPillarNorm.setData(pillar.getNormals(),3);
  vbPillarTan.setData(pillar.getTangents(),3);
  vbPillarTc.setData(pillar.getTexCoords(),2);
  ibPillar.setData(pillar.getIndices());
  Image pillarAlbedoImage{BMP::load("BackgroundAlbedo.bmp")};
  pillarAlbedo.setData(pillarAlbedoImage.data, pillarAlbedoImage.width, pillarAlbedoImage.height, pillarAlbedoImage.componentCount);
  Image pillarNormalImage{BMP::load("BackgroundNormal.bmp")};
  pillarNormalMap.setData(pillarNormalImage.data, pillarNormalImage.width, pillarNormalImage.height, pillarNormalImage.componentCount);
  
  pillarArray.bind();
  pillarArray.connectVertexAttrib(vbPillarPos,progNormalMap,"vPos",3);
  pillarArray.connectVertexAttrib(vbPillarNorm,progNormalMap,"vNorm",3);
  pillarArray.connectVertexAttrib(vbPillarTan,progNormalMap,"vTan",3);
  pillarArray.connectVertexAttrib(vbPillarTc,progNormalMap,"vTc",2);
  pillarArray.connectIndexBuffer(ibPillar);
}

Vec3 OpenGLRenderer::pos2Coord(const Vec2& pos, float dist) const {
  return {float(pos.x)-float(width())/2+0.5f,
      float(height()-pos.y)-float(height()/2)-0.5f,
      -dist};
}


void OpenGLRenderer::clearRows() {
  if (clearedRows.empty()) return;
  
  const size_t particlesPerRow{((1<<clearedRows.size())*200)/clearedRows.size()};
  
  for (uint32_t row : clearedRows) {
    Vec3 coord{pos2Coord(Vec2(width()/2,row), 20.0f)};
    starter->setStart(coord, Vec3(float(width()),1.0f,1.0f));
    particleSystem.setInitialSpeed( (viewerPos-coord)*2, (viewerPos-coord)*2+Vec3{staticRand.rand01(),staticRand.rand01(),staticRand.rand01()}*3  );
    particleSystem.restart(particlesPerRow);
  }
}

void OpenGLRenderer::dropAnimation(const std::array<Vec2i,4>& source, const Vec3& sourceColor, const std::array<Vec2i,4>& target, const std::vector<uint32_t>& clearedRows) {
  size_t i = 1;
  size_t maxIndex = 0;
  for (;i<source.size();++i) {
    if (source[i].y > source[maxIndex].y) maxIndex = i;
  }
  animationStart = source[maxIndex];
  animationCurrent = Vec2(animationStart);
  animationTarget = target[maxIndex];
  droppedTetromino = source;
  droppedTetrominoColor = sourceColor;
  this->clearedRows = clearedRows;
  
  animationStartTime = currentTime;
}

bool OpenGLRenderer::isAnimating() const {
  return animationCurrent != Vec2(animationTarget);
}

void OpenGLRenderer::setGameOver(bool gameOver, uint32_t score) {
  this->gameOver = gameOver;
  if (gameOver) {
    FontRenderer fr("numbers.bmp", "numbers.pos");
    particleBitmap = std::make_shared<Bitmap>(fr.render(score), 64);
    scoreParticleSystem.setBitmap(particleBitmap);
  }
}

void OpenGLRenderer::render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor,
              const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
              const std::array<Vec2i,4>& targerTetrominoPos, float time) {
                
  currentTime = time;
                
  // setup basic OpenGL states that do not change during the frame
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearDepth(1.0f);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  
  Vec3 lookAtVec;
  Vec3 lookFromVec;
  Vec3 upVec;
  Vec3 lightPos;
  std::array<Vec2,4> activeTetrominoPos;
  Vec3 activeTetrominoColor;
  
  lookFromVec = viewerPos;
  lookAtVec = Vec3{0,0,0};
  upVec = Vec3{0,1,0};
  lightPos = Mat4::rotationZ(time*20) * Vec3{0,10,-4};

  if (isAnimating()) {
    if (clearedRows.size() == 4) {
      const float totalTime = float(animationTarget.y - animationStart.y)*0.08f;
      const float a = float((currentTime - animationStartTime)/totalTime);
      if (a < 1.0f)
        animationCurrent = Vec2(animationStart) * (1-a) + Vec2(animationTarget) * a;
      else {
        animationCurrent = Vec2(animationTarget);
      }
      lookFromVec = Vec3{pos2Coord(animationCurrent, 20.0f)};
      lookAtVec = lookFromVec+Vec3(0,-10,0);
      upVec = Vec3{0,0,1};
      lightPos = lookFromVec;
    } else {
      const float totalTime = float(animationTarget.y - animationStart.y)*0.01f;
      double a = (currentTime - animationStartTime)/totalTime;
      if (a>=1.0) {
        animationCurrent = Vec2(animationTarget);
        a = 1.0;
      }
      for (size_t i = 0;i<tetrominoPos.size();++i) {
        Vec2 temp{float(droppedTetromino[i].x), float(droppedTetromino[i].y + (animationTarget.y-animationStart.y) * a)};
        activeTetrominoPos[i] = temp;
      }
      activeTetrominoColor = droppedTetrominoColor;
    }
    if (!isAnimating()) clearRows();
  } else {
    for (size_t i = 0;i<tetrominoPos.size();++i)
      activeTetrominoPos[i] = Vec2(tetrominoPos[i]);
    activeTetrominoColor = currentColor;
  }
  
  Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};

  // setup viewport and clear buffers
  glViewport(0, 0, dim.width, dim.height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const Mat4 p{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 1000.0f)};

  Mat4 m{Mat4::translation(Vec3{0,0,-21.0f })};
  backgroundArray.bind();
  
  progBackground.enable();
  progBackground.setUniform(lpLocationBackground, lightPos);
  progBackground.setUniform(mvpLocationBackground, p*v*m);
  progBackground.setUniform(mLocationBackground, m);
  progBackground.setUniform(mitLocationBackground, Mat4::inverse(m), true);
  progBackground.setUniform(invVLocationBackground, Mat4::inverse(v));
  progBackground.setUniform(animationBackgroundLocation, time/10);
  progBackground.setUniform(fractalParamBackgroundLocation, getBackgroundParam());

  glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);

  pillarArray.bind();
  progNormalMap.enable();
  m = Mat4{Mat4::translation(Vec3{float(width())/2.0f+0.5f,0,-20.0f })};
  progNormalMap.setUniform(mvpLocationNormalMap, p*v*m);
  progNormalMap.setUniform(mLocationNormalMap, m);
  progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
  progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
  progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});
  progNormalMap.setUniform(lpLocationNormalMap, lightPos);
  progNormalMap.setTexture(normMapLocationNormalMap,backgroundNormalMap,0);
  progNormalMap.setTexture(texLocationNormalMap,backgroundAlbedo,1);

  glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);

  m = Mat4::translation(Vec3{-(float(width())/2.0f+0.5f),0,-20.0f });
  progNormalMap.setUniform(mvpLocationNormalMap, p*v*m);
  progNormalMap.setUniform(mLocationNormalMap, m);
  progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
  progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
  progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});

  glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);

  progBrick.enable();
  brickArray.bind();
  progBrick.setUniform(opacityBrickLocation, 1.0f);
  progBrick.setUniform(invVLocationBrick, Mat4::inverse(v));
  progBrick.setUniform(fractalAnimationLocation, time);
  progBrick.setUniform(lpLocationNormalMap, lightPos);

  uint32_t i{0};
  for (uint32_t y = 0;y < height();++y) {
    for (uint32_t x = 0;x < width();++x) {
      const Vec3 c = getObstacles()[i++];
      
      if (c.r < 0) continue; // empty spaces
      
      const Mat4 m{Mat4::translation(pos2Coord(Vec2(x,y), 20.0f))};
      progBrick.setUniform(mvpLocationBrick, p*v*m);
      progBrick.setUniform(mLocationBrick, m);
      progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);
      progBrick.setUniform(colorBrickLocation, c);
      glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);
    }
  }
  
  for (const Vec2& pos : activeTetrominoPos) {
    const Mat4 m{Mat4::translation(pos2Coord(pos, 20.0f))};
    progBrick.setUniform(mvpLocationBrick, p*v*m);
    progBrick.setUniform(mLocationBrick, m);
    progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);
    progBrick.setUniform(colorBrickLocation, activeTetrominoColor);
    glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);
  }

  if (getShowPreview()) {
    for (const Vec2i& pos : nextTetrominoPos) {
      const Mat4 m{Mat4::translation(pos2Coord(Vec2(pos)+Vec2(13,17), 20.0f))};
      progBrick.setUniform(mvpLocationBrick, p*v*m);
      progBrick.setUniform(mLocationBrick, m);
      progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);
      progBrick.setUniform(colorBrickLocation, nextColor);
      glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);
    }
  }
  
  if (getShowTarget() && !isAnimating()) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);
      
    for (const Vec2i& pos : targerTetrominoPos) {
      const Mat4 m{Mat4::translation(pos2Coord(Vec2(pos), 20.0f))};
      progBrick.setUniform(mvpLocationBrick, p*v*m);
      progBrick.setUniform(mLocationBrick, m);
      progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);
      progBrick.setUniform(colorBrickLocation, currentColor);
      progBrick.setUniform(opacityBrickLocation, 0.2f);
      glDrawElements(GL_TRIANGLES, GLsizei(brick.getIndices().size()), GL_UNSIGNED_INT, (void*)0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
  
  particleSystem.setPointSize(dim.height/10.0f);
  particleSystem.render(v,p);
  particleSystem.update(time);

    if (gameOver) {
        m = Mat4::scaling(2,2,0)*Mat4::translation(Vec3{-0.5,-0.5,0});
        scoreParticleSystem.setPointSize(dim.height/10.0f);
        scoreParticleSystem.render(v*m,p);
        scoreParticleSystem.update(time);
    }
}
