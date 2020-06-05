#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include "Scene.h"
#include "TextRenderer.h"

std::unique_ptr<Scene> scene{nullptr};

bool animate{true};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
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
                scene->rotateCW();
                break;
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
} 

int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"OpenGL Tetris Teil 9", true, true, 4, 1, true};
        
    std::shared_ptr<Renderer> renderer = std::make_shared<TextRenderer>(10,20);    
    Grid grid(renderer);
    scene = std::make_unique<Scene>(grid);
    gl.setKeyCallback(keyCallback);

    // setup basic OpenGL states that do not change during the frame
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
    glfwSetTime(0);    
    float t0{0.0f};
        
    do {
        scene->render();
        gl.endOfFrame();        
        float t1 = glfwGetTime();
        if (animate) t0 = t1;
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  