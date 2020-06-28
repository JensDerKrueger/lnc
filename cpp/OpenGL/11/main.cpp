#define showOctree
//#define only2D


#include <fstream>
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

#include "Octree.h"

#include <AbstractParticleSystem.h>

class SimpleStaticParticleSystem : public AbstractParticleSystem {
public:
    SimpleStaticParticleSystem(const std::vector<Vec3> data, float pointSize) :
        AbstractParticleSystem(pointSize),
        color{1.0f,1.0f,1.0f}
    {
        setData(data);
    }
    
    virtual void update(float t) {}
    virtual void setColor(const Vec3& color) {this->color = color;}

    void setData(const std::vector<float> data) {
        floatParticleData.resize(data.size()/3*7);
        size_t j = 0;
        for (size_t i = 0;i<data.size();i+=3) {
            floatParticleData[j*7+0] = data[i+0];
            floatParticleData[j*7+1] = data[i+1];
            floatParticleData[j*7+2] = data[i+2];
            
            Vec3 c = color == RAINBOW_COLOR ? Vec3::hsvToRgb({float(i)/data.size()*360,1.0,1.0}) : computeColor(color);
            
            floatParticleData[j*7+3] = c.x();
            floatParticleData[j*7+4] = c.y();
            floatParticleData[j*7+5] = c.z();
            floatParticleData[j*7+6] = 1.0f;
            j+=1;
        }
    }
    
    void setData(const std::vector<Vec3> data) {
        floatParticleData.resize(data.size()*7);
        for (size_t i = 0;i<data.size();++i) {
            floatParticleData[i*7+0] = data[i].x();
            floatParticleData[i*7+1] = data[i].y();
            floatParticleData[i*7+2] = data[i].z();
            
            Vec3 c = color == RAINBOW_COLOR ? Vec3::hsvToRgb({float(i)/data.size()*360,1.0,1.0}) : computeColor(color);
            
            floatParticleData[i*7+3] = c.x();
            floatParticleData[i*7+4] = c.y();
            floatParticleData[i*7+5] = c.z();
            floatParticleData[i*7+6] = 1.0f;
        }
    }
    
    virtual std::vector<float> getData() const {return floatParticleData;}
    virtual size_t getParticleCount() const  {return floatParticleData.size()/7;}
private:
    Vec3 color;
    std::vector<float> floatParticleData;

};


std::vector<Vec3> fixedParticles{};
const float radius = 0.001f;
const float colDist = 2*radius;
const size_t particleCount = 100000;
Octree octree{1.0f, Vec3{0.0f,0.0f,0.0f}, 10};
bool rotation{true};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_R :
                rotation = !rotation;
                break;
        }
    } 
}

float mindDist(const Vec3& pos) {
    return octree.minDist(pos);
}

bool checkCollision(const Vec3& pos) {
    return mindDist(pos) < colDist;
}

Vec3 randomWalk(const Vec3& pos) {
    #ifdef only2D
        return pos + Vec3::randomPointInDisc()*mindDist(pos);
    #else
        return pos + Vec3::randomPointInSphere()*mindDist(pos);
    #endif
}

Vec3 genRandomStartpoint() {
#ifdef only2D
    Vec3 current{Rand::rand11(),Rand::rand11(),0};
#else
    Vec3 current{Rand::rand11(),Rand::rand11(),Rand::rand11()};
#endif
    while (checkCollision(current))
        current = genRandomStartpoint();
    return current;
}

void initParticles() {
    fixedParticles.clear();
    fixedParticles.push_back(Vec3(0.0f,0.0f,0.0f));
}

void simulate(size_t particleCount) {
    for (size_t i = 0;i<particleCount;++i) {
        Vec3 current = genRandomStartpoint();
        while (!checkCollision(current)) {
            current = randomWalk(current);
            if (current.sqlength() > 5*5) {
                current = genRandomStartpoint();
            }
        }
        fixedParticles.push_back(current);
        octree.add(current);
        std::cout << i+1 << "/" << particleCount << "\r" << std::flush;
    }
}


void simulate(size_t maxParticleCount, uint32_t quota) {
    auto t1 = Clock::now();
    
    for (size_t i = fixedParticles.size();i<maxParticleCount;++i) {
        Vec3 current = genRandomStartpoint();
        while (!checkCollision(current)) {
            current = randomWalk(current);
            if (current.sqlength() > 5*5) {
                current = genRandomStartpoint();
            }
        }
        fixedParticles.push_back(current);
        octree.add(current);

        auto t2 = Clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() > quota) {
            std::cout << i+1 << "/" << particleCount << "\r" << std::flush;
            return;
        }
    }
    std::cout << particleCount << "/" << particleCount << "\r" << std::flush;
}

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Dendrite Growth Simulation", true, true, 4, 1, true};

#ifdef showOctree
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
    
    GLArray octreeLineArray{};
    GLBuffer vbOctreeLinePos{GL_ARRAY_BUFFER};
    GLArray octreeFaceArray{};
    GLBuffer vbOctreeFacePos{GL_ARRAY_BUFFER};

    std::vector<float> empty;
    vbOctreeFacePos.setData(empty,7,GL_DYNAMIC_DRAW);
    vbOctreeLinePos.setData(empty,7,GL_DYNAMIC_DRAW);

    
    octreeLineArray.bind();
    octreeLineArray.connectVertexAttrib(vbOctreeLinePos, prog, "vPos", 3);
    octreeLineArray.connectVertexAttrib(vbOctreeLinePos, prog, "vColor", 4, 3);

    octreeFaceArray.bind();
    octreeFaceArray.connectVertexAttrib(vbOctreeFacePos, prog, "vPos", 3);
    octreeFaceArray.connectVertexAttrib(vbOctreeFacePos, prog, "vColor", 4, 3);
#endif

    SimpleStaticParticleSystem simplePS(fixedParticles, 5);
    simplePS.setColor(RAINBOW_COLOR);

    gl.setKeyCallback(keyCallback);
    glfwSetTime(0);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef only2D
    Vec3 lookFromVec{0,0,1};
#else
    Vec3 lookFromVec{0,1,1};
#endif
    Vec3 lookAtVec{0,0,0};
    Vec3 upVec{0,1,0};

    GLEnv::checkGLError("init");
#ifdef showOctree
    size_t trisVertexCount = 0;
    size_t lineVertexCount = 0;
#endif
    do {
                
        if (fixedParticles.size() < particleCount) {
            simulate(particleCount, 5);
#ifdef showOctree
            octreeLineArray.bind();
            std::vector<float> data{octree.toLineList()};
            lineVertexCount = data.size()/7;
            vbOctreeLinePos.setData(data,7,GL_DYNAMIC_DRAW);
            
            octreeFaceArray.bind();
            data = octree.toTriList();
            trisVertexCount = data.size()/7;
            vbOctreeFacePos.setData(data,7,GL_DYNAMIC_DRAW);
#endif
            simplePS.setData(fixedParticles);
        }
        
        Dimensions dim{ gl.getFramebufferSize() };
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!rotation) glfwSetTime(1);

#ifdef only2D
        const Mat4 p{Mat4::perspective(16.0f, dim.aspect(), 0.0001f, 1000.0f)};
        const Mat4 m{};
#else
        const Mat4 p{Mat4::perspective(6.0f, dim.aspect(), 0.0001f, 1000.0f)};
        const Mat4 m{Mat4::rotationY(45*float(glfwGetTime())/2.0f)*Mat4::rotationX(30*float(glfwGetTime())/2.0f)};
#endif
        const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};

        simplePS.setPointSize(dim.height/80.0f);
        simplePS.render(m*v,p);

#ifdef showOctree
        if (fixedParticles.size() < particleCount) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            glDisable(GL_CULL_FACE);
            glDepthMask(GL_FALSE);
            
            octreeFaceArray.bind();
            prog.enable();
            prog.setUniform(mvpLocation, m*v*p);
            glDrawArrays(GL_TRIANGLES, 0, GLsizei(trisVertexCount));

            octreeLineArray.bind();
            glDrawArrays(GL_LINES, 0, GLsizei(lineVertexCount));


            glEnable(GL_CULL_FACE);

            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
        }
#endif
        
        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
