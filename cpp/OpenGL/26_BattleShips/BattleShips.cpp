#include "BattleShips.h"

#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer BattleShips::fr{fontImage, fontPos};
#else
  FontRenderer BattleShips::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

BattleShips::BattleShips() : GLApp(1024, 786, 4, "Network Painter") {}

BattleShips::~BattleShips() {
}

void BattleShips::tryToLoadSettings() {
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

void BattleShips::init() {
  tryToLoadSettings();

  glEnv.setCursorMode(CursorMode::HIDDEN);
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
}

void BattleShips::updateMousePos() {
  Dimensions s = glEnv.getWindowSize();
  normPos = Vec2{float(xPositionMouse/s.width)-0.5f,float(1.0-yPositionMouse/s.height)-0.5f} * 2.0f;
}

void BattleShips::mouseMove(double xPosition, double yPosition) {
  if (!client) return;
  
  Dimensions s = glEnv.getWindowSize();
  if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
  
  xPositionMouse = xPosition;
  yPositionMouse = yPosition;
  updateMousePos();

  // if (rightMouseDown) dropPaint();
    
}

void BattleShips::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  if (!client) return;
  
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    rightMouseDown = (state == GLFW_PRESS);
    //if (rightMouseDown) dropPaint();
  }

}

void BattleShips::mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) {
  if (!client) return;
}

void BattleShips::keyboardChar(unsigned int codepoint) {
  if (!client) {
    if (!addressComplete) {
      serverAddress += char(codepoint);
      responseImage = fr.render(serverAddress);
    } else {
      userName += char(codepoint);
      responseImage = fr.render(userName);
    }
  }
}
  
void BattleShips::keyboard(int key, int scancode, int action, int mods) {
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
          responseImage = fr.render(str);
          break;
        case GLFW_KEY_ENTER :
          if (str.size() > 0) complete = true;
          break;
      }
      return;
    }
    
    switch (key) {
      case GLFW_KEY_Q:
        wheelScale /= 1.5f;
        break;
      case GLFW_KEY_W:
        wheelScale *= 1.5f;
        break;
    }
  }
}

Mat4 BattleShips::computeImageTransform(const Vec2ui& imageSize) const {
  const Dimensions s = glEnv.getWindowSize();
  const float ax = imageSize.x()/float(s.width);
  const float ay = imageSize.y()/float(s.height);
  const float m = std::max(ax,ay);
  return Mat4::scaling({ax/m, ay/m, 1.0f});
}

void BattleShips::animate(double animationTime) {
  currentImage = size_t(animationTime*2) % connectingImage.size();
}

void BattleShips::draw() {
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  std::vector<float> glShape;

  if (!client) {
    
    if (nameComplete && addressComplete) {
      connectingImage[0] = fr.render("Connecting to " + serverAddress);
      connectingImage[1] = fr.render("Connecting to " + serverAddress + " .");
      connectingImage[2] = fr.render("Connecting to " + serverAddress + " ..");
      connectingImage[3] = fr.render("Connecting to " + serverAddress + " ...");

      client = std::make_shared<GameClient>(serverAddress, serverPort, userName);
    } else {
      if (serverAddress.empty()) {
        responseImage = fr.render("Type in server address:");
      } else if (addressComplete && userName.empty()) {
        responseImage = fr.render("Type in your name:");
      }
      setDrawTransform(Mat4::scaling(1.0f/3.0f) * computeImageTransform({responseImage.width, responseImage.height}) );
      drawImage(responseImage);
      return;
    }
  }
  
  if (!client->isValid()) {
    setDrawTransform(Mat4::scaling(connectingImage[currentImage].width / (connectingImage[0].width * 2.0f)) * computeImageTransform({connectingImage[currentImage].width, connectingImage[currentImage].height}) );
    drawImage(connectingImage[currentImage]);
    return;
  }

  // TODO Draw
}
