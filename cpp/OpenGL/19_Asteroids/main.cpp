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
  GameObject(const Vec2& position) :
    position(position)
  {
  }
  
  void draw(GLApp& app) {
    drawInt(app, shape, colors);
  }

  void animate(float deltaT) {
    position = position + velocity*deltaT;
    velocity = velocity * (1.0f-resistance);
    
    if (position.x() < -200 || position.x() > 200) position = Vec2(position.x()*-1.0f, position.y());
    if (position.y() < -200 || position.y() > 200) position = Vec2(position.x(), position.y()*-1.0f);
  }

  bool collision(const Vec2& pos) const {
    Vec4 hPos{pos.x(), pos.y(), 0.0f, 1.0f};
    hPos = Mat4::inverse(getTransform()) * hPos;
    return Collision::pointInPolygon(Vec2(hPos.x(),hPos.y()), shape);
  }

  void rotate(float angle) {
    rotation += angle;
  }

  void accelerate(float thrust) {
    Vec4 direction = Mat4::rotationZ(rotation) * Vec4{0.0f,1.0f,0.0,0.0};
    velocity = velocity + direction.xy() * thrust;
  }
  
  bool collision(const GameObject& pos) const {
    return Collision::polygonPolygonCollision(getTransformedShape(), pos.getTransformedShape());
  }
  
  Mat4 getTransform() const {
    return Mat4::rotationZ(rotation)*Mat4::translation(position.x(),position.y(),0.0f)*Mat4::scaling(0.005f);
  }
  
  Vec2 getPosition() const {
    return position;
  }

  Vec2 getVelocity() const {
    return velocity;
  }
  
  float getRotation() const {
    return rotation;
  }

protected:
  Vec2 position;
  Vec2 velocity{0.0f,0.0f};
  float resistance{0.0f};
  
  float rotation{0.0f};
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
  
  void drawInt(GLApp& app, std::vector<Vec2>& pData, std::vector<Vec4>& cData) {
    std::vector<float> glShape;
    
    for (size_t i = 0;i<pData.size();++i) {
      glShape.push_back(pData[i].x());
      glShape.push_back(pData[i].y());
      glShape.push_back(0.0f);

      glShape.push_back(cData[i].x());
      glShape.push_back(cData[i].y());
      glShape.push_back(cData[i].z());
      glShape.push_back(cData[i].w());
    }
    
    app.setDrawTransform(getTransform());
    app.drawLines(glShape, LineDrawType::LD_LOOP);
  }
  
};



class Projectile : public GameObject {
public:
  Projectile() :
    GameObject({0.0f,0.0f})
  {
    addCoord({0, 10});
    addCoord({0, 8});
  }
  
  bool isValid() const {
    return remainingLife > 0.0f;
  }
  
  void invalidate() {
    remainingLife = 0.0f;
  }
  
  void fire(const Vec2& startPos, float rotation) {
    this->remainingLife = 200;
    this->position = startPos;
    this->rotation = rotation;
    this->velocity = (Mat4::rotationZ(rotation) * Vec4{0.0f,projectileVelocity,0.0,0.0}).xy();
  }
  
  void animate(float deltaT) {
    if (isValid()) {
      remainingLife -= deltaT;
      GameObject::animate(deltaT);
    } else {
      remainingLife = 0.0f;
    }
  }

  void draw(GLApp& app) {
    if (isValid()) {
      GameObject::draw(app);
    }
  }
  
  Vec2 getTransformedStart() const {
    return getTransformedShape()[0];
  }
  
  Vec2 getTransformedEnd() const {
    return getTransformedShape()[1];
  }
  
private:
  float remainingLife{0.0f};
  const float projectileVelocity{2.0f};
};

class Asteroid : public GameObject {
public:
  Asteroid(const Vec2& position, const Vec2& startVelocity, uint32_t type=3, float size = 3.0f, size_t vertexCount=12) :
    GameObject(position),
    type(type)
  {
    for (size_t i = 0;i<vertexCount;++i) {
      float angle = 2*PI*i/float(vertexCount);
      float dist = (Rand::rand01() < 0.4)? 0.5f+Rand::rand01()*0.5f : 0.9f+Rand::rand01()*0.1f;
      const Vec2 coord = Vec2{type*size*cosf(angle), type*size*sinf(angle)} * dist;
      addCoord( coord );
            
      velocity = startVelocity;
    }
  }
  
  bool isHitByLaser(const Projectile& projectile) const {
    return Collision::linePolygonIntersect(projectile.getTransformedStart(), projectile.getTransformedEnd(), getTransformedShape());
  }
  
  uint32_t getType() {
    return type;
  }
    
private:
  uint32_t type;
  
  
};

class Ship : public GameObject {
public:
  Ship(const Vec2& position) :
    GameObject(position)
  {
    addCoord({2, -5});
    addCoord({5, -10});
    addCoord({0, 8});
    addCoord({-5,-10});
    addCoord({-2, -5});
    resistance = 0.0005f;
  }
  
  void draw(GLApp& app, bool drawExhaust) {
    // TODO: fix this
    static uint8_t toggle = 0;
    toggle = (toggle+1)%3;

    if (drawExhaust && toggle==0) {
      std::vector<Vec2> exhaustShape{Vec2{-2,-5}, Vec2{0,-9}, Vec2{2,-5}};
      std::vector<Vec4> exhaustColors{Vec4{1,1,1,1}, Vec4{1,1,1,1}, Vec4{1,1,1,1}};
      drawInt(app, exhaustShape, exhaustColors);
    }
    
    GameObject::draw(app);
  }

};

class MyGLApp : public GLApp {
public:
  const uint32_t initialLives = 3;
  const size_t initalAsteroidCount = 4;
  const size_t maxProjectileCount = 30;
  const float rotationVelocity = 3.0f;
  const float animationVelocity = 1.0f;
  const float thrusterVelocity = 0.015f;

  Ship ship{{0.0f,0.0f}};
  std::vector<Asteroid> asteroids;
  std::vector<Projectile> projectiles;
  bool fireEngine{false};
  bool fireLaser{false};
  bool fireCW{false};
  bool fireCCW{false};
  uint32_t lives;
    
  MyGLApp() :
  GLApp(800,800,4, "Asteroids Demo")
  {
  }
  
  virtual void init() {
    glEnv.setTitle("Asteroids Demo");
    GL(glClearColor(0,0,0,0));
    
    resetGame();
    resetSpace();
    resetShip(true);
  }
  
  virtual void animate(double animationTime) {
    
    if (fireCW) {
      ship.rotate(rotationVelocity);
    }
    
    if (fireCCW) {
      ship.rotate(-rotationVelocity);
    }

    if (fireEngine) {
      ship.accelerate(thrusterVelocity);
    }
    
    if (fireLaser) {
      for (auto& projectile : projectiles) {
        if (!projectile.isValid()) {
          projectile.fire(ship.getPosition(), ship.getRotation());
          fireLaser = false;
          break;
        }
      }
    }

    for (auto& asteroid : asteroids) {
      asteroid.animate(animationVelocity);
    }
    
    for (auto& projectile : projectiles) {
      projectile.animate(animationVelocity);
    }

    ship.animate(animationVelocity);
    
    bool shipCollision = false;
    for (auto& asteroid : asteroids) {
      if (asteroid.collision(ship)) {
        shipCollision = true;
      }
    }
    
    if (shipCollision) {
      shipWrecked();
    }

    for (auto& projectile : projectiles) {
      if (projectile.isValid()) {

        for (size_t i = 0;i<asteroids.size();++i) {
          if (asteroids[i].isHitByLaser(projectile)) {
            
            float direction = Rand::rand01()*360.0f;
            float velocity = 0.2f+Rand::rand01()*0.2f;
            const Vec2 randomVelocity{velocity*cosf(direction), velocity*sinf(direction)};
            
            if (asteroids[i].getType() > 1) {
              asteroids.push_back(Asteroid{asteroids[i].getPosition(),
                                           asteroids[i].getVelocity() + randomVelocity,
                                           asteroids[i].getType()-1});
              asteroids.push_back(Asteroid{asteroids[i].getPosition(),
                                           asteroids[i].getVelocity() - randomVelocity,
                                           asteroids[i].getType()-1});
            }
            projectile.invalidate();
            asteroids.erase(asteroids.begin() + i);
            break;
          }
        }
      }
    }
  }
   
  void resetGame() {
    lives = initialLives;
  }
  
  void resetSpace() {
    projectiles.clear();
    asteroids.clear();
    
    for (size_t i = 0; i < maxProjectileCount; ++i) {
      projectiles.push_back({});
    }

    for (size_t i = 0; i < initalAsteroidCount; ++i) {
      float startAngle = Rand::rand01()*360.0f;
      float startDist = 80.0f+Rand::rand01()*20.0f;
      
      const Vec2 startPos{startDist*cosf(startAngle), startDist*sinf(startAngle)};
      const Vec2 startVelocity{Rand::rand11()*0.5f,Rand::rand11()*0.5f};
      asteroids.push_back(Asteroid{startPos, startVelocity, 3});
    }
  }
  
  void resetShip(bool startCenter) {
    if (startCenter) {
      ship = Ship({0.0f,0.0f});
    } else {
      Vec2 startPos;
      bool isTooClose = false;
      do {
        startPos = Vec2{Rand::rand11()*100.0f,Rand::rand11()*100.0f};
        for (auto& asteroid : asteroids) {
          if ((asteroid.getPosition() - startPos).length() < 10) {
            isTooClose = true;
          }
        }
      } while (isTooClose);
      ship = Ship(startPos);
    }
  }
  
  void shipWrecked() {
    lives--;
    resetShip(lives == 0);
    if (lives == 0) {
      resetGame();
      resetSpace();
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
  
  virtual void keyboard(int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
      closeWindow();
    }
    
    switch (key) {
      case GLFW_KEY_ESCAPE:
        closeWindow();
        break;
      case GLFW_KEY_UP:
      case GLFW_KEY_DOWN:
      case GLFW_KEY_W:
        fireEngine = action != GLFW_RELEASE;
        break;
      case GLFW_KEY_SPACE:
        fireLaser = action != GLFW_RELEASE;
        break;
      case GLFW_KEY_A:
      case GLFW_KEY_LEFT:
        fireCCW = action != GLFW_RELEASE;
        break;
      case GLFW_KEY_D:
      case GLFW_KEY_RIGHT:
        fireCW = action != GLFW_RELEASE;
        break;
    }
  }
  
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

    for (auto& asteroid : asteroids) {
      asteroid.draw(*this);
    }

    for (auto& projectile : projectiles) {
      projectile.draw(*this);
    }

    ship.draw(*this, fireEngine);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
