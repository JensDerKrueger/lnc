#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <sstream>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include <Vec3.h>
#include <Vec2.h>
#include <Mat4.h>
#include <bmp.h>

#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"
#include "Grid2D.h"

#include <exception>

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}

void pushVertex(const Vec3& v, std::vector<float>& a ) {
    a.push_back(v.x());
    a.push_back(v.y());
    a.push_back(v.z());
}

void pushValue(size_t x,size_t y, const Grid2D& heightField, std::vector<float>& tris) {
//    Vec3 v{float(x-heightField.getWidth()/2),heightField.getValue(x,y),float(y-heightField.getHeight()/2)};
    Vec3 v{float(x)/heightField.getWidth()-0.5f,heightField.getValue(x,y)-0.5f,float(y)/heightField.getHeight()-0.5f};

    pushVertex(v, tris);
    pushVertex(v, tris);
}

const std::vector<float> convertHeightFieldToTriangles(const Grid2D& heightField) {
    std::vector<float> tris;
    
    for (size_t y = 0;y<heightField.getHeight()-1;++y) {
        for (size_t x = 0;x<heightField.getWidth()-1;++x) {
            pushValue(x,y+1,heightField,tris);
            pushValue(x+1,y,heightField,tris);
            pushValue(x,y,heightField,tris);

            pushValue(x,y+1,heightField,tris);
            pushValue(x+1,y+1,heightField,tris);
            pushValue(x+1,y,heightField,tris);
        }
    }
    
    return tris;
}

int main(int argc, char ** argv) {

    const size_t octaves = 7;
    const Vec2t<size_t> startRes{512, 512};
    Grid2D heightField{startRes.x(),startRes.y()};
    for (size_t octave = 0;octave<octaves;++octave) {
        const Vec2 currentRes = startRes / (1<<octave);
        Grid2D currentGrid = Grid2D::genRandom(currentRes.x(), currentRes.y());
        heightField = heightField + currentGrid/(1<<(octaves-octave));
    }

    GLEnv gl{640,480,4,"Terrain Generator", true, true, 4, 1, true};

    std::string vsString{
    "#version 410\n"
    "uniform mat4 MVP;\n"
    "uniform mat4 itMV;\n"
    "in vec3 vPos;\n"
    "in vec3 vNormal;\n"
    "in vec4 vColor;\n"
    "out vec4 color;"
    "out vec3 normal;"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos/vec3(1.0,4.0,1.0), 1.0);\n"
    "    normal = (itMV * vec4(vNormal, 0.0)).xyz;\n"
    "    color = vColor;\n"
    "}\n"};

    std::string fsString{
    "#version 410\n"
    "in vec3 normal;"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(normal,1.0);\n"
    "}\n"};
    
    GLProgram prog{GLProgram::createFromString(vsString, fsString)};
    GLint mvpLocation{prog.getUniformLocation("MVP")};
    GLint mitLocation{prog.getUniformLocation("itMV")};
    
    GLArray terrainArray{};
    GLBuffer vbTerrain{GL_ARRAY_BUFFER};

    const std::vector<float> tris = convertHeightFieldToTriangles(heightField);
    vbTerrain.setData(tris,6,GL_DYNAMIC_DRAW);

    terrainArray.bind();
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vPos", 3);
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vNormal", 3, 3);
    
    gl.setKeyCallback(keyCallback);
    
    glDisable(GL_CULL_FACE);   // TODO: enable
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    Vec3 lookAtVec{0,0,0};
    Vec3 lookFromVec{0,1,1};
    Vec3 upVec{0,1,0};
        
    GLEnv::checkGLError("init");

    glfwSetTime(0);
    
    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        
        const Mat4 p{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 100000.0f)};
        const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
        const Mat4 m{Mat4::rotationY(glfwGetTime()*20)};
        
        prog.enable();
        prog.setUniform(mvpLocation, m*v*p);
        prog.setUniform(mitLocation, Mat4::inverse(m*v), true);
    
        terrainArray.bind();
        glDrawArrays(GL_TRIANGLES, 0, tris.size()/6 );

        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
