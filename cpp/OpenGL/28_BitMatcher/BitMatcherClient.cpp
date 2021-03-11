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
  currentTitle = GLTexture2D{fr.render("Current")};
  targetTitle = GLTexture2D{fr.render("Target")};
  remainTitle = GLTexture2D{fr.render("Time Remaining")};
  highscoreTitle = GLTexture2D{fr.render("Highscore")};
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

void BitMatcher::drawNumber(const GLTexture2D& title, uint8_t number, const Vec2& offset) {
  setDrawTransform(Mat4::scaling(0.3f,0.5f,1.0f) * Mat4::translation(offset.x(), offset.y(), 0.0f));
  drawRect({0.0f,0.0f,0.2f,1.0f});
  
  setDrawTransform(computeImageTransformFixedHeight({title.getWidth(), title.getHeight()},
                                                    0.1f, {offset.x(), offset.y() + 0.35f,0}));
  drawImage(title);

  Image numberImage = fr.render(number);
  setDrawTransform(computeImageTransformFixedHeight({numberImage.width, numberImage.height},
                                                    0.2f,{offset.x(),offset.y(),0}));
  drawImage(numberImage);
  
  numberImage = fr.render(intToBin(number));
  setDrawTransform(computeImageTransformFixedHeight({numberImage.width, numberImage.height},
                                                    0.05f,{offset.x(),offset.y() - 0.4f,0}));
  drawImage(numberImage);
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

  setDrawTransform(computeImageTransformFixedHeight({remainTitle.getWidth(), remainTitle.getHeight()},0.05f,{0,0,0}) * Mat4::translation({offset.x(),offset.y()+0.25f,0}));
  drawImage(remainTitle);

  Image numberImage = fr.render(client.countdown);
  setDrawTransform(computeImageTransformFixedHeight({numberImage.width, numberImage.height},
                                                    0.2f,{offset.x(),offset.y()-0.05f,0}));
  drawImage(numberImage);
}

void BitMatcher::drawHighscore(const Vec2& offset) {
  setDrawTransform(Mat4::scaling(0.5f,0.35f,1.0f) * Mat4::translation(offset.x(), offset.y()+0.05f, 0.0f));
  drawRect({0.0f,0.2f,0.0f,1.0f});

  setDrawTransform(computeImageTransformFixedHeight({highscoreTitle.getWidth(), highscoreTitle.getHeight()},0.05f,{0,0,0}) * Mat4::translation({offset.x(),offset.y()+0.25f,0}));
  drawImage(highscoreTitle);

  for (size_t i=0;i<5;++i) {
    std::pair<std::string, uint32_t> e = (i >= client.highscore.size()) ? std::make_pair<std::string, uint32_t>("----",0) : client.highscore[i];
    std::stringstream ss;
    ss << e.first << " " << e.second;
    Image lineImage = fr.render(ss.str());
    setDrawTransform(computeImageTransformFixedHeight({lineImage.width, lineImage.height},0.04f,{0,0,0}) * Mat4::translation({offset.x(),offset.y()+(i-1.5f)*-0.08f,0}));
    drawImage(lineImage);
  }
}

void BitMatcher::draw() {
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  
  drawNumber(currentTitle, client.current, {-0.45f, 0.4f});
  drawNumber(targetTitle, client.target, {0.45f, 0.4f});
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
  ss << name << ": " << opText; //int(current) << " > " << int(next);
  text = app->fr.render(ss.str());

  float r = Rand::rand01();
  float g = Rand::rand01();
  float b = 2.0f-(r*g);
  color = Vec3(r,g,b);
}

void OverlayImage::animate(double animationTime) {
  if (startTime == 0) startTime = animationTime;
  alpha = float(1.0-std::min((animationTime-startTime)/12.0, 1.0));
}

void OverlayImage::draw() {
  Image i = text;
  i.multiply(Vec4(1,1,1,alpha));
  
  const uint32_t w = i.width;
  const uint32_t h = i.height;
    
  Mat4 textTrans = app->computeImageTransformFixedHeight({w,h},0.05f,{position.x(),position.y(),0});
  app->setDrawTransform(Mat4::scaling(1.1f,1.1f,1.0f) * textTrans);
  app->drawRect(Vec4(color, alpha));
  app->setDrawTransform(textTrans);
  app->drawImage(i);
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
          highscore[i].second = t.nextUint8();
        }
      }
      break;
    }
  } catch (const MessageException e) {
  }
}
