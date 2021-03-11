#include <algorithm>
#include <bitset>

#include "BitMatcher.h"

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
  Server(5566)
{
  loadHighscore();
}

BitMatcher::~BitMatcher() {
  saveHighscore();
}

void BitMatcher::init() {
  currentTitle = GLTexture2D{fr.render("Current")};
  targetTitle = GLTexture2D{fr.render("Target")};
  remainTitle = GLTexture2D{fr.render("Time Remaining")};
  highscoreTitle = GLTexture2D{fr.render("Highscore")};
  
  shuffleNumbers();
  start();
}

void BitMatcher::shuffleNumbers() {
  current = uint8_t(Rand::rand01()*256);
  do {
    target  = uint8_t(Rand::rand01()*256);
  } while (target == current);
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
  if (challangeStartTime == 0) {
    challangeStartTime = animationTime;
  }
  
  if (animationTime-challangeStartTime > challangeLength)
    countdown = 0;
  else
    countdown = uint32_t(challangeLength - (animationTime-challangeStartTime));
    
  if (challangeSolved || countdown == 0 ) {
    startNewChallange();
  }
  
  for (OverlayImage& o : overlays) {
    o.animate(animationTime);
  }

  if (!overlays.empty() && overlays.front().getAlpha() == 0) overlays.pop_front();
}

void BitMatcher::drawCountdown(const Vec2& offset) {
  const float alpha = countdown/float(challangeLength);
  const Vec4 color = Vec4{0.0f,0.2f,0.0f,1.0f} * alpha + Vec4{1.0f,0.0f,0.0f,1.0f} * (1.0f-alpha);
  
  setDrawTransform(Mat4::scaling(0.3f,0.35f,1.0f) * Mat4::translation(offset.x(), offset.y()+0.05f, 0.0f));
  drawRect(color);

  setDrawTransform(computeImageTransformFixedHeight({remainTitle.getWidth(), remainTitle.getHeight()},0.05f,{0,0,0}) * Mat4::translation({offset.x(),offset.y()+0.25f,0}));
  drawImage(remainTitle);

  Image numberImage = fr.render(countdown);
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
    HighScoreEntry e = (i >= highscore.size()) ? HighScoreEntry{"----",0,0} : highscore[i];
    std::stringstream ss;
    ss << limitString(e.name,20) << " " << e.score;
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
  
  drawNumber(currentTitle, current, {-0.45f, 0.4f});
  drawNumber(targetTitle, target, {0.45f, 0.4f});
  drawHighscore({-0.45f,-0.6f});
  drawCountdown({0.45f, -0.6f});
    
  for (OverlayImage& o : overlays) {
    o.draw();
  }
}

std::pair<std::string,std::string> BitMatcher::parseParameter(const std::string& param) const {
  size_t sep = param.find("=");
  if (sep == std::string::npos)
    throw MessageException("Seperator = not found in parameter");
  return std::make_pair<std::string,std::string>(param.substr(0, sep), param.substr(sep+1));
}

std::map<std::string,std::string> BitMatcher::parseParameters(const std::string& params) const {
  std::map<std::string,std::string> result;
  size_t start = params.find("?");
  while (start != std::string::npos) {
    const size_t end = params.find("&", start+1);
    const std::string param = (end != std::string::npos)
                              ? params.substr(start+1, end-(start+1))
                              : params.substr(start+1);
        
    std::pair<std::string,std::string> p = parseParameter(param);
    result[p.first] = p.second;
    start = end;
  }
  return result;
}

std::string BitMatcher::limitString(const std::string& str, size_t maxSize) const {
  if (str.length() <= maxSize) return str;
  return str.substr(0, maxSize-3) + std::string("...");
}

void BitMatcher::handleClientMessage(uint32_t id, const std::string& message) {
  Tokenizer t{message, ' '};
  try {
    std::string command = t.nextString();
    std::string parameter = t.nextString();
    
    if (command == "GET") {
      std::map<std::string,std::string> p = parseParameters(parameter);
      
      const std::string name = base64url_decode(p["name"]);
      const std::string text = base64url_decode(p["text"]);

      if (!name.empty()) {
        sendMessage("Player " + name + " wrote " + text , id);
        processInput(name, text);
      } else {
        sendMessage("Name Missing from message");
      }
      return;
    }
  } catch (const MessageException& ) {
  }
  sendMessage("Something went wrong with the message:" + message, id);
}

void BitMatcher::loadHighscore() {
  std::string line;
  highscore.clear();
  std::ifstream scoreFile("scores.txt");
  try {
    if (scoreFile.is_open()) {
      while (getline(scoreFile,line) && !line.empty() ) {
        Tokenizer tokenizer{line+",", ','};
        const std::string encName = tokenizer.nextString();
        const std::string name = base64_decode(encName);
        const uint32_t score = tokenizer.nextUint32();
        const uint32_t opID = tokenizer.nextUint32();
        highscore.push_back({name,score,opID});
      }
      scoreFile.close();
    }
  } catch (const MessageException& ) {
  }
}

void BitMatcher::saveHighscore() {
  std::ofstream scoreFile ("scores.txt");
  if (scoreFile.is_open()) {
    for (const HighScoreEntry& e : highscore) {
      scoreFile << base64_encode(e.name) << "," << e.score << "," << e.opID << std::endl;
    }
    scoreFile.close();
  }
}

size_t BitMatcher::getPlayerIndex(const std::string& name) {
  for (size_t i = 0;i<highscore.size();++i) {
    if (highscore[i].name == name) return i;
  }
  highscore.push_back({name,0,Rand::rand<uint32_t>(0,op.getOperatorCount())});
  return highscore.size()-1;
}

void BitMatcher::processInput(const std::string& name, const std::string& text) {
  const std::scoped_lock<std::mutex> lock(gameStateMutex);
  const size_t index = getPlayerIndex(name);
  
  uint8_t next = op.execute(current, highscore[index].opID, text);
  overlays.push_back({name, current, next, this});
  current = next;
  
  if (current == target) {
    challangeSolved = true;
    highscore[index].score += uint32_t(0.5f + 100.0f*countdown/challangeLength);
    std::sort(highscore.begin(), highscore.end());
  }
}

void BitMatcher::startNewChallange() {
  const std::scoped_lock<std::mutex> lock(gameStateMutex);
  if (challangeSolved)
    --challangeLength;
  else
    ++challangeLength;
  challangeStartTime = 0;
  challangeSolved = false;
  shuffleNumbers();
}


OverlayImage::OverlayImage(const std::string& name, uint8_t current, uint8_t next, BitMatcher* app) :
position{Rand::rand<float>(-0.5,0.5),Rand::rand<float>(-0.9f,-0.1f)},
alpha{1.0f},
startTime{0.0},
app(app)
{
  std::stringstream ss;
  ss << name << ": " << int(current) << " > " << int(next);
  text = app->fr.render(ss.str());

  float r = Rand::rand01();
  float g = Rand::rand01();
  float b = 2.0f-(r*g);
  color = Vec3(r,g,b);
}

void OverlayImage::animate(double animationTime) {
  if (startTime == 0) startTime = animationTime;
  alpha = float(1.0-std::min((animationTime-startTime)/2.0, 1.0));
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
