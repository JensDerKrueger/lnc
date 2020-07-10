#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <sstream>
#include <exception>
#include <chrono>
#include <cmath>
typedef std::chrono::high_resolution_clock Clock;

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>
#include <Grid2D.h>

#include <Vec4.h>
#include <Vec3.h>
#include <Vec2.h>
#include <Mat4.h>
#include <bmp.h>
#include <Camera.h>
#include <ParticleSystem.h>

#include <GLTexture1D.h>
#include <GLProgram.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLTexture2D.h>

#include "GradientGenerator.h"


static Camera camera = Camera(/*Position:*/{ 0.0f, 0.5f, 1.0f });

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_W:
                camera.moveFront(true);
                break;
            case GLFW_KEY_S:
                camera.moveBack(true);
                break;
            case GLFW_KEY_A:
                camera.moveLeft(true);
                break;
            case GLFW_KEY_D:
                camera.moveRight(true);
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_W:
                camera.moveFront(false);
                break;
            case GLFW_KEY_S:
                camera.moveBack(false);
                break;
            case GLFW_KEY_A:
                camera.moveLeft(false);
                break;
            case GLFW_KEY_D:
                camera.moveRight(false);
                break;
            default:
                break;
        }
    }
}

static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition)
{
    camera.mouseMove(xPosition, yPosition);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
            camera.enableMouse();
        else if (action == GLFW_RELEASE)
            camera.disableMouse();
    }
}

static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset)
{
}



void pushVertex(const Vec3& v, std::vector<float>& a ) {
    a.push_back(v.x());
    a.push_back(v.y());
    a.push_back(v.z());
}

void pushValue(size_t x,size_t y, const Grid2D& heightField, std::vector<float>& tris) {
    Vec3 v{float(x)/heightField.getWidth()-0.5f,heightField.getValue(x,y),float(y)/heightField.getHeight()-0.5f};
    pushVertex(v, tris);
    
    const Vec3 t1{0.0f,(heightField.getValue(x,y+1) - heightField.getValue(x,y-1))/(2.0f/heightField.getWidth()), 1.0f};
    const Vec3 t2{1.0f,(heightField.getValue(x+1,y) - heightField.getValue(x-1,y))/(2.0f/heightField.getHeight()), 0.0f};

    const Vec3 normal{Vec3::normalize(Vec3::cross(t1,t2))};
    pushVertex(normal, tris);
}

const std::vector<float> convertHeightFieldToTriangles(const Grid2D& heightField) {
    std::vector<float> tris;
    
    for (size_t y = 1;y<heightField.getHeight()-2;++y) {
        for (size_t x = 1;x<heightField.getWidth()-2;++x) {
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
    Grid2D parameterField{Grid2D::fromBMP("param.bmp")};
    
    const size_t maxOctaves = 10;
    const Vec2t<size_t> startRes{size_t(1)<<maxOctaves, size_t(1)<<maxOctaves};
    Grid2D smoothHeightField{startRes.x(),startRes.y()};
    for (size_t octave = 0;octave<maxOctaves;++octave) {
        const Vec2 currentRes = startRes / (size_t(1)<<octave);
        Grid2D currentGrid = Grid2D::genRandom(currentRes.x(), currentRes.y());
        smoothHeightField = smoothHeightField + currentGrid/(powf(2.5f,maxOctaves-octave));
    }
    smoothHeightField.normalize();
    smoothHeightField = smoothHeightField / 4.0f + 0.2f;
    
    const size_t reducedOctaves = 9;
    Grid2D roughHeightField{startRes.x(),startRes.y()};
    for (size_t octave = 0;octave<reducedOctaves;++octave) {
        const Vec2 currentRes = startRes / (size_t(1)<<octave);
        Grid2D currentGrid = Grid2D::genRandom(currentRes.x(), currentRes.y());
        roughHeightField = roughHeightField + currentGrid/(1<<(reducedOctaves-octave));
    }
    roughHeightField.normalize();
    
    const float reduction = 1.5f;
    
    
    std::shared_ptr<Grid2D> heightField = std::make_shared<Grid2D>((smoothHeightField * parameterField + roughHeightField * (parameterField*-1+1))/reduction);
    
    const MaxData maxv{heightField->maxValue()};
    const Vec3 mountainTop{maxv.pos.x()-0.5f,maxv.value+0.0005f,maxv.pos.y()-0.5f};

    GLEnv gl{1024,768,4,"Terrain Generator", true, true, 4, 1, true};

    std::shared_ptr<SphereStart> starter = std::make_shared<SphereStart>(mountainTop, 0.001f);
    ParticleSystem particleSystem{20000, starter,
                                  Vec3{-0.01,0.0,-0.01}, Vec3{0.02,0.04,0.02}, Vec3{0,-0.01, 0},
                                  Vec3{-0.5f,0.0f,-0.5f}, Vec3{0.5f,1.0f,0.5f}, 10, 10, Vec3{1.0f,0.5f,0.0f}, true, heightField};
    
    particleSystem.setBounce(false);

    
    GradientGenerator gend(256);
    
    gend.addColor(0.0f,  Vec4{ 0.09f, 0.27f, 0.63f, 1.0f });   //water
    gend.addColor(0.2f,  Vec4{ 0.09f, 0.27f, 0.63f, 1.0f });   //water
    gend.addColor(0.25f, Vec4{ 0.79f, 0.62f, 0.41f, 0.0f });   //beach
    gend.addColor(0.3f,  Vec4{ 0.79f, 0.62f, 0.41f, 0.0f });   //beach
    gend.addColor(0.35f, Vec4{ 0.23f, 0.50f, 0.17f, 0.0f });  //chlorophyll 1
    gend.addColor(0.50f, Vec4{ 0.11f, 0.38f, 0.08f, 0.0f });  //chlorophyll 2
    gend.addColor(0.6f,  Vec4{ 0.32f, 0.31f, 0.31f, 0.0f });  //rock
    gend.addColor(0.7f,  Vec4{ 0.32f, 0.31f, 0.31f, 0.0f });  //rock
    gend.addColor(0.8f,  Vec4{ 1.00f, 1.00f, 1.00f, 0.5f });  //snow
    gend.addColor(1.0f,  Vec4{ 1.00f, 1.00f, 1.00f, 0.5f });  //snow
    
    GLTexture1D heightTexture = gend.getTexture();

    
    GLProgram prog{GLProgram::createFromFile("terrain-vs.glsl", "terrain-fs.glsl")};
    GLint mvpLocation{prog.getUniformLocation("MVP")};
    GLint mvLocation{prog.getUniformLocation("MV")};
    GLint mvitLocation{prog.getUniformLocation("itMV")};
    GLint lightPosLocation{prog.getUniformLocation("lightPos")};
    GLint reductionLocation{prog.getUniformLocation("reduction")};
    GLint alphaLocation{prog.getUniformLocation("alpha")};
    GLint heightTextureLocation{prog.getUniformLocation("heightSampler")};
    
    prog.enable();
    prog.setTexture(heightTextureLocation,heightTexture);
    
    GLArray terrainArray{};
    GLBuffer vbTerrain{GL_ARRAY_BUFFER};
    
    const std::vector<float> tris = convertHeightFieldToTriangles(*heightField);
    vbTerrain.setData(tris,6,GL_STATIC_DRAW);

    terrainArray.bind();
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vPos", 3);
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vNormal", 3, 3);
    
    GLArray waterArray{};
    GLBuffer vbWater{GL_ARRAY_BUFFER};
    const float waterLevel = 0.2f/reduction;
    const std::vector<float> waterVertices{-0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f,
                                           -0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f,
                                           -0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f};
    vbWater.setData(waterVertices,6,GL_STATIC_DRAW);

    waterArray.bind();
    waterArray.connectVertexAttrib(vbWater, prog, "vPos", 3);
    waterArray.connectVertexAttrib(vbWater, prog, "vNormal", 3, 3);

    gl.setKeyCallback(keyCallback);
    gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
    
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLEnv::checkGLError("init");

    glfwSetTime(0);
        
    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.updatePosition();
        const Mat4 p{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 100000.0f)};
        const Mat4 v = camera.viewMatrix();
        const Mat4 m{};
        
        prog.enable();
        prog.setUniform(mvpLocation, m*v*p);
        prog.setUniform(mvitLocation, Mat4::inverse(m*v), true);
        prog.setUniform(mvLocation, m*v);
        prog.setUniform(lightPosLocation, Vec3{0,0,0});
        prog.setUniform(alphaLocation, 1.0f);
        prog.setUniform(reductionLocation, reduction);


        terrainArray.bind();
        glDrawArrays(GL_TRIANGLES, 0, tris.size()/6 );
      
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glDepthMask(GL_FALSE);
        
        prog.setUniform(alphaLocation, 0.9f);
        prog.setUniform(reductionLocation, 1.0f);
        
        waterArray.bind();
        glDrawArrays(GL_TRIANGLES, 0, waterVertices.size()/6 );

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        particleSystem.render(m*v,p);
        particleSystem.update(glfwGetTime());

        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
