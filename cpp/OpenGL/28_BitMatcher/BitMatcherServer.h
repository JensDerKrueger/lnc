#pragma once

#include <array>
#include <vector>
#include <map>
#include <deque>

#include <Server.h>
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

class FrontendServer : public Server<SizedClientConnection> {
public:
  FrontendServer(uint16_t port);

  virtual void handleClientConnection(uint32_t id) override;
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {}
  void updateState(uint32_t countdown, uint8_t current, uint8_t target, uint32_t challangeLength);
  void updateHighscore(const std::vector<HighScoreEntry>& highscore);
  void newInput(const std::string& name, uint8_t current, uint8_t next, const std::string& opText);

private:
  std::vector<std::pair<std::string, uint32_t>> highscore;
  uint32_t lastCountdown;
  uint8_t lastCurrent;
  uint8_t lastTarget;
  uint32_t lastChallangeLength;
  
  void updateHighscore();
  void updateState();
};

class BitMatcher : public Server<HttpClientConnection> {
public:
  BitMatcher();
  virtual ~BitMatcher();
    
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;

  void mainLoop(double animationTime);
private:
  FrontendServer frontendConnections{11005};
  
  uint8_t current{0};
  uint8_t target{0};
  bool challangeSolved{false};
  double challangeStartTime{0};
  
  std::mutex gameStateMutex;
  uint32_t countdown;
  uint32_t challangeLength{100};
  
  Operator op;
  
  std::vector<HighScoreEntry> highscore;
  
  void shuffleNumbers();
  void assignNewOps();
    
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
