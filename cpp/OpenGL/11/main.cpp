#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include <Vec3.h>
#include <Mat4.h>
#include <Rand.h>

#include <GLProgram.h>
#include <GLBuffer.h>
#include <GLArray.h>


static std::string vsString{
"#version 410\n"
"uniform mat4 MVP;\n"
"layout (location = 0) in vec3 vPos;\n"
"void main() {\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"}\n"};

static std::string fsString{
"#version 410\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(1.0,1.0,1.0,1.0);\n"
"}\n"};

std::vector<Vec3> fixedParticles{};
const float radius = 0.001f;
const float colDist = 2*radius;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}

float mindDist(const Vec3& pos) {
    float minDist = (pos - fixedParticles[0]).length();
    for (const Vec3& p : fixedParticles) {
        float currentDist = (pos - p).length();
        if (currentDist < minDist) minDist = currentDist;
    }
    return minDist;
}

bool checkCollision(const Vec3& pos) {
    return mindDist(pos) < colDist;
}

Vec3 randomWalk(const Vec3& pos) {
    return pos + Vec3::randomPointInSphere()*mindDist(pos);
}

Vec3 genRandomStartpoint() {
    Vec3 current{Rand::rand11(),Rand::rand11(),Rand::rand11()};
    while (checkCollision(current))
        current = genRandomStartpoint();
    return current;
}

void initParticles() {
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
        std::cout << "." << std::flush;
    }
}


void checkGLError(const std::string& id) {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        std::cerr << "An openGL error occured:" << e << " at " << id << std::endl;
    }
}

int main(int agrc, char ** argv) {
    initParticles();
    std::cout << "Growing structure ";
    simulate(100);
    std::cout << " Done" << std::endl;

    std::vector<float> floatParticleData(fixedParticles.size()*3);
    for (size_t i = 0;i<fixedParticles.size();++i) {
        floatParticleData[i*3+0] = fixedParticles[i].x();
        floatParticleData[i*3+1] = fixedParticles[i].y();
        floatParticleData[i*3+2] = fixedParticles[i].z();
    }
    
    
    GLEnv gl{640,480,4,"Growy", true, true, 4, 1, true};
        
    GLBuffer vbParticlePositions{GL_ARRAY_BUFFER};
    std::vector<float> empty;
  //  vbParticlePositions.setData(empty,3,GL_DYNAMIC_DRAW);

    GLArray particleArray{};
    GLProgram particleProgram{GLProgram::createFromString(vsString, fsString)};
    GLint mvpLocationParticle{particleProgram.getUniformLocation("MVP")};
    Mat4 m,v,p;

    particleArray.bind();
    particleArray.connectVertexAttrib(vbParticlePositions, particleProgram, "vPos", 3);
        
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

        const Mat4 p{Mat4::perspective(5.0f, dim.aspect(), 0.0001f, 1000.0f)};
        const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
        
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        particleProgram.enable();
        particleProgram.setUniform(mvpLocationParticle, m*v*p);
                    
        glPointSize(10);
        particleArray.bind();
        vbParticlePositions.setData(floatParticleData,3,GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, GLsizei(fixedParticles.size()));
        
        checkGLError("EOF");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
