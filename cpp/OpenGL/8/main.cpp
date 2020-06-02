#include <iostream>
#include <vector>
#include <string>


#include <GL/glew.h>  
#include <GLFW/glfw3.h>  

#include "Scene.h"

std::unique_ptr<Scene> scene{nullptr};

bool bounce{true};
std::vector<Vec3> colors{RANDOM_COLOR,RAINBOW_COLOR,{1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,0,1}};
uint32_t currentColor{0};
std::vector<Vec3> accelerations{Vec3{0,0,0},Vec3{0,-5,0},Vec3{0,5,0}};
uint32_t currentAcceleration{0};
std::vector<float> ages{0,0.4,1,4,10,20};
uint32_t currentAge{0};
bool showFresnelFrame{false};
bool animate = true;
float t0{0.0f};
float viewAngle{0.0f};
  
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT)
        viewAngle -= 1.0f;

    if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT)
        viewAngle += 1.0f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        showFresnelFrame = !showFresnelFrame;
        scene->setShowFresnelFrame(showFresnelFrame);
    }
        
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        bounce = !bounce;
        scene->setParticleBounce(bounce);
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        currentColor = (currentColor + 1)%colors.size();
        scene->setParticleColors(colors[currentColor]);
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        currentAcceleration = (currentAcceleration + 1)%accelerations.size();
        scene->setParticleAcceleration(accelerations[currentAcceleration]);
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        currentAge = (currentAge + 1)%ages.size();
        scene->setParticleMaxAge(ages[currentAge]);        
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        animate = !animate;
        if (animate) {
            glfwSetTime(t0);
        }
    }
} 

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Interactive Late Night Coding Teil 8", true, true, 4, 1, true};
    gl.setKeyCallback(keyCallback);

    scene = std::make_unique<Scene>();

    // setup basic OpenGL states that do not change during the frame
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
    glfwSetTime(0);    
    
    do {                
        const Vec3 lookFromVec{Mat4::rotationY(viewAngle)*Vec3{0,0,5}};
        const Vec3 lookAtVec{0,0,0};
        const Vec3 upVec{0,1,0};
        const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
        
        // setup viewport and clear buffers
        const Dimensions dim{gl.getFramebufferSize()};    
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 100)};

        scene->render(t0, v, p, dim);

        gl.endOfFrame();        
        float t1 = glfwGetTime();
        if (animate) t0 = t1;
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  