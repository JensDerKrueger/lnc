#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>  

#include "GLEnv.h"
#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLTexture2D.h"

#include "Mat4.h"
#include "Tesselation.h"

  
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
} 

int main(int argc, char ** argv) {
    GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 5"};
    
    gl.setKeyCallback(keyCallback);

    Tesselation sphere{Tesselation::genSphere({0,0,0}, 0.7f, 30, 30)};

    GLBuffer vbPos{GL_ARRAY_BUFFER};
    vbPos.setData(sphere.getVertices(),3);

    GLBuffer vbNorm{GL_ARRAY_BUFFER};
    vbNorm.setData(sphere.getNormals(),3);

    GLBuffer vbTc{GL_ARRAY_BUFFER};
    vbTc.setData(sphere.getTexCoords(),2);

    GLBuffer ib{GL_ELEMENT_ARRAY_BUFFER};
    ib.setData(sphere.getIndices());
    
    GLTexture2D checkerTex{2,2};    
    checkerTex.setData({255,0,0,255, 0,0,0,255, 0,0,0,255, 255,0,0,255});

    GLTexture2D checkerTex2{4,4};    
    checkerTex2.setData({0,255,0,255, 0,0,0,255, 0,255,0,255, 0,0,0,255,
                        0,0,0,255, 0,255,0,255,0,0,0,255, 0,255,0,255,
                        0,255,0,255, 0,0,0,255, 0,255,0,255, 0,0,0,255,
                        0,0,0,255, 0,255,0,255,0,0,0,255, 0,255,0,255});
    
    const GLProgram prog = GLProgram::createFromFiles("vertex.glsl", "fragment.glsl");
    const GLint mvpLocation = prog.getUniformLocation("MVP");
    const GLint mLocation = prog.getUniformLocation("M");
    const GLint mitLocation = prog.getUniformLocation("Mit");
    const GLint invVLocation = prog.getUniformLocation("invV");
    const GLint posLocation = prog.getAttributeLocation("vPos");
    const GLint tcLocation = prog.getAttributeLocation("vTc");
    const GLint normLocation = prog.getAttributeLocation("vNorm");
    const GLint lpLocation  = prog.getUniformLocation("vLightPos");
    const GLint texLocation  = prog.getUniformLocation("textureSampler"); 
    const GLint texLocation2  = prog.getUniformLocation("textureSampler2");  
    
    vbPos.connectVertexAttrib(posLocation, 3);
    vbNorm.connectVertexAttrib(normLocation, 3);
    vbTc.connectVertexAttrib(tcLocation, 2);
   
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
    
        const Mat4 m{Mat4::translation({0.0f,0.0f,0.2f})*Mat4::rotationX(glfwGetTime()*57)*Mat4::translation({0.2f,0.0f,0.0f})*Mat4::rotationY(glfwGetTime()*177)};
        const Mat4 v{Mat4::lookAt({0,0,2},{0,0,0},{0,1,0})};
        const Mat4 p{Mat4::perspective(90, dim.aspect(), 0.0001, 100)};
        const Mat4 mvp{m*v*p};    
        prog.enable();
        prog.setUniform(mvpLocation, mvp);
        prog.setUniform(mLocation, m);
        prog.setUniform(mitLocation, Mat4::inverse(m), true);
        prog.setUniform(invVLocation, Mat4::inverse(v));
        prog.setUniform(lpLocation, {0,1,1});
        prog.setTexture(texLocation,checkerTex,0);
        prog.setTexture(texLocation2,checkerTex2,1);
                
        glDrawElements(GL_TRIANGLES, sphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
