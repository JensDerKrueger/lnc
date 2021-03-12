#pragma once

#include <vector>
#include <mutex>
#include <deque>

#include <GLApp.h>
#include <Client.h>
#include <GLTexture2D.h>
#include <FontRenderer.h>

class BitMatcher;

class OverlayImage {
public:
  OverlayImage(const std::string& name, uint8_t current, uint8_t next, const std::string& opText, BitMatcher* app);
  void animate(double animationTime);
  void draw();
  float getAlpha() const {return alpha;}

private:
  Vec3 color;
  Vec2 position;
  float alpha;
  double startTime;
  std::string text;
  BitMatcher* app;
};

class GameClient : public Client {
public:
  GameClient(const std::string& address, short port, BitMatcher* app);
  virtual void handleServerMessage(const std::string& message) override;

  uint8_t current{0};
  uint8_t target{0};
  uint32_t challangeLength{0};
  uint32_t countdown{0};
  std::vector<std::pair<std::string, uint32_t>> highscore;

private:
  BitMatcher* app;
};

class BitMatcher : public GLApp {
public:
  BitMatcher();
  virtual ~BitMatcher();
  
  virtual void animate(double animationTime) override;
  virtual void init() override;
  virtual void draw() override;

  void addOverlay(const std::string& name, uint8_t current, uint8_t next, const std::string& opText);
  
  std::shared_ptr<FontEngine> fe{nullptr};

private:
  static FontRenderer fr;
  std::mutex overlayMutex;
  
  std::deque<OverlayImage> overlays;
  
  GameClient client;
  
  std::string intToBin(uint8_t number) const;

  void drawNumber(const std::string& title, uint8_t number, const Vec2& offset);
  void drawHighscore(const Vec2& offset);
  void drawCountdown(const Vec2& offset);
};
