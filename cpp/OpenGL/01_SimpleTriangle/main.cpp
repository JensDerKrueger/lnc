#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>  

#include <Mat4.h>

static void errorCallback(int error, const char* description) {  
   std::cerr << "Fatal Error: " << description << " (" << error << ")" << std::endl;
}  
  
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {  
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
} 

int main(int argc, char ** argv) {

    glfwSetErrorCallback(errorCallback);  
    if (!glfwInit()) {  
        return EXIT_FAILURE; 
    }  
  
    glfwWindowHint(GLFW_SAMPLES, 4);
  
    GLFWwindow* window{glfwCreateWindow(640, 480, "Interactive Late Night Coding Teil 1", nullptr, nullptr)};
    if (window == nullptr) {  
        std::cerr << "Failed to open GLFW window." << std::endl; 
        glfwTerminate();  
        return EXIT_FAILURE;
    }  
  
    glfwMakeContextCurrent(window);    
    glfwSetKeyCallback(window, keyCallback);  
  
    GLenum err{glewInit()};
    if (err != GLEW_OK) {  
        std::cerr << "Failed to init GLEW " << glewGetErrorString(err) << std::endl; 
        glfwTerminate();  
        return EXIT_FAILURE;
    }

    std::vector<float> vertices {
        -0.6f, -0.4f, -1.0f, 1.0f, 0.0f, 0.0f,
         0.6f, -0.4f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.6f, -1.0f, 0.0f, 0.0f, 1.0f
    };

    GLuint vertex_buffer{};
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

    const GLchar* vertex_shader_text{
    "#version 110\n"
    "uniform mat4 MVP;\n"
    "attribute vec3 vCol;\n"
    "attribute vec3 vPos;\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = vCol;\n"
    "}\n"};
        
    GLuint vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLchar* fragment_shader_text{
    "#version 110\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n"};    
    GLuint fragment_shader{glCreateShader(GL_FRAGMENT_SHADER)};
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    GLuint program{glCreateProgram()};
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint mvp_location{glGetUniformLocation(program, "MVP")};
    GLint vpos_location{glGetAttribLocation(program, "vPos")};
    GLint vcol_location{glGetAttribLocation(program, "vCol")};

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 4, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)(sizeof(float) * 3));

    do  {  
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio{float(width)/float(height)};
    
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
        const Mat4 mv{Mat4::rotationZ(glfwGetTime()*20)};
        const Mat4 p{Mat4::perspective(90, ratio, 0.0001, 100)};
        const Mat4 mvp{mv * p};
    
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_TRUE, mvp);  // OpenGL uses column-major order -> transpose
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);  
        glfwPollEvents();  
    } while (!glfwWindowShouldClose(window));  
  
    glfwDestroyWindow(window);  
    glfwTerminate();
    return EXIT_SUCCESS;
}  
