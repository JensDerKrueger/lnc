#include <algorithm>
#include <bitset>

#include "BitMatcherClient.h"

#include <Rand.h>
#include <Base64.h>

#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer BitMatcher::fr{fontImage, fontPos};
#else
  FontRenderer BitMatcher::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

BitMatcher::BitMatcher() :
  GLApp(1024, 768, 1, "BitMatcher"),
  client("134.91.11.186", 11005, this)
{
}

BitMatcher::~BitMatcher() {
}

void BitMatcher::init() {
  fe = fr.generateFontEngine();
}

std::string BitMatcher::intToBin(uint8_t number) const {
  std::string str = std::bitset< 8 >( number ).to_string();
  std::stringstream ss;
  ss << " ";
  for (char c : str) {
    ss << c << " ";
  }
  return ss.str();
}

void BitMatcher::drawNumber(const std::string& title, uint8_t number, const Vec2& offset) {
  setDrawTransform(Mat4::scaling(0.3f,0.5f,1.0f) * Mat4::translation(offset.x(), offset.y(), 0.0f));
  drawRect({0.0f,0.0f,0.2f,1.0f});
    
  fe->render(title, getAspect(), 0.1f, {offset.x(), offset.y() + 0.35f}, Alignment::Center);
  fe->render(number, getAspect(), 0.2f, offset, Alignment::Center);
  fe->render(intToBin(number), getAspect(), 0.05f, {offset.x(),offset.y() - 0.4f}, Alignment::Center);
}

void BitMatcher::animate(double animationTime) {
  overlayMutex.lock();
  for (OverlayImage& o : overlays) {
    o.animate(animationTime);
  }
  if (!overlays.empty() && overlays.front().getAlpha() == 0) overlays.pop_front();
  overlayMutex.unlock();
}

void BitMatcher::drawCountdown(const Vec2& offset) {
  const float alpha = client.countdown/float(client.challangeLength);
  const Vec4 color = Vec4{0.0f,0.2f,0.0f,1.0f} * alpha + Vec4{1.0f,0.0f,0.0f,1.0f} * (1.0f-alpha);
  
  setDrawTransform(Mat4::scaling(0.3f,0.35f,1.0f) * Mat4::translation(offset.x(), offset.y()+0.05f, 0.0f));
  drawRect(color);

  fe->render("Time Remaining", getAspect(), 0.05f, {offset.x(),offset.y()+0.3f}, Alignment::Center);
  fe->render(client.countdown, getAspect(), 0.2f, {offset.x(),offset.y()-0.05f}, Alignment::Center);
}

void BitMatcher::drawHighscore(const Vec2& offset) {
  setDrawTransform(Mat4::scaling(0.4f,0.35f,1.0f) * Mat4::translation(offset.x(), offset.y()+0.05f, 0.0f));
  drawRect({0.0f,0.2f,0.0f,1.0f});
    
  fe->render("Highscore", getAspect(), 0.05f, {offset.x(),offset.y()+0.3f}, Alignment::Center);

  for (size_t i=0;i<5;++i) {
    std::pair<std::string, uint32_t> e = (i >= client.highscore.size()) ? std::make_pair<std::string, uint32_t>("----",0) : client.highscore[i];
    fe->render(uint32_t(i+1), getAspect(), 0.04f, {offset.x()-0.33f,offset.y()+(i-1.5f)*-0.08f}, Alignment::Center);
    fe->render(e.first, getAspect(), 0.04f, {offset.x(),offset.y()+(i-1.5f)*-0.08f}, Alignment::Center);
    fe->render(e.second, getAspect(), 0.04f, {offset.x()+0.3f,offset.y()+(i-1.5f)*-0.08f}, Alignment::Center);
  }
}

void BitMatcher::draw() {
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  
  drawNumber("Current", client.current, {-0.45f, 0.4f});
  drawNumber("Target", client.target, {0.45f, 0.4f});
  drawHighscore({-0.45f,-0.6f});
  drawCountdown({0.45f, -0.6f});
    
  overlayMutex.lock();
  for (OverlayImage& o : overlays) {
    o.draw();
  }
  overlayMutex.unlock();
}

void BitMatcher::addOverlay(const std::string& name, uint8_t current, uint8_t next, const std::string& opText) {
  const std::scoped_lock<std::mutex> lock(overlayMutex);
  overlays.push_back({name, current, next, opText, this});
}

OverlayImage::OverlayImage(const std::string& name, uint8_t current, uint8_t next, const std::string& opText, BitMatcher* app) :
position{Rand::rand<float>(-0.5,0.5),Rand::rand<float>(-0.9f,-0.1f)},
alpha{1.0f},
startTime{0.0},
app(app)
{
  std::stringstream ss;
  ss << name << ": " << opText;
  text = ss.str();

  float r = Rand::rand01();
  float g = Rand::rand01();
  float b = 2.0f-(r*g);
  color = Vec3(r,g,b)*0.8f;
}

void OverlayImage::animate(double animationTime) {
  if (startTime == 0) startTime = animationTime;
  alpha = float(1.0-std::min((animationTime-startTime)/12.0, 1.0));
}

void OverlayImage::draw() {
  const Vec2 size = app->fe->getSize(text, app->getAspect(), 0.05f);
  app->setDrawTransform(Mat4::scaling(1.1f*size.x(), 1.1f*size.y(), 1.0f) * Mat4::translation(Vec3{position.x(),position.y(), 0.0f}));
  app->drawRect(Vec4(color, alpha));

  app->fe->render(text, app->getAspect(), 0.05f,  position, Alignment::Center, Vec4(1.0f,1.0f,1.0f,alpha));
}

GameClient::GameClient(const std::string& address, short port, BitMatcher* app) :
  Client{address, port , "", 5000},
  app{app}
{
  highscore.resize(5);
  for (size_t i = 0;i<highscore.size();++i) {
    highscore[i].first = "---";
    highscore[i].second = 0;
  }
}

void GameClient::handleServerMessage(const std::string& message) {
  try {
    Tokenizer t{message, char(1)};
    uint32_t messageID = t.nextUint32();
    
    switch (messageID) {
      case 0 : {
        countdown = t.nextUint32();
        current = t.nextUint8();
        target = t.nextUint8();
        challangeLength = t.nextUint32();
      }
      break;
      case 1 : {
        const std::string name   = t.nextString();
        const uint8_t current    = t.nextUint8();
        const uint8_t next       = t.nextUint8();
        const std::string opText = t.nextString();
        app->addOverlay(name, current, next, opText);
      }
      break;
      case 2 : {
        highscore.resize(t.nextUint32());
        for (size_t i = 0;i<highscore.size();++i) {
          highscore[i].first = t.nextString();
          highscore[i].second = t.nextUint32();
        }
      }
      break;
    }
  } catch (const MessageException e) {
  }
}
