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
  
  virtual ~GameObject() {}
  
  virtual void draw(GLApp& app) {
    if (isAlive()) {
      drawShape(app, shape, colors);
    }
  }

  virtual void animate(double deltaT, double animationTime) {
    if (isAlive()) {
      animateInt(deltaT, animationTime);
    }
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
  
  virtual Mat4 getTransform() const {
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
  
  virtual bool isAlive() const {
    return true;
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
  
  virtual void drawShape(GLApp& app, std::vector<Vec2>& pData, std::vector<Vec4>& cData) {
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
    app.drawLines(glShape, LineDrawType::LOOP);
  }
  
  virtual void animateInt(double deltaT, double animationTime) {
    position = position + velocity*float(deltaT);
    velocity = velocity * float(std::pow(1.0f - resistance, deltaT));
    if (position.x() < -200 || position.x() > 200) position = Vec2(position.x()*-1.0f, position.y());
    if (position.y() < -200 || position.y() > 200) position = Vec2(position.x(), position.y()*-1.0f);
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
  
  virtual bool isAlive() const override {
    return remainingLife > 0.0;
  }
  
  void kill() {
    remainingLife = 0.0;
  }
  
  void fire(const Vec2& startPos, float rotation) {
    this->remainingLife = 130;
    this->position = startPos;
    this->rotation = rotation;
    this->velocity = (Mat4::rotationZ(rotation) * Vec4{0.0f,projectileVelocity,0.0,0.0}).xy();
  }
    
  Vec2 getTransformedStart() const {
    return getTransformedShape()[0];
  }
  
  Vec2 getTransformedEnd() const {
    return getTransformedShape()[1];
  }
  
private:
  double remainingLife{0.0};
  const float projectileVelocity{3.0f};
  
protected:
  virtual void animateInt(double deltaT, double animationTime) override {
    GameObject::animateInt(deltaT, animationTime);
    remainingLife -= deltaT;
  }
  
};


enum class ObjectState {
  NORMAL,
  EXPLODING,
  DEAD
};

class ExplodableObject : public GameObject {
public:
  ExplodableObject(const Vec2& position) :
    GameObject(position)
  {
  }

  void setState(ObjectState objectState) {
    this->objectState = objectState;
  }
  
  ObjectState getState() const {
    return objectState;
  }
private:
  ObjectState objectState{ObjectState::NORMAL};
  
};

class Asteroid : public ExplodableObject {
public:
  Asteroid(const Vec2& position, const Vec2& startVelocity, uint32_t type=3, float size = 3.0f, size_t vertexCount=12) :
    ExplodableObject(position),
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
    return getState() == ObjectState::NORMAL && Collision::linePolygonIntersect(projectile.getTransformedStart(), projectile.getTransformedEnd(), getTransformedShape());
  }
  
  uint32_t getType() {
    return type;
  }
    
  
  virtual void draw(GLApp& app) override {
    std::vector<float> glShape;
    if (getState() == ObjectState::EXPLODING) {
      std::vector<float> glShape;
      const float alpha{ float(1.0 + explosionTime / maxExplosionTime) };
      for (size_t i = 0;i<shape.size();++i) {
        const Vec2 point = shape[i] * alpha;
        
        glShape.push_back(point.x());
        glShape.push_back(point.y());
        glShape.push_back(0.0f);
        
        glShape.push_back(colors[i].x());
        glShape.push_back(colors[i].y());
        glShape.push_back(colors[i].z());
        glShape.push_back(colors[i].w());
      }
      
      app.setDrawTransform(getTransform());
      app.drawPoints(glShape);
    } else {
      GameObject::draw(app);
    }
  }

  
private:
  uint32_t type;
  double maxExplosionTime{100.0};
  double explosionTime{0.0};
  
  virtual void animateInt(double deltaT, double animationTime) override {
    if (getState() == ObjectState::EXPLODING) {
      explosionTime += deltaT;
      if (explosionTime > maxExplosionTime) {
        explosionTime = 0.0;
        setState(ObjectState::DEAD);
      }
    } else {
      GameObject::animateInt(deltaT, animationTime);
    }
  }
};

class Ship : public ExplodableObject {
public:
  Ship() :
    ExplodableObject({0.0f,0.0f}),
    alive(true)
  {
    addCoord({ 2,  -2});
    addCoord({ 5,  -6});
    addCoord({ 0,   7});
    addCoord({-5,  -6});
    addCoord({-2,  -2});
    resistance = 0.0005f;
  }
  
  virtual void draw(GLApp& app) override {
    if (getState() == ObjectState::DEAD) return;
        
    if (getState() == ObjectState::EXPLODING) {
      std::vector<float> glShape;
      const float alpha{ float(1.0 + explosionTime / maxExplosionTime) };

      for (size_t i = 0;i<shape.size();++i) {
        const size_t endIndex = (i+1)%shape.size();
        
        const Vec2 start = shape[i] * alpha;
        const Vec2 end   = shape[endIndex] + (shape[i] * alpha - shape[i]);
        
        glShape.push_back(start.x());
        glShape.push_back(start.y());
        glShape.push_back(0.0f);
        
        glShape.push_back(colors[i].x());
        glShape.push_back(colors[i].y());
        glShape.push_back(colors[i].z());
        glShape.push_back(colors[i].w());

        glShape.push_back(end.x());
        glShape.push_back(end.y());
        glShape.push_back(0.0f);

        glShape.push_back(colors[endIndex].x());
        glShape.push_back(colors[endIndex].y());
        glShape.push_back(colors[endIndex].z());
        glShape.push_back(colors[endIndex].w());

      }
      
      app.setDrawTransform(getTransform());
      app.drawLines(glShape, LineDrawType::LIST);
      
      
    } else {
      GameObject::draw(app);
      if (isAlive()) {
        if (drawExhaust && sparkle) {
          std::vector<Vec2> exhaustShape{Vec2{-2,-3}, Vec2{0,-7}, Vec2{2,-3}};
          std::vector<Vec4> exhaustColors{Vec4{1,1,1,1}, Vec4{1,1,1,1}, Vec4{1,1,1,1}};
          drawShape(app, exhaustShape, exhaustColors);
        }
      }
    }
  }
    
  virtual bool isAlive() const override {
    return alive;
  }
  
  void setDrawExhaust(bool drawExhaust) {
    this->drawExhaust = drawExhaust;
  }
  

protected:
  virtual void animateInt(double deltaT, double animationTime) override {
    if (getState() == ObjectState::EXPLODING) {
      explosionTime += deltaT;
      if (explosionTime > maxExplosionTime) {
        explosionTime = 0.0;
        setState(ObjectState::DEAD);
      }
    } else {
      GameObject::animateInt(deltaT, animationTime);
      if (animationTime-lastSparkleTime > 3) lastSparkleTime = animationTime;
      sparkle = (animationTime-lastSparkleTime > 1);
    }
  }
  
  
private:
  double maxExplosionTime{100.0};
  double explosionTime{0.0};
  bool alive;
  double lastSparkleTime{0};
  bool sparkle{false};
  bool drawExhaust{false};
  
};

class MyGLApp : public GLApp {
public:
  const uint32_t initialLives = 3;
  const size_t initalAsteroidCount = 4;
  const size_t maxProjectileCount = 3;
  const float rotationVelocity = 3.0f;
  const double animationSpeed = 60.0;
  const float thrusterVelocity = 0.015f;

  Ship ship;
  std::vector<Asteroid> asteroids;
  std::vector<Projectile> projectiles;
  bool fireEngine{false};
  bool fireLaser{false};
  bool fireCW{false};
  bool fireCCW{false};
  uint32_t lives{initialLives};
  double lastAnimationTime{-1.0};
    
  MyGLApp() :
  GLApp(800,800,4, "Asteroids Demo", true, false)
  {
  }
  
  virtual void init() override {
    glEnv.setTitle("Asteroids Demo");
    glEnv.setCursorMode(CursorMode::HIDDEN);
    
    GL(glClearColor(0,0,0,0));
    
    resetGame();
    resetSpace();
    resetShip(true);
  }
  
  virtual void animate(double animationTime) override {
    animationTime *= animationSpeed;
    
    if (lastAnimationTime == -1)  lastAnimationTime = animationTime;    
    const double deltaT = animationTime - lastAnimationTime;
    lastAnimationTime = animationTime;
    
    if (ship.isAlive() && ship.getState() == ObjectState::NORMAL) {
      if (fireCW) ship.rotate(float(rotationVelocity*deltaT));
      if (fireCCW) ship.rotate(float(-rotationVelocity*deltaT));
      if (fireEngine) ship.accelerate(float(thrusterVelocity*deltaT));
      if (fireLaser) {
        for (auto& projectile : projectiles) {
          if (!projectile.isAlive()) {
            projectile.fire(ship.getPosition(), ship.getRotation());
            fireLaser = false;
            break;
          }
        }
      }
      
      bool shipCollision = false;
      for (auto& asteroid : asteroids) {
        if (asteroid.getState() == ObjectState::NORMAL && asteroid.collision(ship)) {
          shipCollision = true;
          break;
        }
      }
      
      if (shipCollision) {
        shipWrecked();
      }
    }

    for (size_t i = 0;i<asteroids.size();++i) {
      if (asteroids[i].getState()  == ObjectState::DEAD) asteroids.erase(asteroids.begin() + i);
    }
    
    
    for (auto& asteroid : asteroids) {
      asteroid.animate(deltaT, animationTime);
    }
    for (auto& projectile : projectiles) {
      projectile.animate(deltaT, animationTime);
    }
    
    ship.animate(deltaT, animationTime);
    
    if (ship.getState() == ObjectState::DEAD) {
      bool isTooClose = false;
      for (auto& asteroid : asteroids) {
        if ((asteroid.getPosition()).length() < 50 && asteroid.getState() == ObjectState::NORMAL) {
          isTooClose = true;
        }
      }
      if (!isTooClose) {
        ship = Ship();
      }
    }
    
    for (auto& projectile : projectiles) {
      for (size_t i = 0;i<asteroids.size();++i) {
        if (projectile.isAlive()) {
          if (asteroids[i].isHitByLaser(projectile)) {
            float direction = Rand::rand01()*360.0f;
            float velocity = 0.4f+Rand::rand01()*0.4f;
            const Vec2 randomVelocity{velocity*cosf(direction), velocity*sinf(direction)};
            
            if (asteroids[i].getType() > 1) {
              asteroids.push_back(Asteroid{asteroids[i].getPosition(),
                                           asteroids[i].getVelocity() + randomVelocity,
                                           asteroids[i].getType()-1});
              asteroids.push_back(Asteroid{asteroids[i].getPosition(),
                                           asteroids[i].getVelocity() - randomVelocity,
                                           asteroids[i].getType()-1});
            }
            projectile.kill();
            asteroids[i].setState(ObjectState::EXPLODING);
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
      float startDist = 180.0f+Rand::rand01()*20.0f;
      
      const Vec2 startPos{startDist*cosf(startAngle), startDist*sinf(startAngle)};
      const Vec2 startVelocity{Rand::rand11()*0.5f,Rand::rand11()*0.5f};
      asteroids.push_back(Asteroid{startPos, startVelocity, 3});
    }
  }
  
  void resetShip(bool startCenter) {
    if (startCenter) {
      ship = Ship();
    } else {
      ship.setState(ObjectState::EXPLODING);
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

  
  virtual void keyboard(int key, int scancode, int action, int mods) override {

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
  
  
  virtual void draw() override{
    GL(glClear(GL_COLOR_BUFFER_BIT));

    for (auto& asteroid : asteroids) {
      asteroid.draw(*this);
    }

    for (auto& projectile : projectiles) {
      projectile.draw(*this);
    }

    ship.setDrawExhaust(fireEngine);
    ship.draw(*this);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
  
