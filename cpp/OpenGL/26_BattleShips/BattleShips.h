#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <GLApp.h>
#include <GLTexture2D.h>

#include <FontRenderer.h>

#include "GameClient.h"

constexpr uint16_t serverPort = 11003;

class BattleShips : public GLApp {
public:
  BattleShips();
  virtual ~BattleShips();
  
  virtual void init() override;
  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override;
  virtual void keyboardChar(unsigned int codepoint) override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void animate(double animationTime) override ;
  virtual void draw() override;

private:
  std::shared_ptr<GameClient> client{nullptr};

  Vec2 normPos{0,0};
  bool rightMouseDown{false};
  bool leftMouseDown{false};
  Vec2i lastMousePos{-1,-1};
  float wheelScale{100};
  Vec2 startDragPos{0,0};
  double xPositionMouse{ 0.0 };
  double yPositionMouse{0.0};
  Mat4 baseTransformation;
  
  size_t currentImage{0};
  std::array<Image,4> connectingImage;

  bool addressComplete{false};
  bool nameComplete{false};
  std::string serverAddress{""};
  std::string userName{""};
  Image responseImage;
  void tryToLoadSettings();
  
  void updateMousePos();
  
  static FontRenderer fr;

  
};
