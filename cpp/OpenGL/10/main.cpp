#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include "Scene.h"
#include "OpenGLRenderer.h"

std::unique_ptr<Scene> scene{nullptr};

bool animate{true};


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_N :
                scene->setShowPreview(!scene->getShowPreview());
                break;
            case GLFW_KEY_P :
                scene->setPause(!scene->getPause());
                break;
            case GLFW_KEY_T:
                scene->setShowTarget(!scene->getShowTarget());
                break;
            case GLFW_KEY_SPACE :
                scene->fullDrop();
                break;
            case GLFW_KEY_LEFT : 
                scene->moveLeft();
                break;
            case GLFW_KEY_RIGHT :
                scene->moveRight();
                break;
            case GLFW_KEY_DOWN :
                scene->advance();
                break;
            case GLFW_KEY_UP :
            case GLFW_KEY_Q :
                scene->rotateCCW();
                break;
            case GLFW_KEY_W :
                scene->rotateCW();
                break;
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
} 

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"OpenGL Tetris Teil 10", true, true, 4, 1, true};
        
    std::shared_ptr<OpenGLRenderer> renderer = std::make_shared<OpenGLRenderer>(10,20);    
    Grid grid(renderer);
    scene = std::make_unique<Scene>(grid);
    gl.setKeyCallback(keyCallback);
        
    glfwSetTime(0);        
    do {
        renderer->setViewport(Dimensions{gl.getFramebufferSize()});        
        if (!scene->render(glfwGetTime())) {
            break;
        }
        gl.endOfFrame();        
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
