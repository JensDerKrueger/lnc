#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <Client.h>
#include <GLApp.h>
#include <Rand.h>
#include <Mat4.h>
#include <FontRenderer.h>

#include "../PainterCommon.h"

#ifndef _WIN32
  #include "helvetica_neue.inc"
#else
  Image fontImage = BMP::load("helvetica_neue.bmp");
  std::vector<CharPosition> fontPos = FontRenderer::loadPositions("helvetica_neue.pos");
#endif

struct ClientMouseInfo : public MouseInfo {
  Image image;
};

class MyClient : public Client {
public:
  MyClient(const std::string& address, short port, const std::string& name) :
    Client{address, port, "asdn932lwnmflj23", 5000},
    name{name},
    color{Vec3::hsvToRgb({360*Rand::rand01(),0.5f,1.0f}), 1.0f}
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
        mouseInfos[i].image = FontRenderer::render(name, fontImage, fontPos);
        mouseInfos[i].color = color;
        found = true;
        break;
      }
    }
    if (!found) {
      mouseInfos.push_back({{userID, name, color, {0,0}}, FontRenderer::render(name, fontImage, fontPos)});
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
  
  void initDataFromServer(const Image& serverImage,
                          const std::vector<MouseInfo>& mi) {
    image      = serverImage;
    
    mouseInfos.clear();
    for (const MouseInfo& m : mi) {
      mouseInfos.push_back({{m.id, m.name, m.color, m.pos}, FontRenderer::render(m.name, fontImage, fontPos)});
    }

    initComplete = true;
  }
    
  virtual void handleNewConnection() override {
    NewUserPayload l(name, color);
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
        InitPayload l(message);
        initDataFromServer(l.image, l.mouseInfos);
        break;
      }
      case PayloadType::CanvasUpdatePayload : {
        CanvasUpdatePayload l(message);
        paint(l.color, l.pos);
        break;
      }
      default:
        std::cout << "unknown message " << int(pt) << " received" << std::endl;
        break;
    };
    miMutex.unlock();
  }
  
  void setMousePos(const Vec2& normPos) {
    if (isConnecting() || !initComplete) {
      initComplete = false;
      return;
    }
    
    MousePosPayload m(normPos);
    sendMessage(m.toString());
  }
  
  
  void paint(const Vec2i& pos) {
    if (isConnecting() || !initComplete) {
      initComplete = false;
      return;
    }

    paint(color, pos);
    CanvasUpdatePayload m{color, pos};
    sendMessage(m.toString());
  }
  
  const std::vector<ClientMouseInfo>& getOtherMouseInfos() const {
    if (!rendererLock) {
      throw std::runtime_error("getImage called without lockData");
    }
    return mouseInfos;
  }
    
  const Image& getImage() {
    if (!rendererLock) {
      throw std::runtime_error("getImage called without lockData");
    }

    if (isConnecting() || !initComplete) {
      initComplete = false;
    }
    return image;
  }
  
  void lockData() {
    if (rendererLock) {
      throw std::runtime_error("lockData called repeatedly without unlockData");
    }
    miMutex.lock();
    rendererLock = true;
  }

  void unlockData() {
    if (!rendererLock) {
      throw std::runtime_error("unlockData called without lockData");
    }
    miMutex.unlock();
    rendererLock = false;
  }

  Vec4 getColor() const {
    return color;
  }
  
  bool isValid() const {
    return initComplete;
  }
  
  void setColor(const Vec4& color) {
    this->color = color;
  }
    
  
private:
  bool rendererLock{false};
  std::mutex miMutex;
  std::vector<ClientMouseInfo> mouseInfos;
  std::string name;
  Vec4 color;
  bool initComplete{false};
  Image image{imageWidth,imageHeight};

  
  void paint(const Vec4& color, const Vec2i& pos) {
    if (pos.x() < 0 || uint32_t(pos.x()) >= image.width) return;
    if (pos.y() < 0 || uint32_t(pos.y()) >= image.height) return;
    
    image.setNormalizedValue(pos.x(),pos.y(),0,color.x());
    image.setNormalizedValue(pos.x(),pos.y(),1,color.y());
    image.setNormalizedValue(pos.x(),pos.y(),2,color.z());
    image.setNormalizedValue(pos.x(),pos.y(),3,color.w());
  }

};

class MyGLApp : public GLApp {
public:

  MyGLApp() : GLApp(1024, 786, 4, "Network Painter") {}
  
  Vec3 convertPosToHSV(float x, float y) {
    return Vec3::hsvToRgb({360*x,y,value});
  }

  void fillHSVImage() {
    for (uint32_t y = 0;y<hsvImage.height;++y) {
      for (uint32_t x = 0;x<hsvImage.width;++x) {
        const Vec3 rgb = convertPosToHSV(float(x)/hsvImage.width, float(y)/hsvImage.height);
        hsvImage.setNormalizedValue(x,y,0,rgb.x());
        hsvImage.setNormalizedValue(x,y,1,rgb.y());
        hsvImage.setNormalizedValue(x,y,2,rgb.z());
        hsvImage.setValue(x,y,3,255);
      }
    }
  }
  
  void trytoLoadSettings() {
    std::ifstream settings ("settings.txt");
    std::string line;
    if (settings.is_open()) {
      if (getline(settings,line) ) {
        serverAddress = line;
        addressComplete = true;
      }
        
      if (getline(settings,line) ) {
        userName = line;
        nameComplete = true;
      }

      settings.close();
    }
  }
  
  virtual void init() override {

    trytoLoadSettings();
    
    hsvImage = Image(255,255);
    fillHSVImage();
    
    glEnv.setCursorMode(CursorMode::HIDDEN);
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  }

  void updateMousePos() {
    Dimensions s = glEnv.getWindowSize();
    normPos = Vec2{float(xPositionMouse/s.width)-0.5f,float(1.0-yPositionMouse/s.height)-0.5f} * 2.0f;
    
    if (colorChooserMode)
      normPos = (Mat4::inverse(baseTransformation) * Vec4{normPos,0.0f,1.0f}).xy();
    else
      normPos = (Mat4::inverse(userTransformation*baseTransformation) * Vec4{normPos,0.0f,1.0f}).xy();
  }
  
  void addTransformation(const Mat4& trafo) {
    userTransformation = trafo * userTransformation;
    updateMousePos();
  }
  
  Vec2i computePixelPos() {
    return Vec2i{int((normPos.x()/2.0f+0.5f)*imageSize.x()),int((normPos.y()/2.0f+0.5f)*imageSize.y())};
  }
  
  void dropPaint() {
    if (client) client->paint(computePixelPos());
  }
  
  Mat4 computeBaseTransform(const Vec2ui& imageSize) {
    const Dimensions s = glEnv.getWindowSize();
    const float ax = imageSize.x()/float(s.width);
    const float ay = imageSize.y()/float(s.height);
    const float m = std::max(ax,ay);
    return Mat4::scaling({ax/m, ay/m, 1.0f});
  }
    
  virtual void mouseMove(double xPosition, double yPosition) override {
    if (!client) return;
    
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    
    xPositionMouse = xPosition;
    yPositionMouse = yPosition;
    updateMousePos();

    if (colorChooserMode) {
      if (rightMouseDown) client->setColor( Vec4{Vec3::hsvToRgb({360*(normPos.x()+1.0f)/2.0f,(normPos.y()+1.0f)/2.0f,value}), 1.0f} );
    } else {
    
      if (rightMouseDown) dropPaint();
      
      if (leftMouseDown) {
        const Vec2 trans = normPos - startDragPos;
        addTransformation(Mat4::translation(trans.x(), trans.y(), 0));
        startDragPos = normPos;
      }

      const Vec2i iPos = computePixelPos();
      if (iPos != lastMousePos) {
        client->setMousePos(normPos);
      }
      lastMousePos = iPos;
    }
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (!client) return;
    
    if (colorChooserMode) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        rightMouseDown = (state == GLFW_PRESS);
        if (rightMouseDown) client->setColor( Vec4{Vec3::hsvToRgb({360*(normPos.x()+1.0f)/2.0f,(normPos.y()+1.0f)/2.0f,value}), 1.0f} );
      }
    } else {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        rightMouseDown = (state == GLFW_PRESS);
        if (rightMouseDown) dropPaint();
      }

      if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        leftMouseDown = (state == GLFW_PRESS);
        if (state == GLFW_PRESS) {
          startDragPos = normPos;
        }
      }
    }
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override {
    if (!client) return;
    
    if (colorChooserMode) {
      value = std::clamp<float>(value + float(y_offset)/100, 0.0f, 1.0);
      fillHSVImage();
    } else {
      addTransformation(Mat4::translation(-normPos.x(), -normPos.y(), 0) *
                        Mat4::scaling(1.0f+float(y_offset)/100) *
                        Mat4::translation(normPos.x(), normPos.y(), 0));
    }
  }
  
  virtual void keyboardChar(unsigned int codepoint) override {
    if (!client) {
      if (!addressComplete) {
        serverAddress += char(codepoint);
        responseImage = FontRenderer::render(serverAddress, fontImage, fontPos);
      } else {
        userName += char(codepoint);
        responseImage = FontRenderer::render(userName, fontImage, fontPos);
      }
    }
  }
    
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      
      if (key == GLFW_KEY_ESCAPE) {
        closeWindow();
        return;
      }

      if (!client) {
        std::string& str = addressComplete ? userName : serverAddress;
        bool& complete = addressComplete ? nameComplete : addressComplete;
        switch (key) {
          case GLFW_KEY_BACKSPACE :
            if (str.size() > 0) str.erase(str.size() - 1);
            responseImage = FontRenderer::render(str, fontImage, fontPos);
            break;
          case GLFW_KEY_ENTER :
            if (str.size() > 0) complete = true;
            break;
        }
        return;
      }
            
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
        case GLFW_KEY_R:
          userTransformation = Mat4{};
          updateMousePos();
          break;
        case GLFW_KEY_C:
          colorChooserMode = !colorChooserMode;
          break;
      }
    }
  }
  
  virtual void animate(double animationTime) override {
    currentImage = size_t(animationTime*2) % connectingImage.size();
  }
  
  virtual void draw() override {
    GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
    GL(glClear(GL_COLOR_BUFFER_BIT));
    std::vector<float> glShape;

    if (!client) {
      
      if (nameComplete && addressComplete) {
        connectingImage[0] = FontRenderer::render("Connecting to " + serverAddress, fontImage, fontPos);
        connectingImage[1] = FontRenderer::render("Connecting to " + serverAddress + " .", fontImage, fontPos);
        connectingImage[2] = FontRenderer::render("Connecting to " + serverAddress + " ..", fontImage, fontPos);
        connectingImage[3] = FontRenderer::render("Connecting to " + serverAddress + " ...", fontImage, fontPos);

        client = std::make_shared<MyClient>(serverAddress, 11001, userName);
      } else {
        if (serverAddress.empty()) {
          responseImage = FontRenderer::render("Type in server address:", fontImage, fontPos);
        } else if (addressComplete && userName.empty()) {
          responseImage = FontRenderer::render("Type in your name:", fontImage, fontPos);
        }
        setDrawTransform(Mat4::scaling(1.0f/3.0f) * computeBaseTransform({responseImage.width, responseImage.height}) );
        drawImage(responseImage);
        return;
      }
      
    }
    
    
    if (!client->isValid()) {
      setDrawTransform(Mat4::scaling(connectingImage[currentImage].width / (connectingImage[0].width * 2.0f)) * computeBaseTransform({connectingImage[currentImage].width, connectingImage[currentImage].height}) );
      drawImage(connectingImage[currentImage]);
      return;
    }
    
    if (colorChooserMode) {
      setDrawTransform(Mat4{});
      drawImage(hsvImage);
      
      glShape.clear();
      glShape.push_back(normPos.x()); glShape.push_back(normPos.y()); glShape.push_back(0.0f);
      glShape.push_back(0.0f); glShape.push_back(0.0f); glShape.push_back(0.0f);  glShape.push_back(1.0f);
      drawPoints(glShape, 10, false);

      return;
    }
    
    
    client->lockData();
    imageSize = Vec2ui{client->getImage().width, client->getImage().height};
    client->unlockData();
    baseTransformation = computeBaseTransform(imageSize);
    
    setDrawTransform(userTransformation*baseTransformation);
    setImageFilter(GL_NEAREST,GL_NEAREST);
    client->lockData();
    drawImage(client->getImage());
    const std::vector<ClientMouseInfo> otherMice = client->getOtherMouseInfos();
    for (const ClientMouseInfo& m : otherMice) {
      glShape.push_back(m.pos.x()); glShape.push_back(m.pos.y()); glShape.push_back(0.0f);
      glShape.push_back(m.color.x()); glShape.push_back(m.color.y()); glShape.push_back(m.color.z());  glShape.push_back(m.color.w());
    }
    Vec4 color{client->getColor()};
    drawPoints(glShape, 10, true);
    
    for (const ClientMouseInfo& m : otherMice) {
      setDrawTransform( Mat4::translation(1.0f, 1.0f, 0.0f) * Mat4::scaling(1/10.0f) * computeBaseTransform({m.image.width, m.image.height}) * Mat4::translation(m.pos.x(), m.pos.y(), 0.0f) * userTransformation * baseTransformation );
      drawImage(m.image);
    }
    
    client->unlockData();
    
    glShape.clear();
    glShape.push_back(normPos.x()); glShape.push_back(normPos.y()); glShape.push_back(0.0f);
    glShape.push_back(color.r()); glShape.push_back(color.y()); glShape.push_back(color.z());  glShape.push_back(color.w());
    setDrawTransform(userTransformation*baseTransformation);
    drawPoints(glShape, 40, true);
  }

private:
  std::shared_ptr<MyClient> client{nullptr};
  Vec2 normPos{0,0};
  bool rightMouseDown{false};
  bool leftMouseDown{false};
  Vec2i lastMousePos{-1,-1};

  Vec2 startDragPos{0,0};
  double xPositionMouse{ 0.0 };
  double yPositionMouse{0.0};
  Mat4 baseTransformation;
  Vec2ui imageSize{0,0};
  Mat4 userTransformation;
  
  Image promptImage;
  Image responseImage;
  
  size_t currentImage{0};
  std::array<Image,4> connectingImage;
  Image hsvImage;
  float value{1.0f};
  bool colorChooserMode{false};

  bool addressComplete{false};
  bool nameComplete{false};
  std::string serverAddress{""};
  std::string userName{""};
};

#ifdef _WIN32
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char ** argv) {
#endif
  try {
    MyGLApp myApp;
    myApp.run();
  } catch (const GLException& e) {

    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();

#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
