#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <Client.h>
#include <GLApp.h>
#include <Rand.h>

#include "../PainterCommon.h"

constexpr uint32_t width = 800;
constexpr uint32_t height= 600;


class MyClient : public Client {
public:
  MyClient(const std::string& address, short port, const std::string& name) :
  Client(address, port, "asdn932lwnmflj23", 5000),
  name(name)
  {
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb{0,0,0};
        image.setNormalizedValue(x,y,0,rgb.x());
        image.setNormalizedValue(x,y,1,rgb.y());
        image.setNormalizedValue(x,y,2,rgb.z());
        image.setValue(x,y,3,255);
      }
    }
  }

  void moveMouse(uint32_t userID, const Vec2& pos) {
    for (size_t i = 0;i<mouseInfos.size();++i) {
      if (mouseInfos[i].id == userID) {
        mouseInfos[i].pos = pos;
        break;
      }
    }
  }

  void addMouse(uint32_t userID, const std::string& name, const Vec4& color) {
    bool found{false};
    for (size_t i = 0;i<mouseInfos.size();++i) {
      if (mouseInfos[i].id == userID) {
        mouseInfos[i].name = name;
        mouseInfos[i].color = color;
        found = true;
        break;
      }
    }
    if (!found) {
      mouseInfos.push_back({userID, name, color, {0,0}});
    }
  }

  void removeMouse(uint32_t userID) {
    for (size_t i = 0;i<mouseInfos.size();++i) {
      if (mouseInfos[i].id == userID) {
        mouseInfos.erase(mouseInfos.begin()+i);
        break;
      }
    }
  }
  
  void paint(const Vec4& color, const Vec2i& pos) {
    image.setNormalizedValue(pos.x(),pos.y(),0,color.x());
    image.setNormalizedValue(pos.x(),pos.y(),1,color.y());
    image.setNormalizedValue(pos.x(),pos.y(),2,color.z());
    image.setNormalizedValue(pos.x(),pos.y(),3,color.w());
  }
  
  virtual void handleNewConnection() override {
    NewUserPayload l(name, Vec4(1,0,0,1));
    sendMessage(l.toString());
  }

  virtual void handleServerMessage(const std::string& message) override {
    PayloadType pt = identifyString(message);
       
    miMutex.lock();
    switch (pt) {
      case PayloadType::MousePosPayload : {
        MousePosPayload l(message);
        moveMouse(l.userID, l.mousePos);
        break;
      }
      case PayloadType::NewUserPayload  : {
        NewUserPayload l(message);
        addMouse(l.userID, l.name, l.color);
        break;
      }
      case PayloadType::LostUserPayload : {
        LostUserPayload l(message);
        removeMouse(l.userID);
        break;
      }
      case PayloadType::InitPayload : {
        // TODO
        break;
      }
      case PayloadType::CanvasUpdatePayload : {
        CanvasUpdatePayload l(message);
        paint(l.color, l.pos);
        break;
      }
      default: break;
    };
    miMutex.unlock();
  }
  
  void setMousePos(const Vec2& normPos) {
    MousePosPayload m(normPos);
    sendMessage(m.toString());
  }
  
  void paintSelf(const Vec4& color, const Vec2i& pos) {
    paint(color, pos);
    CanvasUpdatePayload m{color, pos};
    sendMessage(m.toString());
  }
  
  const std::vector<MouseInfo> getOtherMouseInfos() {
    std::vector<MouseInfo> mi;
    miMutex.lock();
    mi = mouseInfos;
    miMutex.unlock();
    return mi;
  }
  
  Image image{width,height};

private:
  std::mutex miMutex;
  std::vector<MouseInfo> mouseInfos;
  std::string name;

};

class MyGLApp : public GLApp {
public:
  Vec2 normPos{};

  MyGLApp(MyClient& client) : GLApp(width,height, 4, "Network Painter"), client(client) {}
  
  virtual void init() {
    color = Vec4{Vec3::hsvToRgb({360*Rand::rand01(),1.0f,1.0f}), 1.0f};
    
    glEnv.setCursorMode(CursorMode::HIDDEN);
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
  }
  
  virtual void mouseMove(double xPosition, double yPosition) {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;

    normPos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
    client.setMousePos(normPos);
    client.paintSelf(color, Vec2i{int(normPos.x()*width),int(normPos.y()*height)});
  }
  
    
  virtual void draw() {
    drawImage(client.image);
    
    std::vector<float> glShape;
    glShape.push_back(normPos.x() *2.0f-1.0f); glShape.push_back(normPos.y() *2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(color.r()); glShape.push_back(color.y()); glShape.push_back(color.z());  glShape.push_back(color.w());
    
    const std::vector<MouseInfo> otherMice = client.getOtherMouseInfos();
    for (const MouseInfo& m : otherMice) {
      glShape.push_back(m.pos.x() *2.0f-1.0f); glShape.push_back(m.pos.y() *2.0f-1.0f); glShape.push_back(0.0f);
      glShape.push_back(m.color.x()); glShape.push_back(m.color.y()); glShape.push_back(m.color.z());  glShape.push_back(m.color.w());
    }
    drawPoints(glShape, 40, true);
  }

private:
  MyClient& client;
  Vec4 color;

};

int main(int argc, char ** argv) {

 if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " ServerIP YourName" << std::endl;
    return EXIT_FAILURE;
  }
 
  MyClient c{argv[1], 11001, argv[2]};
  std::cout << "connecting ...";
  while (c.isConnecting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << " Done" << std::endl;
  
  if (c.isOK()) {
    MyGLApp myApp(c);
    myApp.run();
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start client" << std::endl;
    return EXIT_FAILURE;
  }
}
