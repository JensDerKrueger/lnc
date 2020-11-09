#include <iostream>
#include <GLApp.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>
#include <Rand.h>

constexpr float PI = 3.14159265358979323846f;

class Collision {
public:
  static bool polygonPolygonCollision(const std::vector<Vec2>& polyA, const std::vector<Vec2>& polyB) {
    for (size_t i = 0;i<polyA.size();++i) {
      const Vec2& start = polyA[i];
      const Vec2& end   = polyA[(i+1)%polyA.size()];
      if (linePolygonIntersect(start, end, polyB) ) return true;
    }
    if (pointInPolygon(polyA[0], polyB)) return true;
    if (pointInPolygon(polyB[0], polyA)) return true;
    return false;
  }
  
  static bool linePolygonIntersect(const Vec2& start, const Vec2& end, const std::vector<Vec2>& poly) {
    for (size_t i = 0;i<poly.size();++i) {
      const Vec2& startP = poly[i];
      const Vec2& endP   = poly[(i+1)%poly.size()];
      if (lineLineIntersect(start, end, startP, endP) ) return true;
    }
    return false;
  }
  
  static bool lineLineIntersect(const Vec2& startA, const Vec2& endA, const Vec2& startB, const Vec2& endB) {
    return (ccw(startA, endA, startB) != ccw(startA, endA, endB)) && (ccw(startB, endB, startA) != ccw(startB, endB, endA));
  }
  
  static bool pointInPolygon(const Vec2& point, const std::vector<Vec2>& poly) {
    bool test = false;
    for (size_t i = 0;i<poly.size();++i) {
      const Vec2& start = poly[i];
      const Vec2& end   = poly[(i+1)%poly.size()];
      if ( (start.y() > point.y()) != (end.y() > point.y()) &&
           (point.x() > start.x() + (end.x()-start.x()) * (point.y()-start.y())/(end.y()-start.y()))) {
        test = !test;
      }
    }
    return test;
  }
  
  static bool ccw(const Vec2& a, const Vec2& b, const Vec2& c) {
    return (c.y()-a.y())*(b.x()-a.x()) > (b.y()-a.y())*(c.x()-a.x());
  }
  
};

class GameObject {
public:
  void draw(GLApp& app) {
    std::vector<float> glShape;
    
    for (size_t i = 0;i<shape.size();++i) {
      glShape.push_back(shape[i].x());
      glShape.push_back(shape[i].y());
      glShape.push_back(0.0f);

      glShape.push_back(colors[i].x());
      glShape.push_back(colors[i].y());
      glShape.push_back(colors[i].z());
      glShape.push_back(colors[i].w());
    }
    
    app.setDrawTransform(getTransform());
    app.drawLines(glShape, LineDrawType::LD_LOOP);
  }

  void animate(float deltaT) {
    position = position + speed*deltaT;
    
    if (position.x() < -200 || position.x() > 200) position = Vec2(position.x()*-1.0f, position.y());
    if (position.y() < -200 || position.y() > 200) position = Vec2(position.x(), position.y()*-1.0f);
  }

  bool collision(const Vec2& pos) const {
    Vec4 hPos{pos.x(), pos.y(), 0.0f, 1.0f};
    hPos = Mat4::inverse(getTransform()) * hPos;
    return Collision::pointInPolygon(Vec2(hPos.x(),hPos.y()), shape);
  }

  
  bool collision(const GameObject& pos) const {
    return Collision::polygonPolygonCollision(getTransformedShape(), pos.getTransformedShape());
  }
  
  Mat4 getTransform() const {
    return Mat4::rotationZ(rotation)*Mat4::translation(position.x(),position.y(),0.0f)*Mat4::scaling(0.005f);
  }
  
protected:
  Vec2 position{0,0};
  Vec2 speed{0,0};
  float rotation{0};
  std::vector<Vec2> shape;
  std::vector<Vec4> colors;

  void addCoord(const Vec2& pos, const Vec4& color={1,1,1,1}) {
    shape.push_back(pos);
    colors.push_back(color);
  }

  std::vector<Vec2> getTransformedShape() const {
    std::vector<Vec2> tShape(shape);
    for (size_t i = 0;i<tShape.size();++i) {
      const Vec4 tPos = getTransform() * Vec4(tShape[i].x(), tShape[i].y(), 0.0f, 1.0f);
      tShape[i] = Vec2(tPos.x(), tPos.y());
    }
    return tShape;
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
      case 2 :
        addCoord({-10, 0});
        addCoord({0,20});
        addCoord({5, 17.5});
        addCoord({0, 5});
        addCoord({30,0});
        addCoord({0, -5});
        addCoord({5, -17.5});
        addCoord({0,-20});
        break;
      case 1 :
        addCoord({0, -5});
        addCoord({5, -10});
        addCoord({0, 10});
        addCoord({-5,-10});
      default:
        addCoord({2, -5});
        addCoord({5, -10});
        addCoord({0, 10});
        addCoord({-5,-10});
        addCoord({-2, -5});
        break;
    }
  }
};

class MyGLApp : public GLApp {
public:
  
  Ship ship;
  std::vector<Asteroid> asteroids;
  
  MyGLApp() :
  GLApp(1024,786,4, "Asteroids Demo")
  {
    
  }
  
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
    
    for (auto& asteroid : asteroids) {
      if (asteroid.collision(ship)) {
        std::cout << "BOOM" << std::endl;
      }
    }

  }
   
  virtual void mouseMove(double xPosition, double yPosition) {
    const Vec2 pos{
      float(2.0*xPosition/glEnv.getFramebufferSize().width-1.0),
      -float(2.0*yPosition/glEnv.getFramebufferSize().height-1.0)
    };
    
    if (ship.collision(pos)) {
      std::cout << "hit ship" << std::endl;
    }
    
    for (auto& asteroid : asteroids) {
      if (asteroid.collision(pos) ) {
        std::cout << "hit asteroid" << std::endl;
      }
    }

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
