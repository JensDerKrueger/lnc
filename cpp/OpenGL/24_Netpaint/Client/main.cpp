#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <Server.h>
#include <Client.h>
#include <GLApp.h>

constexpr uint32_t width = 800;
constexpr uint32_t height= 600;

struct MouseInfo {
  size_t id;
  Vec2 pos;
  Vec4 color;
};

class MyClient : public Client {
public:
  MyClient(const std::string& address, short port) : Client(address, port, "asdn932lwnmflj23", 5000) {}

  virtual void handleServerMessage(const std::string& message) override {
    const std::pair<std::string, std::string> data = Coder::decode(message);
            
  }
  
  void setMousePos(const Vec2& normPos) {
    //sendMessage(Coder::encode("mousepos","xx"));
  }
  
  const std::vector<MouseInfo> getOtherMouseInfos() {
    std::vector<MouseInfo> mi;
    miMutex.lock();
    mi = mouseInfos;
    miMutex.unlock();
    return mi;
  }
  
private:
  std::mutex miMutex;
  std::vector<MouseInfo> mouseInfos;
  Image image{width,height};
};

class MyGLApp : public GLApp {
public:
  Image image{width,height};
  Vec2 normPos{};

  MyGLApp(MyClient& client) : GLApp(width,height, 4, "Network Painter"), client(client) {}
  
  
  virtual void init() {
    glEnv.setCursorMode(CursorMode::HIDDEN);
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));

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
  
  virtual void mouseMove(double xPosition, double yPosition) {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;

    normPos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
    client.setMousePos(normPos);
  }
    
  virtual void draw() {
    drawImage(image);
    
    std::vector<float> glShape;
    glShape.push_back(normPos.x() *2.0f-1.0f); glShape.push_back(normPos.y() *2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f);  glShape.push_back(1.0f);
    
    const std::vector<MouseInfo> otherMice = client.getOtherMouseInfos();
    for (const MouseInfo& m : otherMice) {
      glShape.push_back(m.pos.x() *2.0f-1.0f); glShape.push_back(m.pos.y() *2.0f-1.0f); glShape.push_back(0.0f);
      glShape.push_back(m.color.x()); glShape.push_back(m.color.y()); glShape.push_back(m.color.z());  glShape.push_back(m.color.w());
    }
    drawPoints(glShape, 40, true);
  }

private:
  MyClient& client;

};

int main(int argc, char ** argv) {

 if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " ServerIP YourName" << std::endl;
    return EXIT_FAILURE;
  }
  
  MyClient c{argv[1],11001};
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
