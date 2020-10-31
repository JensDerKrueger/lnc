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
#include <Vec2.h>
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
    size_t depth;
};


struct Vertex {
    Vec3 pos;
    size_t depth;
};

struct LShape {
    std::string name;
    std::string start;
    std::vector<Rule> rules;
    float angle;
    Vec3 startDir;
};

std::vector<LShape> systems {
    #include "lsystems.inc"
    LShape {"koch curve", "F--F--F", {Rule{'F',"F+F--F+F"}}, 60, {1.0f,0.0f,0.0f}},
    LShape {"sierpinski triangle", "F-G-G", {Rule{'F',"F-G+F+G-F"},Rule{'G',"GG"}}, 120, {1.0f,0.0f,0.0f}},
    LShape {"dragon curve", "FX", {Rule{'X',"X+YF+"},Rule{'Y',"-FX-Y"}}, 90, {1.0f,0.0f,0.0f}},
    LShape {"gosper curve", "F", {Rule{'F',"F-G--G+F++FF+G-"},Rule{'G',"+F-GG--G-F++F+G"}}, 60, {1.0f,0.0f,0.0f}},
    LShape {"hilbert curve", "A", {Rule{'A',"-BF+AFA+FB-"},Rule{'B',"+AF-BFB-FA+"}}, 90, {1.0f,0.0f,0.0f}},
    LShape {"fractal plant", "X", {Rule{'X',"F+[[X]-X]-F[-FX]+X"},Rule{'F',"FF"}}, 25, {0.0f,1.0f,0.0f}},
    LShape {"random plant", "X", {Rule{'X',std::vector<Target>{Target{0.5f,"F[-FX]FX"},Target{0.5f,"F[+FX]FX"}}}}, 15, {0.0f,1.0f,0.0f}}
};

size_t currentSystem = 0;


std::string executeRules(const std::string& input, const std::vector<Rule>& rules) {
    
    std::string result;
    
    for (const char symbol : input) {
        bool found = false;
        for (const Rule& r : rules) {
            if (r.source == symbol) {
                result += r.getTarget();
                found = true;
                break;
            }
        }
        if (!found) result.push_back(symbol);
    }
    
    return result;
}

const std::vector<Vertex> drawString(const std::string& s, float angle, const Vec3& startDir) {
    std::vector<Vertex> result;
    std::stack<DrawState> drawStack;
        
    DrawState currentState{{0.0f,0.0f,0.0f}, startDir, 1};
    
    for (const char symbol : s) {
        switch (symbol) {
            case 'G' :
            case 'F' :
                result.push_back(Vertex{currentState.pos, currentState.depth});
                currentState.pos = currentState.pos + currentState.dir;
                result.push_back(Vertex{currentState.pos, currentState.depth});
                break;
            case '[' :
                drawStack.push(DrawState{currentState});
                break;
            case ']' :
                currentState = drawStack.top();
                drawStack.pop();
                break;
            case '+' :
                currentState.dir = Mat4::rotationZ(-angle)*currentState.dir;
                currentState.depth++;
                break;
            case '-' :
                currentState.dir = Mat4::rotationZ(angle)*currentState.dir;
                currentState.depth++;
                break;
        }
    }
    
    return result;
}

std::vector<float> genTree(size_t iterations, const LShape& currenShape) {

    std::string target = currenShape.start;
    for (size_t i = 0;i<iterations;++i) {
        target = executeRules(target, currenShape.rules);
    }
    
    if (target.empty()) {
        throw LException("Evaluated String should not be empty.");
    }
    
    const std::vector<Vertex> structure = drawString(target, currenShape.angle, currenShape.startDir);
    
    if (structure.empty()) {
        return {};
    }
    
    Vec3 minV = structure[0].pos;
    Vec3 maxV = structure[0].pos;
    size_t maxDepth = 0;
    
    for (const Vertex& p : structure) {
        minV = Vec3::minV(minV, p.pos);
        maxV = Vec3::maxV(maxV, p.pos);
        maxDepth = std::max(maxDepth,p.depth);
    }
    
    const Vec3 sizeV = maxV-minV;
    float maxSize = std::max(sizeV.x(), std::max(sizeV.y(), sizeV.z()));
    
    std::vector<float> data{};
    for (const Vertex& p : structure) {
        data.push_back((p.pos.x()-minV.x()-sizeV.x()/2)/maxSize);
        data.push_back((p.pos.y()-minV.y()-sizeV.y()/2)/maxSize);
        data.push_back((p.pos.z()-minV.z()-sizeV.z()/2)/maxSize);
                
        Vec3 color = Vec3::hsvToRgb(Vec3{360.0f * p.depth/maxDepth,1.0f,1.0f});
        data.push_back(color.r());
        data.push_back(color.g());
        data.push_back(color.b());
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
            case GLFW_KEY_T :
                currentSystem = (currentSystem + 1) % systems.size();
                glfwSetTime(0);
                break;
        }
    } 
}


float scale{1.0f};
Vec2 translation{0.0f,0.0f};
Vec2 lastTranslation{0.0f,0.0f};
bool leftMousePressed = false;
bool rightMousePressed = false;
Vec2 currentPos{0.0f,0.0f};
Vec2 leftStartPos{0.0f,0.0f};
Vec2 rigthStartPos{0.0f,0.0f};
Dimensions dim;
float rotation = 0.0f;
float lastRotation = 0.0f;

void mousePosCallback(GLFWwindow* window, double xpos, double ypos) {
    currentPos = Vec2(xpos/(dim.width/2), 1.0f-ypos/(dim.height/2));
    
    if (leftMousePressed) {
        translation = lastTranslation + currentPos-leftStartPos;
    }
    
    if (rightMousePressed) {
        rotation = lastRotation + 360*(currentPos.x()-rigthStartPos.x());
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        leftStartPos = currentPos;
        leftMousePressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        leftMousePressed = false;
        lastTranslation = translation;
    }
    
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rigthStartPos = currentPos;
        rightMousePressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        rightMousePressed = false;
        lastRotation = rotation;
    }
}

void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    scale = std::min(100.0f,std::max(0.1f,scale+float(yoffset)));
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
    gl.setMouseCallbacks(mousePosCallback, mouseButtonCallback, mouseScrollCallback);
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
    GLsizei vertexCount=0;
    size_t iterations=0;
    size_t lastIterations=1;
    size_t lastSystem=1;
    
    do {
        dim = Dimensions{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        const Mat4 p{Mat4::scaling(scale, scale, scale) * Mat4::rotationZ(rotation)* Mat4::translation(translation.x(), translation.y(), 0) * Mat4::ortho(-1,1,-1/dim.aspect(),1/dim.aspect(),-1,1)};
        
            
        prog.enable();
        prog.setUniform(mvpLocation, p);
    
        lineArray.bind();
        
        if (currentSystem != lastSystem || vertexCount < 10000) {
            iterations = size_t(glfwGetTime()*10);
        }
        
        if (iterations != lastIterations) {
            const std::vector<float> data{genTree(iterations, systems[currentSystem])};
            lastSystem = currentSystem;
            vbLinePos.setData(data,7,GL_DYNAMIC_DRAW);
            lastIterations = iterations;
            vertexCount = GLsizei(data.size()/7);
        }
        
        
        glDrawArrays(GL_LINES, 0, vertexCount );

        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
