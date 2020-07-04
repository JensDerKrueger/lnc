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
#include <bmp.h>

#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"

#include <exception>

/*
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}
 */

class Grid2D {
public:
    Grid2D(size_t width, size_t height) :
        width(width),
        height(height),
        data(width*height)
    {
    }
    
    size_t getWidth() const {
        return width;
    }
    
    size_t getHeight() const {
        return height;
    }

    std::string toString() const {
        std::stringstream s;
        for (size_t i = 0;i<data.size();++i) {
            s << data[i];
            if (i%width == width-1 && i != 0)
                s << std::endl;
            else
                s << ", ";
        }
        return s.str();
    }
    
    std::vector<uint8_t> toByteArray() const {
        std::vector<uint8_t> uidata(data.size()*3);
        for (size_t i = 0;i<data.size();++i) {
            uidata[i*3+0] = uint8_t(data[i]*255);
            uidata[i*3+1] = uint8_t(data[i]*255);
            uidata[i*3+2] = uint8_t(data[i]*255);
        }
        return uidata;
    }

    friend std::ostream& operator<<(std::ostream &os, const Grid2D& v) {
        os << v.toString() ; return os;
    }
    
    void setValue(size_t x, size_t y, float value) {
        data[index(x,y)] = value;
    }

    float getValue(size_t x, size_t y) const {
        return data[index(x,y)];
    }
    
    float sample(float x, float y) const {
        // TODO: check value range
        
        float sx = x*(width-1);
        float sy = y*(height-1);
        
        float alpha = sx - floor(sx);
        float beta  = sy - floor(sy);
        
        Vec2ui a{uint32_t(floor(sx)),uint32_t(floor(sy))};
        Vec2ui b{uint32_t(ceil(sx)),uint32_t(floor(sy))};
        Vec2ui c{uint32_t(floor(sx)),uint32_t(ceil(sy))};
        Vec2ui d{uint32_t(ceil(sx)),uint32_t(ceil(sy))};
        
        float va = getValue(a.x(), a.y());
        float vb = getValue(b.x(), b.y());
        float vc = getValue(c.x(), c.y());
        float vd = getValue(d.x(), d.y());
        
        return (va * (1.0f-alpha) + vb * alpha) * (1.0f-beta) + (vc * (1.0f-alpha) + vd * alpha) * beta;
    }
    
    static Grid2D genRandom(size_t x, size_t y) {
        Grid2D result{x,y};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = Rand::rand01();
        }
        return result;
    }
    
    Grid2D operator*(const float& value) const {
        Grid2D result{width,height};
        for (size_t i = 0;i<result.data.size();++i) {
            result.data[i] = data[i]*value;
        }
        return result;
    }
    
    Grid2D operator/(const float& value) const {
        return *this * (1.0f/value);
    }
    
    Grid2D operator+(const Grid2D& other) const {
        // TODO: think about what todo when one
        //       is bigger in one dim and the other
        //       is bigger in the other
                
        if (other.width > width) {
            Grid2D result{other.width,other.height};
            size_t i=0;
            for (size_t y = 0;y<other.height;++y) {
                for (size_t x = 0;x<other.width;++x) {
                    result.data[i] = other.data[i] + sample(x/float(other.width-1.0f),y/float(other.height-1.0f));
                    i++;
                }
            }
            return result;
        } else if (other.width < width) {
            Grid2D result{width,height};
            size_t i=0;
            for (size_t y = 0;y<height;++y) {
                for (size_t x = 0;x<width;++x) {
                    result.data[i] = data[i] + other.sample(x/float(width-1.0f),y/float(height-1.0f));
                    i++;
                }
            }
            return result;
        } else {
            Grid2D result{width,height};
            for (size_t i = 0;i<result.data.size();++i) {
                result.data[i] = data[i]+other.data[i];
            }
            return result;
        }
    }
        
private:
    size_t width;
    size_t height;
    std::vector<float> data{};
    
    size_t index(size_t x, size_t y) const {
        return x + y * width;
    }
};


int main(int argc, char ** argv) {
    Grid2D g0 = Grid2D::genRandom(256, 256);
    Grid2D g1 = Grid2D::genRandom(128, 128);
    Grid2D g2 = Grid2D::genRandom(64, 64);
    Grid2D g3 = Grid2D::genRandom(32, 32);
    Grid2D g4 = Grid2D::genRandom(16, 16);
    Grid2D g5 = Grid2D::genRandom(8, 8);
        
    Grid2D g = g0*1.0f/32.0f+g1*1.0f/16.0f+g2*1.0f/8.0f+g3*1.0f/4.0f+g4/2.0f;

    BMP::save("g0.bmp", g0.getWidth(), g0.getHeight(), g0.toByteArray(), 3);
    BMP::save("g1.bmp", g1.getWidth(), g1.getHeight(), g1.toByteArray(), 3);
    BMP::save("g2.bmp", g2.getWidth(), g2.getHeight(), g2.toByteArray(), 3);
    BMP::save("g3.bmp", g3.getWidth(), g3.getHeight(), g3.toByteArray(), 3);
    BMP::save("g4.bmp", g4.getWidth(), g4.getHeight(), g4.toByteArray(), 3);
    BMP::save("g5.bmp", g5.getWidth(), g5.getHeight(), g5.toByteArray(), 3);
    BMP::save("g.bmp", g.getWidth(), g.getHeight(), g.toByteArray(), 3);

    return 0;
 /*
    GLEnv gl{640,480,4,"Terrain Generator", true, true, 4, 1, true};

    std::string vsString{
    "#version 410\n"
    "uniform mat4 MVP;\n"
    "uniform mat4 itMV;\n"
    "in vec3 vPos;\n"
    "in vec3 vNormal;\n"
    "in vec4 vColor;\n"
    "out vec4 color;"
    "out vec3 normal;"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    normal = (itMV * vec4(vNormal, 0.0)).xyz;\n"
    "    color = vColor;\n"
    "}\n"};

    std::string fsString{
    "#version 410\n"
    "in vec4 color;"
    "in vec3 normal;"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(normal,1.0)*color;\n"
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
  */
    return EXIT_SUCCESS;
}  
