#include <iostream>
#include <GLApp.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>
#include <Rand.h>

constexpr float PI = 3.14159265358979323846f;

class GameObject {
public:
  void draw(GLApp& app) {
    app.setDrawTransform(Mat4::rotationZ(rotation)*Mat4::translation(position.x(),position.y(),0.0f)*Mat4::scaling(0.01f,0.01f,0.01f));
    app.drawLines(shape, LineDrawType::LD_LOOP);
  }

  void animate(float deltaT) {
    position = position + speed*deltaT;
    
    if (position.x() < -100 || position.x() > 100) position = Vec2(position.x()*-1.0f, position.y());
    if (position.y() < -100 || position.y() > 100) position = Vec2(position.x(), position.y()*-1.0f);
  }

protected:
  Vec2 position{0,0};
  Vec2 speed{0,0};
  float rotation{0};
  std::vector<float> shape;

  void addCoord(const Vec2& pos, const Vec4& color={1,1,1,1}) {
    shape.push_back(pos.x()); shape.push_back(pos.y()); shape.push_back(0);
    shape.push_back(color.x()); shape.push_back(color.y()); shape.push_back(color.z()); shape.push_back(color.w());
  }

};

class Asteroid : public GameObject{
public:
  Asteroid(float size = 10.0f, size_t vertexCount=12) {
    for (size_t i = 0;i<vertexCount;++i) {
      float angle = 2*PI*i/float(vertexCount);
      float dist = (Rand::rand01() < 0.4)? 0.5f+Rand::rand01()*0.5f : 0.9f+Rand::rand01()*0.1f;
      const Vec2 coord = Vec2{size*cosf(angle), size*sinf(angle)} * dist;
      addCoord( coord );
      
      float startAngle = Rand::rand01()*360.0f;
      float startDist = 30.0f+Rand::rand01()*20.0f;
      
      position = Vec2(startDist*cosf(startAngle), startDist*sinf(startAngle));
      speed = Vec2(Rand::rand11()*0.5f,Rand::rand11()*0.5f);
    }
  }
};

class Ship : public GameObject {
public:
  Ship(size_t design=0) {
    switch (design) {
      case 1 :
        addCoord({-10, 0});
        addCoord({0,20});
        addCoord({5, 17.5});
        addCoord({0, 5});
        addCoord({30,0});
        addCoord({0, -5});
        addCoord({5, -17.5});
        addCoord({0,-20});
        break;
      default:
        addCoord({0, -5});
        addCoord({5, -10});
        addCoord({0, 10});
        addCoord({-5,-10});
        break;
    }
  }
};

class MyGLApp : public GLApp {
public:
  
  Ship ship;
  std::vector<Asteroid> asteroids;
  
  virtual void init() {
    glEnv.setTitle("Asteroids Demo");
    GL(glClearColor(0,0,0,0));
    
    for (size_t i = 0;i<4;++i)
      asteroids.push_back(Asteroid());
  }
  
  virtual void animate(double animationTime) {
    for (auto& asteroid : asteroids) {
      asteroid.animate(1);
    }
    ship.animate(1);
  }
   
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

    for (auto& asteroid : asteroids) {
      asteroid.draw(*this);
    }
    ship.draw(*this);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
