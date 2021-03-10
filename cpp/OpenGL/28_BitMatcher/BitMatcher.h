#pragma once

#include <array>
#include <vector>
#include <map>

#include <GLApp.h>
#include <Server.h>
#include <GLTexture2D.h>
#include <FontRenderer.h>

#include "Operator.h"

struct HighScoreEntry {
  HighScoreEntry(const std::string& name, uint32_t score, uint32_t opID) :
    name(name),
    score(score),
    opID(opID)
  {}
  
  std::string name;
  uint32_t score;
  uint32_t opID;
  
  bool operator<(const HighScoreEntry& other) const {
    return score > other.score || (score == other.score && name > other.name) ;
  }
};

class BitMatcher : public GLApp, public Server<HttpClientConnection> {
public:
  BitMatcher();
  virtual ~BitMatcher();
  
  virtual void animate(double animationTime) override;
  virtual void init() override;
  virtual void draw() override;

  virtual void handleClientMessage(uint32_t id, const std::string& message) override;


private:
  uint8_t current{0};
  uint8_t target{0};
  bool challangeSolved{false};
  double challangeStartTime{0};
  
  std::mutex gameStateMutex;
  uint32_t countdown;
  uint32_t challangeLength{100};
  
  Operator op;
  
  std::vector<HighScoreEntry> highscore;
  
  GLTexture2D currentTitle;
  GLTexture2D targetTitle;
  
  static FontRenderer fr;
  
  void shuffleNumbers();
  void drawNumber(const GLTexture2D& title, uint8_t number, const Vec2& offset);
  std::string intToBin(uint8_t number) const;
  
  void drawHighscore(const Vec2& offset);
  void drawCountdown(const Vec2& offset);

  void startNewChallange();    
  void processInput(const std::string& name, const std::string& text);
  size_t getPlayerIndex(const std::string& name);
    
  std::pair<std::string,std::string> parseParameter(const std::string& param) const;
  std::map<std::string,std::string> parseParameters(const std::string& params) const;
  std::string limitString(const std::string& str, size_t maxSize) const;

  void loadHighscore();
  void saveHighscore();
  void applyOperator(uint32_t opID, const std::string& parameter);
};
