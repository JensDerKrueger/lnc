#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GLEnv.h>
#include <GLProgram.h>
#include <GLBuffer.h>
#include <Mat4.h>
  
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char ** argv) {
  GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 3"};
  gl.setKeyCallback(keyCallback);

  GLBuffer vbPosColor{GL_ARRAY_BUFFER};
  vbPosColor.setData(std::vector<float>{
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
     0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f
  }, 6);
  

  GLBuffer ib{GL_ELEMENT_ARRAY_BUFFER};
  ib.setData(std::vector<GLuint>{
    0,1,2,0,2,3,
    7,6,4,6,5,4,
    
    3,2,6,6,7,3,
    5,1,0,4,5,0,
    
    1,5,6,6,2,1,
    3,7,4,4,0,3
  });
  
  const GLProgram prog = GLProgram::createFromFile("vertex.glsl", "fragment.glsl");
  const GLint posLocation = prog.getAttributeLocation("vPos");
  const GLint colLocation = prog.getAttributeLocation("vCol");

  vbPosColor.connectVertexAttrib(posLocation, 3);
  vbPosColor.connectVertexAttrib(colLocation, 3, 3);
 
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  glClearDepth(1.0f);
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
  do {
    const Dimensions dim{gl.getFramebufferSize()};
  
    glViewport(0, 0, dim.width, dim.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    const Mat4 m{Mat4::rotationY(glfwGetTime()*77)*Mat4::rotationX(glfwGetTime()*57)};
    const Mat4 v{Mat4::lookAt({0,0,2},{0,0,0},{0,1,0})};
    const Mat4 p{Mat4::perspective(90, dim.aspect(), 0.0001, 100)};
    const Mat4 mvp{p*v*m};
    
    prog.enable();
    prog.setUniform("MVP", mvp);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

    gl.endOfFrame();
    } while (!gl.shouldClose());
  
    return EXIT_SUCCESS;
}

