#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <sstream>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <GLEnv.h>

#include <Vec3.h>
#include <Vec2.h>
#include <Mat4.h>
#include <bmp.h>

#include <GLTexture1D.h>
#include <GLProgram.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLTexture2D.h>

#include "Grid2D.h"

#include <exception>

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    } 
}

void pushVertex(const Vec3& v, std::vector<float>& a ) {
    a.push_back(v.x());
    a.push_back(v.y());
    a.push_back(v.z());
}

void pushValue(size_t x,size_t y, const Grid2D& heightField, std::vector<float>& tris) {
    Vec3 v{float(x)/heightField.getWidth()-0.5f,heightField.getValue(x,y),float(y)/heightField.getHeight()-0.5f};
    pushVertex(v, tris);
    
    const Vec3 t1{0.0f,(heightField.getValue(x,y+1) - heightField.getValue(x,y-1))/(2.0f/heightField.getWidth()), 1.0f};
    const Vec3 t2{1.0f,(heightField.getValue(x+1,y) - heightField.getValue(x-1,y))/(2.0f/heightField.getHeight()), 0.0f};

    const Vec3 normal{Vec3::normalize(Vec3::cross(t1,t2))};
    pushVertex(normal, tris);
}

const std::vector<float> convertHeightFieldToTriangles(const Grid2D& heightField) {
    std::vector<float> tris;
    
    for (size_t y = 1;y<heightField.getHeight()-2;++y) {
        for (size_t x = 1;x<heightField.getWidth()-2;++x) {
            pushValue(x,y+1,heightField,tris);
            pushValue(x+1,y,heightField,tris);
            pushValue(x,y,heightField,tris);

            pushValue(x,y+1,heightField,tris);
            pushValue(x+1,y+1,heightField,tris);
            pushValue(x+1,y,heightField,tris);
        }
    }
    
    return tris;
}



class PosColor {
public:
    float pos;
    Vec3 color;
    bool operator<(const PosColor& other) const {
        return pos < other.pos;
    }
};

class GradientGenerator {
public:
    GradientGenerator(size_t texSize) : texSize(texSize) {}

    void addColor(float pos, const Vec3& color) {
        addColor(PosColor{pos, color});
    }

    void addColor(const PosColor& c) {
        colors.push_back(c);
    }
    
    GLTexture1D getTexture() {
        std::sort(colors.begin(), colors.end());
        
        std::vector<GLubyte> textureData(texSize*3);

        if (colors.size() > 0) {
                    
            size_t prevIndex = 0;
            size_t nextIndex = 0;
            for (size_t p = 0;p<texSize;++p) {
                const float normIndex = float(p)/float(texSize-1);
                
                if (normIndex > colors[nextIndex].pos) {
                    prevIndex = nextIndex;
                    nextIndex = nextIndex+1;
                }
                if (normIndex >= colors.back().pos) {
                    prevIndex = colors.size()-1;
                    nextIndex = colors.size()-1;
                }
                            
                const Vec3& prev = colors[prevIndex].color;
                const Vec3& next = colors[nextIndex].color;
                
                                
                float alpha = (prevIndex == nextIndex) ? 0.5f :  (normIndex-colors[prevIndex].pos)/(colors[nextIndex].pos-colors[prevIndex].pos);
                
                const Vec3 curreColor{prev*(1-alpha)+next*alpha};
                textureData[p*3+0] = GLubyte(curreColor.r()*255);
                textureData[p*3+1] = GLubyte(curreColor.g()*255);
                textureData[p*3+2] = GLubyte(curreColor.b()*255);
            }
        }
        
        GLTexture1D texture{GL_LINEAR, GL_LINEAR};
        texture.setData(textureData, texSize, 3);
        return texture;
    }
    
private:
    size_t texSize;
    std::vector<PosColor> colors;
    
};




int main(int argc, char ** argv) {
    const size_t octaves = 9;
    const Vec2t<size_t> startRes{1024, 1024};
    Grid2D heightField{startRes.x(),startRes.y()};
    for (size_t octave = 0;octave<octaves;++octave) {
        const Vec2 currentRes = startRes / (1ll<<octave);
        Grid2D currentGrid = Grid2D::genRandom(currentRes.x(), currentRes.y());
        heightField = heightField + currentGrid/(1<<(octaves-octave));
    }
    
    const float reduction = 3.0f;
    
    heightField.normalize();
    heightField = heightField/reduction;

    
    
    
    GLEnv gl{1024,768,4,"Terrain Generator", true, true, 4, 1, true};

    GradientGenerator gend(256);
    

    gend.addColor(0.0f, Vec3{ 0.09f, 0.27f, 0.63f });   //water
    gend.addColor(0.2f, Vec3{ 0.09f, 0.27f, 0.63f });   //water
    gend.addColor(0.25f, Vec3{ 0.79f, 0.62f, 0.41f });   //beach
    gend.addColor(0.3f, Vec3{ 0.79f, 0.62f, 0.41f });   //beach
    gend.addColor(0.35f, Vec3{ 0.23f, 0.50f, 0.17f });  //chlorophyll 1
    gend.addColor(0.50f, Vec3{ 0.11f, 0.38f, 0.08f });  //chlorophyll 2
    gend.addColor(0.6f, Vec3{ 0.32f, 0.31f, 0.31f });  //rock
    gend.addColor(0.7f, Vec3{ 0.32f, 0.31f, 0.31f });  //rock
    gend.addColor(0.8f, Vec3{ 1.0f, 1.0f, 1.0f });  //snow
    gend.addColor(1.0f, Vec3{ 1.0f, 1.0f, 1.0f });  //snow
    
    GLTexture1D heightTexture = gend.getTexture();

    
    std::string vsString{
    "#version 410\n"
    "uniform mat4 MVP;\n"
    "uniform mat4 MV;\n"
    "uniform mat4 itMV;\n"
    "in vec3 vPos;\n"
    "in vec3 vNormal;\n"
    "in vec4 vColor;\n"
    "out vec4 color;"
    "out vec3 normal;"
    "out vec3 pos;"
    "out float height;"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos/vec3(1.0,1.0,1.0), 1.0);\n"
    "    normal = (itMV * vec4(vNormal, 0.0)).xyz;\n"
    "    height = vPos.y;"
    "    pos = (MV * vec4(vPos, 1.0)).xyz;\n"
    "    color = vColor;\n"
    "}\n"};

    std::string fsString{
    "#version 410\n"
    "uniform vec3 lightPos;\n"
    "uniform sampler1D heightSampler;\n"
    "in vec3 normal;\n"
    "in vec3 pos;\n"
    "in float height;\n"
    "uniform float reduction;\n"
    "uniform float alpha;\n"
        
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    vec3 lightDir = normalize(lightPos-pos);\n"
    "    float diffuse = dot(normalize(normal), lightDir);\n"
    "    vec3 texValue = texture(heightSampler, height*reduction).rgb*diffuse;\n"
    "    FragColor = vec4(texValue,alpha);\n"
    "}\n"};
    
    GLProgram prog{GLProgram::createFromString(vsString, fsString)};
    GLint mvpLocation{prog.getUniformLocation("MVP")};
    GLint mvLocation{prog.getUniformLocation("MV")};
    GLint mvitLocation{prog.getUniformLocation("itMV")};
    GLint lightPosLocation{prog.getUniformLocation("lightPos")};
    GLint reductionLocation{prog.getUniformLocation("reduction")};
    GLint alphaLocation{prog.getUniformLocation("alpha")};
    GLint heightTextureLocation{prog.getUniformLocation("heightSampler")};
    
    prog.enable();
    prog.setTexture(heightTextureLocation,heightTexture);
    
    GLArray terrainArray{};
    GLBuffer vbTerrain{GL_ARRAY_BUFFER};
    
    const std::vector<float> tris = convertHeightFieldToTriangles(heightField);
    vbTerrain.setData(tris,6,GL_STATIC_DRAW);

    terrainArray.bind();
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vPos", 3);
    terrainArray.connectVertexAttrib(vbTerrain, prog, "vNormal", 3, 3);
    
    GLArray waterArray{};
    GLBuffer vbWater{GL_ARRAY_BUFFER};
    const float waterLevel = 0.2f/reduction;
    const std::vector<float> waterVertices{-0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f,
                                           -0.5f,waterLevel,-0.5f, 0.0f,1.0f,0.0f,
                                            0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f,
                                           -0.5f,waterLevel,0.5f, 0.0f,1.0f,0.0f};
    vbWater.setData(waterVertices,6,GL_STATIC_DRAW);

    waterArray.bind();
    waterArray.connectVertexAttrib(vbWater, prog, "vPos", 3);
    waterArray.connectVertexAttrib(vbWater, prog, "vNormal", 3, 3);
    
    
    gl.setKeyCallback(keyCallback);
    
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    Vec3 lookAtVec{0,0.2f,0};
    Vec3 lookFromVec{0,0.8,0.8};
    Vec3 upVec{0,1,0};
        
    GLEnv::checkGLError("init");

    glfwSetTime(0);
        
    do {
        Dimensions dim{gl.getFramebufferSize()};
        glViewport(0, 0, dim.width, dim.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        const Mat4 p{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 100000.0f)};
        const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
        const Mat4 m{Mat4::rotationY(glfwGetTime()*20)};
        
        prog.enable();
        prog.setUniform(mvpLocation, m*v*p);
        prog.setUniform(mvitLocation, Mat4::inverse(m*v), true);
        prog.setUniform(mvLocation, m*v);
        prog.setUniform(lightPosLocation, Vec3{0,0,0});
        prog.setUniform(alphaLocation, 1.0f);
        prog.setUniform(reductionLocation, reduction);


        terrainArray.bind();
        glDrawArrays(GL_TRIANGLES, 0, tris.size()/6 );
      
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glDepthMask(GL_FALSE);
        
        prog.setUniform(alphaLocation, 0.9f);
        prog.setUniform(reductionLocation, 1.0f);

        
        waterArray.bind();
        glDrawArrays(GL_TRIANGLES, 0, waterVertices.size()/6 );

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        
        GLEnv::checkGLError("endOfFrame");
        gl.endOfFrame();
    } while (!gl.shouldClose());  
  
    return EXIT_SUCCESS;
}  
