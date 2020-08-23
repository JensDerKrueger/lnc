#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

#include <GLEnv.h>
#include <GLTexture2D.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLProgram.h>
#include <GLFramebuffer.h>
#include <Rand.h>

#include <Tesselation.h>

GLEnv gl{1024,1024,4,"OpenGL Game of Life 3D", true, false, 4, 1, true};

GLFramebuffer framebuffer;
GLTexture2D frontFaceTexture{GL_NEAREST, GL_NEAREST};


Tesselation cube{Tesselation::genBrick({0, 0, 0}, {1, 1, 1})};
GLBuffer vbCube{GL_ARRAY_BUFFER};
GLBuffer ibCube{GL_ELEMENT_ARRAY_BUFFER};
GLArray cubeArray;
GLProgram progCubeFront{GLProgram::createFromFile("cubeVS.glsl", "frontFS.glsl")};
GLProgram progCubeBack{GLProgram::createFromFile("cubeVS.glsl", "backFS.glsl")};


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    }
  }
}


void init() {
  GL(glEnable(GL_CULL_FACE));
  GL(glDepthFunc(GL_LESS));

  cubeArray.bind();
  vbCube.setData(cube.getVertices(), 3);
  ibCube.setData(cube.getIndices());
  cubeArray.connectVertexAttrib(vbCube, progCubeFront, "vPos", 3);
  cubeArray.connectIndexBuffer(ibCube);

  // TODO: recreate on window resize
  frontFaceTexture.setEmpty( 1024, 1024, 3);
  
  GL(glClearDepth(1.0f));
  GL(glClearColor(0,0,0,0));
  GL(glEnable(GL_DEPTH_TEST));
}

void render() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glViewport(0, 0, dim.width, dim.height));

  GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));


  const float t0 = glfwGetTime()/5.0;
  const Mat4 m{Mat4::rotationX(t0*157)*Mat4::rotationY(t0*47)};
  const Mat4 v{Mat4::lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0})};
  const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 100)};
  const Mat4 mvp{m*v*p};

  GL(glCullFace(GL_BACK));
  
  framebuffer.bind( frontFaceTexture );
  progCubeFront.enable();
  progCubeFront.setUniform(progCubeFront.getUniformLocation("MVP"), mvp);
  cubeArray.bind();
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  framebuffer.unbind();
  
  
  GL(glCullFace(GL_FRONT));

  progCubeBack.enable();
  progCubeBack.setTexture(progCubeBack.getUniformLocation("frontFaces"),frontFaceTexture,0);
  progCubeBack.setUniform(progCubeBack.getUniformLocation("MVP"), mvp);
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  progCubeBack.unsetTexture(0);
}


int main(int argc, char** argv) {
 // gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  gl.setKeyCallback(keyCallback);

  init();
  GLEnv::checkGLError("BeforeFirstLoop");
  do {
    render();
    GLEnv::checkGLError("endOfFrame");
    gl.endOfFrame();
  } while (!gl.shouldClose());
  
  return EXIT_SUCCESS;
}
