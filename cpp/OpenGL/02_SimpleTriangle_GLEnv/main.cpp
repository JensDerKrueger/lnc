#include <vector>

#include <GLEnv.h>
#include <GLProgram.h>
#include <Mat4.h>

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char ** argv) {
  GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 2"};
  gl.setKeyCallback(keyCallback);

  std::vector<float> vertices {
   -0.6f, -0.4f, -1.0f, 1.0f, 0.0f, 0.0f,
    0.6f, -0.4f, -1.0f, 0.0f, 1.0f, 0.0f,
    0.0f,  0.6f, -1.0f, 0.0f, 0.0f, 1.0f
  };

  GLuint vertexBuffer{};
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

  GLProgram program{GLProgram::createFromFile("vertexShader.glsl", "fragmentShader.glsl")};

  GLint mvpLocation = program.getUniformLocation("MVP");
  GLint posLocation = program.getAttributeLocation("vPos");
  GLint colLocation = program.getAttributeLocation("vCol");
  
  glEnableVertexAttribArray(posLocation);
  glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)0);
  glEnableVertexAttribArray(colLocation);
  glVertexAttribPointer(colLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)(sizeof(float)*3));
  
  do {
    const Dimensions dim{gl.getFramebufferSize()};
    glClearColor(0.0f,0.0f,1.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, dim.width, dim.height);
    const Mat4 m{Mat4::rotationZ(glfwGetTime()*20)};
    const Mat4 v{Mat4::lookAt({0,0,0}, {0,0,-1}, {0,1,0})};
    const Mat4 p{Mat4::perspective(90, dim.aspect(), 0.0001, 100)};
    const Mat4 mvp{p*v*m};
      
    program.enable();
    glUniformMatrix4fv(mvpLocation, 1, GL_TRUE, mvp);  // OpenGL uses column-major order -> transpose
    glDrawArrays(GL_TRIANGLES, 0, 3);
    gl.endOfFrame();
  } while (!gl.shouldClose());
  return EXIT_SUCCESS;
}  
