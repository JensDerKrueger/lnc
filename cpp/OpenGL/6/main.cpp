#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>  

#include "bmp.h"
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

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 5"};
    
    gl.setKeyCallback(keyCallback);

    Tesselation sphere{Tesselation::genSphere({0,0,0}, 0.7f, 100, 100)};

    GLBuffer vbPos{GL_ARRAY_BUFFER};
    vbPos.setData(sphere.getVertices(),3);

    GLBuffer vbNorm{GL_ARRAY_BUFFER};
    vbNorm.setData(sphere.getNormals(),3);

    GLBuffer vbTan{GL_ARRAY_BUFFER};
    vbTan.setData(sphere.getTangents(),3);

    GLBuffer vbTc{GL_ARRAY_BUFFER};
    vbTc.setData(sphere.getTexCoords(),2);

    GLBuffer ib{GL_ELEMENT_ARRAY_BUFFER};
    ib.setData(sphere.getIndices());
    
    BMP::Image image{BMP::load("albedo.bmp")};
    GLTexture2D lena{image.width, image.height, image.componentCount, GL_LINEAR, GL_LINEAR};
    lena.setData(image.data);

    BMP::Image normals{BMP::load("normal.bmp")};
    GLTexture2D normalMap{normals.width, normals.height, normals.componentCount, GL_LINEAR, GL_LINEAR};
    normalMap.setData(normals.data);
    
    const GLProgram prog = GLProgram::createFromFiles("vertex.glsl", "fragment.glsl");
    const GLint mvpLocation = prog.getUniformLocation("MVP");
    const GLint mLocation = prog.getUniformLocation("M");
    const GLint mitLocation = prog.getUniformLocation("Mit");
    const GLint invVLocation = prog.getUniformLocation("invV");
    const GLint posLocation = prog.getAttributeLocation("vPos");
    const GLint tanLocation = prog.getAttributeLocation("vTan");
    const GLint tcLocation = prog.getAttributeLocation("vTc");
    const GLint normLocation = prog.getAttributeLocation("vNorm");
    const GLint lpLocation  = prog.getUniformLocation("vLightPos");
    const GLint texLocation  = prog.getUniformLocation("textureSampler"); 
    const GLint normMapLocation  = prog.getUniformLocation("normalSampler");  
    
    vbPos.connectVertexAttrib(posLocation, 3);
    vbNorm.connectVertexAttrib(normLocation, 3);
    vbTan.connectVertexAttrib(tanLocation, 3);
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
    
        prog.enable();
        prog.setUniform(lpLocation, {0,0,2});
        prog.setTexture(normMapLocation,normalMap,0);
        prog.setTexture(texLocation,lena,1);

        const Mat4 m{Mat4::translation({0.0f,0.0f,0.2f})*Mat4::rotationX(glfwGetTime()*57)*Mat4::translation({0.2f,0.0f,0.0f})*Mat4::rotationY(glfwGetTime()*17)};
        const Mat4 v{Mat4::lookAt({0,0,2},{0,0,0},{0,1,0})};
        const Mat4 p{Mat4::perspective(90, dim.aspect(), 0.0001, 100)};
        const Mat4 mvp{m*v*p};    
        prog.setUniform(mvpLocation, mvp);
        prog.setUniform(mLocation, m);
        prog.setUniform(mitLocation, Mat4::inverse(m), true);
        prog.setUniform(invVLocation, Mat4::inverse(v));
                

        glDrawElements(GL_TRIANGLES, sphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
