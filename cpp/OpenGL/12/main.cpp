#include <iostream>
#include <vector>
#include <string>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include <Vec3.h>
#include <Mat4.h>
#include <Rand.h>

#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"

std::vector<float> genTree() {
    std::vector<float> data{ 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                             0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
    };
    return data;
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}

void checkGLError(const std::string& id) {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        std::cerr << "An openGL error occured:" << e << " at " << id << std::endl;
    }
}
    
int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Plant Growth", true, true, 4, 1, true};

    std::string vsString{
    "#version 410\n"
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec3 vPos;\n"
    "layout (location = 1) in vec4 vColor;\n"
    "out vec4 color;"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = vColor;\n"
    "}\n"};

    std::string fsString{
    "#version 410\n"
    "in vec4 color;"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = color;\n"
    "}\n"};
    
    GLProgram prog{GLProgram::createFromString(vsString, fsString)};
    GLint mvpLocation{prog.getUniformLocation("MVP")};
    
    GLArray lineArray{};
    GLBuffer vbLinePos{GL_ARRAY_BUFFER};

    const std::vector<float> data{genTree()};
    const size_t lineVertexCount{data.size()/7};
    
    vbLinePos.setData(data,7,GL_DYNAMIC_DRAW);

    lineArray.bind();
    lineArray.connectVertexAttrib(vbLinePos, prog, "vPos", 3);
    lineArray.connectVertexAttrib(vbLinePos, prog, "vColor", 4, 3);
    
    gl.setKeyCallback(keyCallback);
    glfwSetTime(0);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    Vec3 lookAtVec{0,0,0};
    Vec3 lookFromVec{0,1,1};
    Vec3 upVec{0,1,0};
        
    checkGLError("init");

    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        const Mat4 p{Mat4::ortho(-1,1,-1/dim.aspect(),1/dim.aspect(),-1,1)};
            
        prog.enable();
        prog.setUniform(mvpLocation, p);
    
        lineArray.bind();
        glDrawArrays(GL_LINES, 0, GLsizei(lineVertexCount));

        checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
