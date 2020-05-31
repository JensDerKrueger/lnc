#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>  

#include "bmp.h"
#include "GLEnv.h"
#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLTexture2D.h"
#include "ParticleSystem.h"

#include "Mat4.h"
#include "Tesselation.h"


bool bounce = true;
std::shared_ptr<ParticleSystem> particleSystem = nullptr;
std::vector<Vec3> colors{RANDOM_COLOR,{1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,0,1}};
uint32_t currentColor{0};
std::vector<Vec3> accelerations{Vec3{0,0,0},Vec3{0,-0.005,0},Vec3{0,0.005,0}};
uint32_t currentAcceleration{0};
  
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
        
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        bounce = !bounce;
        particleSystem->setBounce(bounce); 
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        currentColor = (currentColor + 1)%colors.size();
        particleSystem->setColor(colors[currentColor]); 
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        currentAcceleration = (currentAcceleration + 1)%accelerations.size();
        particleSystem->setAcceleration(accelerations[currentAcceleration]); 
    }
               
} 

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 5", true};
    gl.setKeyCallback(keyCallback);

    // generate ball resources and load textures
    Tesselation sphere{Tesselation::genSphere({0,0,0}, 0.4f, 50, 50)};
    GLBuffer vbBallPos{GL_ARRAY_BUFFER};
    vbBallPos.setData(sphere.getVertices(),3);
    GLBuffer vbBallNorm{GL_ARRAY_BUFFER};
    vbBallNorm.setData(sphere.getNormals(),3);
    GLBuffer vbBallTan{GL_ARRAY_BUFFER};
    vbBallTan.setData(sphere.getTangents(),3);
    GLBuffer vbBallTc{GL_ARRAY_BUFFER};
    vbBallTc.setData(sphere.getTexCoords(),2);
    GLBuffer ibBall{GL_ELEMENT_ARRAY_BUFFER};
    ibBall.setData(sphere.getIndices());
    BMP::Image ballAlbedoImage{BMP::load("ballAlbedo.bmp")};
    GLTexture2D ballAlbedo{ballAlbedoImage.width, ballAlbedoImage.height, ballAlbedoImage.componentCount, GL_LINEAR, GL_LINEAR};
    ballAlbedo.setData(ballAlbedoImage.data);
    BMP::Image ballNormalImage{BMP::load("ballNormal.bmp")};
    GLTexture2D ballNormalMap{ballNormalImage.width, ballNormalImage.height, ballNormalImage.componentCount, GL_LINEAR, GL_LINEAR};
    ballNormalMap.setData(ballNormalImage.data);
    
    // generate wall geoemtry (for all 5 walls)
    Tesselation square{Tesselation::genRectangle({0,0,0},4,4)};
    GLBuffer vbWallPos{GL_ARRAY_BUFFER};
    vbWallPos.setData(square.getVertices(),3);
    GLBuffer vbWallNorm{GL_ARRAY_BUFFER};
    vbWallNorm.setData(square.getNormals(),3);
    GLBuffer vbWallTan{GL_ARRAY_BUFFER};
    vbWallTan.setData(square.getTangents(),3);
    GLBuffer vbWallTc{GL_ARRAY_BUFFER};
    vbWallTc.setData(square.getTexCoords(),2);
    GLBuffer ibWall{GL_ELEMENT_ARRAY_BUFFER};
    ibWall.setData(square.getIndices());
    
    // load brick wall textures (sides
    BMP::Image brickWallAlbedoImage{BMP::load("brickWallAlbedo.bmp")};
    GLTexture2D brickWallAlbedo{brickWallAlbedoImage.width, brickWallAlbedoImage.height, brickWallAlbedoImage.componentCount, GL_LINEAR, GL_LINEAR};
    brickWallAlbedo.setData(brickWallAlbedoImage.data);
    BMP::Image brickWallNormalImage{BMP::load("brickWallNormal.bmp")};
    GLTexture2D brickWallNormalMap{brickWallNormalImage.width, brickWallNormalImage.height, brickWallNormalImage.componentCount, GL_LINEAR, GL_LINEAR};
    brickWallNormalMap.setData(brickWallNormalImage.data);

    // load brick wall textures (floor)
    BMP::Image floorAlbedoImage{BMP::load("floorAlbedo.bmp")};
    GLTexture2D floorAlbedo{floorAlbedoImage.width, floorAlbedoImage.height, floorAlbedoImage.componentCount, GL_LINEAR, GL_LINEAR};
    floorAlbedo.setData(floorAlbedoImage.data);
    BMP::Image floorNormalImage{BMP::load("floorNormal.bmp")};
    GLTexture2D floorNormalMap{floorNormalImage.width, floorNormalImage.height, floorNormalImage.componentCount, GL_LINEAR, GL_LINEAR};
    floorNormalMap.setData(floorNormalImage.data);

    // load brick wall textures (ceiling)
    BMP::Image ceilingAlbedoImage{BMP::load("ceilingAlbedo.bmp")};
    GLTexture2D ceilingAlbedo{ceilingAlbedoImage.width, ceilingAlbedoImage.height, ceilingAlbedoImage.componentCount, GL_LINEAR, GL_LINEAR};
    ceilingAlbedo.setData(ceilingAlbedoImage.data);
    BMP::Image ceilingNormalImage{BMP::load("ceilingNormal.bmp")};
    GLTexture2D ceilingNormalMap{ceilingNormalImage.width, ceilingNormalImage.height, ceilingNormalImage.componentCount, GL_LINEAR, GL_LINEAR};
    ceilingNormalMap.setData(ceilingNormalImage.data);
    
    // setup normal mapping shader
    const GLProgram progNormalMap = GLProgram::createFromFile("normalMapVertex.glsl", "normalMapFragment.glsl");
    const GLint mvpLocationNormalMap = progNormalMap.getUniformLocation("MVP");
    const GLint mLocationNormalMap = progNormalMap.getUniformLocation("M");
    const GLint mitLocationNormalMap = progNormalMap.getUniformLocation("Mit");
    const GLint invVLocationNormalMap = progNormalMap.getUniformLocation("invV");
    const GLint posLocationNormalMap = progNormalMap.getAttributeLocation("vPos");
    const GLint tanLocationNormalMap = progNormalMap.getAttributeLocation("vTan");
    const GLint tcLocationNormalMap = progNormalMap.getAttributeLocation("vTc");
    const GLint normLocationNormalMap = progNormalMap.getAttributeLocation("vNorm");
    const GLint lpLocationNormalMap  = progNormalMap.getUniformLocation("vLightPos");
    const GLint texRescaleLocationNormalMap = progNormalMap.getUniformLocation("texRescale");
    const GLint texLocationNormalMap  = progNormalMap.getUniformLocation("textureSampler"); 
    const GLint normMapLocationNormalMap  = progNormalMap.getUniformLocation("normalSampler");  
   
    // setup basic OpenGL states that do not change during the frame
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    
    glClearDepth(1.0f);
    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    
    const Vec3 lookFromVec{0,0,5};
    const Vec3 lookAtVec{0,0,0};
    const Vec3 upVec{0,1,0};
    const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
    
    
    do {
        // setup viewport and clear buffers
        const Dimensions dim{gl.getFramebufferSize()};    
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 100)};

        // animate lightpos
        progNormalMap.enable();

        Vec3 lightPos{Mat4::rotationY(glfwGetTime()*55)*Vec3{0,0,1}};
        progNormalMap.setUniform(lpLocationNormalMap, lightPos);

        // ************* the ball
        
        // setup texures
        progNormalMap.setTexture(normMapLocationNormalMap,ballNormalMap,0);
        progNormalMap.setTexture(texLocationNormalMap,ballAlbedo,1);

        // bind geometry
        vbBallPos.connectVertexAttrib(posLocationNormalMap, 3);
        vbBallNorm.connectVertexAttrib(normLocationNormalMap, 3);
        vbBallTan.connectVertexAttrib(tanLocationNormalMap, 3);
        vbBallTc.connectVertexAttrib(tcLocationNormalMap, 2);
        ibBall.bind();
        
        // setup transformations
        const Mat4 mBall{Mat4::translation({0.0f,0.0f,0.8f})*Mat4::rotationX(glfwGetTime()*157)*Mat4::translation({0.8f,0.0f,0.0f})*Mat4::rotationY(glfwGetTime()*47)};
        progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
        progNormalMap.setUniform(mvpLocationNormalMap, {mBall*v*p});
        progNormalMap.setUniform(mLocationNormalMap, mBall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mBall), true);
        progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
                
        // render geometry
        glDrawElements(GL_TRIANGLES, sphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
        
        
        // ************* the left wall
        
        // setup texures (shader is already active)
        progNormalMap.setTexture(normMapLocationNormalMap,brickWallNormalMap,0);
        progNormalMap.setTexture(texLocationNormalMap,brickWallAlbedo,1);

        // bind geometry
        vbWallPos.connectVertexAttrib(posLocationNormalMap, 3);
        vbWallNorm.connectVertexAttrib(normLocationNormalMap, 3);
        vbWallTan.connectVertexAttrib(tanLocationNormalMap, 3);
        vbWallTc.connectVertexAttrib(tcLocationNormalMap, 2);
        ibWall.bind();

        const Mat4 mLeftWall{Mat4::rotationY(90)*Mat4::translation(-2.0f,0.0f,0.0f)};
        progNormalMap.setUniform(mvpLocationNormalMap, {mLeftWall*v*p});
        progNormalMap.setUniform(mLocationNormalMap, mLeftWall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mLeftWall), true);

        // render geometry
        glDrawElements(GL_TRIANGLES, square.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        // ************* the right wall
        
        const Mat4 mRightWall{Mat4::rotationY(-90)*Mat4::translation(2.0f,0.0f,0.0f)};
        progNormalMap.setUniform(mvpLocationNormalMap, mRightWall*v*p);
        progNormalMap.setUniform(mLocationNormalMap, mRightWall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mRightWall), true);

        // render geometry
        glDrawElements(GL_TRIANGLES, square.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        // ************* the top wall
        
        const Mat4 mTopWall{Mat4::rotationX(90)*Mat4::translation(0.0f,2.0f,0.0f)};
        progNormalMap.setUniform(mvpLocationNormalMap, mTopWall*v*p);
        progNormalMap.setUniform(mLocationNormalMap, mTopWall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mTopWall), true);

        progNormalMap.setTexture(normMapLocationNormalMap,ceilingNormalMap,0);
        progNormalMap.setTexture(texLocationNormalMap,ceilingAlbedo,1);

        // render geometry
        glDrawElements(GL_TRIANGLES, square.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        // ************* the bottom wall
        
        const Mat4 mBottomWall{Mat4::rotationX(-90)*Mat4::translation(0.0f,-2.0f,0.0f)};
        progNormalMap.setUniform(mvpLocationNormalMap, {mBottomWall*v*p});
        progNormalMap.setUniform(mLocationNormalMap, mBottomWall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mBottomWall), true);
        
        progNormalMap.setTexture(normMapLocationNormalMap,floorNormalMap,0);
        progNormalMap.setTexture(texLocationNormalMap,floorAlbedo,1);

        // render geometry
        glDrawElements(GL_TRIANGLES, square.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        // ************* the back wall
        
        const Mat4 mBackWall{Mat4::translation(0.0f,0.0f,-2.0f)};
        progNormalMap.setUniform(mvpLocationNormalMap, {mBackWall*v*p});
        progNormalMap.setUniform(mLocationNormalMap, mBackWall);
        progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(mBackWall), true);

        // render geometry
        glDrawElements(GL_TRIANGLES, square.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

        // ************* particles
        if (!particleSystem)
            particleSystem = std::make_shared<ParticleSystem>(2000, mBall * Vec3(0.0f,0.0f,0.0f), 0.1f, accelerations[currentAcceleration], Vec3{-1.9f,-1.9f,-1.9f}, Vec3{1.9,1.9,1.9}, 200, 100);

        particleSystem->setSize(dim.height/30);
        particleSystem->setCenter(mBall * Vec3(0.0f,0.0f,0.0f));
        
        particleSystem->render(v,p);
        particleSystem->update();

        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
