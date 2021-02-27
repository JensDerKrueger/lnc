#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <GLApp.h>
#include <GLTexture2D.h>

#include "MyClient.h"

class MyGLApp : public GLApp {
public:
  MyGLApp();
  virtual ~MyGLApp();
  
  virtual void init() override;
  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override;
  virtual void keyboardChar(unsigned int codepoint) override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void animate(double animationTime) override ;
  virtual void draw() override;

private:
  std::shared_ptr<MyClient> client{nullptr};
  std::shared_ptr<GLTexture2D> texture{nullptr};

  Vec2 normPos{0,0};
  bool rightMouseDown{false};
  bool leftMouseDown{false};
  Vec2i lastMousePos{-1,-1};
  std::array<Vec4,10> quickColors;

  float wheelScale{100};
  Vec2 startDragPos{0,0};
  double xPositionMouse{ 0.0 };
  double yPositionMouse{0.0};
  Mat4 baseTransformation;
  Vec2ui imageSize{0,0};
  Mat4 userTransformation;
  
  Image responseImage;  
  size_t currentImage{0};
  std::array<Image,4> connectingImage;
  Image hsvImage;
  Image cursorShape;
  Image cursorHighlight;
  float value{1.0f};
  bool colorChooserMode{false};
  bool showLabel{true};

  bool addressComplete{false};
  bool nameComplete{false};
  std::string serverAddress{""};
  std::string userName{""};
  
  Vec3 convertPosToHSV(float x, float y);
  void fillHSVImage();
  void tryToLoadSettings();
  void updateMousePos();
  void addTransformation(const Mat4& trafo);
  Vec2i computePixelPos();
  void dropPaint();
  
  void genMouseCursor();
};
