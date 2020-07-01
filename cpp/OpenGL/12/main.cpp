#include <iostream>
#include <vector>
#include <stack>
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


struct Target {
    float probability;
    std::string targetString;
};

class Rule {
public:
    Rule(char source, const std::string& target) :
        source(source),
        targets{Target{1.0f,target}}
    {}

    Rule(char source, const std::vector<Target>& targets) :
        source(source),
        targets{targets}
    {
        normalize();
    }

    std::string getTarget() const {
        float p = Rand::rand01();
        
        for (const Target& t : targets) {
            p -= t.probability;
            if (p <= 0)
                return t.targetString;
        }
        
        return targets.back().targetString;
    }
    
    char source;
    
private:
    std::vector<Target> targets;
    
    void normalize() {
        float sum = 0.0f;
        for (const Target& t : targets) {
            sum += t.probability;
        }
        for (Target& t : targets) {
            t.probability /= sum;
        }

    }
};

struct DrawState {
    Vec3 pos;
    Vec3 dir;
};

struct Symbol {
    char c;
    size_t iteration;
};

struct Vertex {
    Vec3 pos;
    size_t iteration;
};

std::vector<Symbol> genSymbolVector(const std::string& s) {
    std::vector<Symbol> sv;
    for (const char c : s) {
        sv.push_back(Symbol{c,0});
    }
    return sv;
}

std::vector<Symbol> executeRules(const std::vector<Symbol>& input, const std::vector<Rule>& rules, size_t iteration) {
    
    std::vector<Symbol> result;
    
    for (const Symbol symbol : input) {
        bool found = false;
        for (const Rule& r : rules) {
            if (r.source == symbol.c) {
                std::string targetString = r.getTarget();
                for (const char c : targetString) {
                    result.push_back(Symbol{c, iteration});
                }
                found = true;
                break;
            }
        }
        if (!found) result.push_back(symbol);
    }
    
    return result;
}

const std::vector<Vertex> drawString(const std::vector<Symbol>& s, float angle, const Vec3& startDir) {
    std::vector<Vertex> result;
    std::stack<DrawState> drawStack;
        
    DrawState currentState{{0.0f,0.0f,0.0f}, startDir};
    
    for (const Symbol& symbol : s) {
        switch (symbol.c) {
            case 'G' :
            case 'F' :
                result.push_back(Vertex{currentState.pos, symbol.iteration});
                currentState.pos = currentState.pos + currentState.dir;
                result.push_back(Vertex{currentState.pos, symbol.iteration});
                break;
            case '[' :
                drawStack.push(DrawState{currentState});
                break;
            case ']' :
                currentState = drawStack.top();
                drawStack.pop();
                break;
            case '+' :
                currentState.dir = Mat4::rotationZ(angle)*currentState.dir;
                break;
            case '-' :
                currentState.dir = Mat4::rotationZ(-angle)*currentState.dir;
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
    Vec3 startDir{1.0f,0.0f,0.0f};
 */
/*
    // Sierpinski triangle
    std::string start = "F-G-G";
    std::vector<Rule> rules{Rule{'F',"F-G+F+G-F"},Rule{'G',"GG"}};
    float angle = 120;
    Vec3 startDir{1.0f,0.0f,0.0f};
*/

/*
    // Dragon Curve
    std::string start = "FX";
    std::vector<Rule> rules{Rule{'X',"X+YF+"},Rule{'Y',"-FX-Y"}};
    float angle = 90;
    Vec3 startDir{1.0f,0.0f,0.0f};
*/
    
/*
    // Gosper Curve
    std::string start = "F";
    std::vector<Rule> rules{Rule{'F',"F-G--G+F++FF+G-"},Rule{'G',"+F-GG--G-F++F+G"}};
    float angle = 60;
    Vec3 startDir{1.0f,0.0f,0.0f};
*/
/*
    // Hilbert Curve
    std::string start = "A";
    std::vector<Rule> rules{Rule{'A',"-BF+AFA+FB-"},Rule{'B',"+AF-BFB-FA+"}};
    float angle = 90;
    Vec3 startDir{1.0f,0.0f,0.0f};
*/
/*
   // Fractal Plant
    std::string start = "X";
    std::vector<Rule> rules{Rule{'X',"F+[[X]-X]-F[-FX]+X"},Rule{'F',"FF"}};
    float angle = 25;
    Vec3 startDir{0.0f,1.0f,0.0f};
*/
 
    
    std::string start = "X";
    std::vector<Rule> rules{Rule{'X',std::vector<Target>{Target{0.5f,"F[-FX]FX"},Target{0.5f,"F[+FX]FX"}}}};
    float angle = 15;
    Vec3 startDir{0.0f,1.0f,0.0f};
    
    std::vector<Symbol> target = genSymbolVector(start);
    for (size_t i = 0;i<iterations;++i) {
        target = executeRules(target, rules, i+1);
    }
    
    if (target.empty()) {
        throw LException("Evaluated String should not be empty.");
    }
    
    const std::vector<Vertex> structure = drawString(target, angle, startDir);
    
    if (structure.empty()) {
        return {};
    }
    
    Vec3 minV = structure[0].pos;
    Vec3 maxV = structure[0].pos;
    
    for (const Vertex& p : structure) {
        minV = Vec3::minV(minV, p.pos);
        maxV = Vec3::maxV(maxV, p.pos);
        
    }
    
    const Vec3 sizeV = maxV-minV;
    float maxSize = std::max(sizeV.x(), std::max(sizeV.y(), sizeV.z()));
    
    std::vector<float> data{};
    for (const Vertex& p : structure) {
        data.push_back((p.pos.x()-minV.x()-sizeV.x()/2)/maxSize);
        data.push_back((p.pos.y()-minV.y()-sizeV.y()/2)/maxSize);
        data.push_back((p.pos.z()-minV.z()-sizeV.z()/2)/maxSize);
        
        if (p.iteration) {
            Vec3 color = Vec3::hsvToRgb(Vec3{360.0f * p.iteration/iterations,1.0f,1.0f});
            
            data.push_back(color.r());
            data.push_back(color.g());
            data.push_back(color.b());
            data.push_back(1.0f);
        }
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

    
int main(int argc, char ** argv) {
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
    size_t lastIterations = 1;
    GLsizei vertexCount=0;
    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        const Mat4 p{Mat4::ortho(-1,1,-1/dim.aspect(),1/dim.aspect(),-1,1)};
            
        prog.enable();
        prog.setUniform(mvpLocation, p);
    
        lineArray.bind();
        
        const size_t iterations = size_t(glfwGetTime()) % maxIter;
        
        
        
        if (iterations != lastIterations) {
            const std::vector<float> data{genTree(iterations)};
            vbLinePos.setData(data,7,GL_DYNAMIC_DRAW);
            lastIterations = iterations;

            if (data.size() > 1000000) {
                maxIter = iterations+1;
            }
            vertexCount = GLsizei(data.size()/7);
        }
        
        
        glDrawArrays(GL_LINES, 0, vertexCount );

        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
