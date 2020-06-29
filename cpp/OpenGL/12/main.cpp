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

#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"

#include <exception>

class LException : public std::exception {
    public:
        LException(const std::string& whatStr) : whatStr(whatStr) {}
        virtual const char* what() const throw() {
            return whatStr.c_str();
        }
    private:
        std::string whatStr;
};


struct Rule {
    char source;
    std::string target;
};

std::string executeRules(const std::string& s, const std::vector<Rule>& rules) {
    std::string result = "";
    
    for (const char c : s) {
        bool found = false;
        for (const Rule& r : rules) {
            if (r.source == c) {
                result += r.target;
                found = true;
                break;
            }
        }
        if (!found) result += c;
    }
    
    return result;
}

const std::vector<Vec3> drawString(const std::string& s, float angle) {
    std::vector<Vec3> result;
    
    Vec3 pos{0.0f,0.0f,0.0f};
    Vec3 dir{1.0f,0.0f,0.0f};
    
    for (const char c : s) {
        switch (c) {
            case 'G' :
            case 'F' :
                result.push_back(pos);
                pos = pos + dir;
                result.push_back(pos);
                break;
            case '+' :
                dir = Mat4::rotationZ(angle)*dir;
                break;
            case '-' :
                dir = Mat4::rotationZ(-angle)*dir;
                break;
        }
    }
    
    return result;
}

std::vector<float> genTree(size_t iterations) {
/*
    // Koch curve
    std::string start = "F--F--F";
    std::vector<Rule> rules{Rule{'F',"F+F--F+F"}};
    float angle = 60;
 */
/*
    // Sierpinski triangle
    std::string start = "F-G-G";
    std::vector<Rule> rules{Rule{'F',"F-G+F+G-F"},Rule{'G',"GG"}};
    float angle = 120;
*/

/*
    // Dragon Curve
    std::string start = "FX";
    std::vector<Rule> rules{Rule{'X',"X+YF+"},Rule{'Y',"-FX-Y"}};
    float angle = 90;
*/
    
/*
    // Gosper Curve
    std::string start = "F";
    std::vector<Rule> rules{Rule{'F',"F-G--G+F++FF+G-"},Rule{'G',"+F-GG--G-F++F+G"}};
    float angle = 60;
*/

    // Hilbert Curve
    std::string start = "A";
    std::vector<Rule> rules{Rule{'A',"-BF+AFA+FB-"},Rule{'B',"+AF-BFB-FA+"}};
    float angle = 90;

    std::string target = start;
    for (size_t i = 0;i<iterations;++i) {
        target = executeRules(target, rules);
    }
    
    if (target.empty()) {
        throw LException("Evaluated String should not be empty.");
    }
    
    const std::vector<Vec3> structure = drawString(target, angle);
    
    if (structure.empty()) {
        return {};
    }
    
    Vec3 minV = structure[0];
    Vec3 maxV = structure[0];
    
    for (const Vec3& p : structure) {
        minV = Vec3::minV(minV, p);
        maxV = Vec3::maxV(maxV, p);
    }
    
    const Vec3 sizeV = maxV-minV;
    float maxSize = std::max(sizeV.x(), std::max(sizeV.y(), sizeV.z()));
    
    std::vector<float> data{};
    for (const Vec3& p : structure) {
        data.push_back((p.x()-minV.x()-sizeV.x()/2)/maxSize);
        data.push_back((p.y()-minV.y()-sizeV.y()/2)/maxSize);
        data.push_back((p.z()-minV.z()-sizeV.z()/2)/maxSize);
        data.push_back(1.0f);
        data.push_back(1.0f);
        data.push_back(1.0f);
        data.push_back(1.0f);
    }
    
    return data;
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}

    
int main(int agrc, char ** argv) {
    GLEnv gl{640,480,4,"Plant Growth", true, true, 4, 1, true};

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
    
    GLArray lineArray{};
    GLBuffer vbLinePos{GL_ARRAY_BUFFER};

    const std::vector<float> empty{};
    vbLinePos.setData(empty,7,GL_DYNAMIC_DRAW);

    lineArray.bind();
    lineArray.connectVertexAttrib(vbLinePos, prog, "vPos", 3);
    lineArray.connectVertexAttrib(vbLinePos, prog, "vColor", 4, 3);
    
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
        
    GLEnv::checkGLError("init");

    glfwSetTime(0);
    size_t maxIter{1000};
    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        const Mat4 p{Mat4::ortho(-1,1,-1/dim.aspect(),1/dim.aspect(),-1,1)};
            
        prog.enable();
        prog.setUniform(mvpLocation, p);
    
        lineArray.bind();
        
        const size_t iterations = size_t(glfwGetTime()) % maxIter;
        const std::vector<float> data{genTree(iterations)};
        vbLinePos.setData(data,7,GL_DYNAMIC_DRAW);
        
        if (data.size() > 100000) {
            maxIter = iterations+1;
        }
        
        glDrawArrays(GL_LINES, 0, GLsizei(data.size()/7) );

        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
