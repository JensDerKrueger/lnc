#include <algorithm>

#include "BitMatcherServer.h"

#include <Rand.h>
#include <Base64.h>

BitMatcher::BitMatcher() :
  Server(11004)
{
  const std::scoped_lock<std::mutex> lock(gameStateMutex);
  loadHighscore();
  shuffleNumbers();
  frontendConnections.start();
  frontendConnections.updateHighscore(highscore);
}

BitMatcher::~BitMatcher() {
  const std::scoped_lock<std::mutex> lock(gameStateMutex);
  saveHighscore();
}

void BitMatcher::shuffleNumbers() {
  current = uint8_t(Rand::rand01()*256);
  do {
    target  = uint8_t(Rand::rand01()*256);
  } while (target == current);
}

void BitMatcher::mainLoop(double animationTime) {
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

  frontendConnections.updateState(countdown, current, target, challangeLength);
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
  std::ofstream scoreFile ("scores.csv");
  if (scoreFile.is_open()) {
    for (const HighScoreEntry& e : highscore) {
      scoreFile << base64_encode(e.name) << ";" << e.score << ";" << e.opID << std::endl;
    }
    scoreFile.close();
  }
  std::ofstream humanReadableScoreFile ("scores.txt");
  if (humanReadableScoreFile.is_open()) {
    for (const HighScoreEntry& e : highscore) {
      scoreFile << e.name << "," << e.score << "," << e.opID << std::endl;
    }
    humanReadableScoreFile.close();
  }
}

size_t BitMatcher::getPlayerIndex(const std::string& name) {
  for (size_t i = 0;i<highscore.size();++i) {
    if (highscore[i].name == name) return i;
  }
  highscore.push_back({name,0,Rand::rand<uint32_t>(0,op.getOperatorCount())});
  frontendConnections.updateHighscore(highscore);
  return highscore.size()-1;
}

void BitMatcher::processInput(const std::string& name, const std::string& text) {
  const std::scoped_lock<std::mutex> lock(gameStateMutex);
  const size_t index = getPlayerIndex(name);
  
  uint8_t next = op.execute(current, highscore[index].opID, text);
  frontendConnections.newInput(name, current, next, op.genOpText(highscore[index].opID));
  current = next;
  
  if (current == target) {
    challangeSolved = true;
    highscore[index].score += uint32_t(0.5f + 100.0f*countdown/challangeLength);
    std::sort(highscore.begin(), highscore.end());
    frontendConnections.updateHighscore(highscore);
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
  assignNewOps();
  saveHighscore();
}

void BitMatcher::assignNewOps() {
  for (size_t i = 0;i<highscore.size();++i) {
    uint32_t oldID = highscore[i].opID;
    uint32_t newID = Rand::rand<uint32_t>(0,op.getOperatorCount());
    while (!op.differInBothOps(oldID, newID)) {
      newID = Rand::rand<uint32_t>(0,op.getOperatorCount());
    }
    highscore[i].opID = newID;
  }
}

FrontendServer::FrontendServer(uint16_t port) :
  Server(port),
  highscore(5),
  lastCountdown{0},
  lastCurrent{0},
  lastTarget{0}
{
}

void FrontendServer::handleClientConnection(uint32_t id) {
  updateState();
  updateHighscore();
}

void FrontendServer::updateState(uint32_t countdown, uint8_t current, uint8_t target, uint32_t challangeLength) {
  if (countdown != lastCountdown ||current != lastCurrent ||target != lastTarget || lastChallangeLength != challangeLength) {
    lastCountdown = countdown;
    lastCurrent = current;
    lastTarget = target;
    lastChallangeLength = challangeLength;
    updateState();
  }
}

void FrontendServer::updateHighscore(const std::vector<HighScoreEntry>& scores) {
  for (size_t i = 0;i<std::min(scores.size(), highscore.size());++i) {
    highscore[i].first = scores[i].name;
    highscore[i].second = scores[i].score;
  }
  updateHighscore();
}

void FrontendServer::updateState() {
  Encoder e{char(1)};
  e.add(0);
  e.add(lastCountdown);
  e.add(lastCurrent);
  e.add(lastTarget);
  e.add(lastChallangeLength);
  sendMessage(e.getEncodedMessage());
}

void FrontendServer::newInput(const std::string& name, uint8_t current, uint8_t next, const std::string& opText) {
  Encoder e{char(1)};
  e.add(1);
  e.add(name);
  e.add(current);
  e.add(next);
  e.add(opText);
  sendMessage(e.getEncodedMessage());
}

void FrontendServer::updateHighscore() {
  Encoder e{char(1)};
  e.add(2);
  e.add(uint32_t(highscore.size()));
  for (size_t i = 0;i<highscore.size();++i) {
    e.add(highscore[i].first);
    e.add(highscore[i].second);
  }
  sendMessage(e.getEncodedMessage());
}
