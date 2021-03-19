#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "ChatRelayServer.h"

#include <Rand.h>
#include <Base64.h>

ChatRelayServer::ChatRelayServer() :
  Server(11004)
{
  frontendConnections1.start();
  frontendConnections2.start();
}

ChatRelayServer::~ChatRelayServer() {
}

std::pair<std::string,std::string> ChatRelayServer::parseParameter(const std::string& param) const {
  size_t sep = param.find("=");
  if (sep == std::string::npos)
    throw MessageException("Seperator = not found in parameter");
  return std::make_pair<std::string,std::string>(param.substr(0, sep), param.substr(sep+1));
}

std::map<std::string,std::string> ChatRelayServer::parseParameters(const std::string& params) const {
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

void ChatRelayServer::handleClientMessage(uint32_t id, const std::string& message) {
  Tokenizer t{message, ' '};
  try {
    std::string command = t.nextString();
    std::string parameter = t.nextString();
    
    if (command == "GET") {
      std::map<std::string,std::string> p = parseParameters(parameter);
      
      const std::string channel = p["chan"];
      const std::string name    = p["name"];
      const std::string text    = p["text"];

      if (!name.empty()) {
        sendMessage("Player " + base64url_decode(name) + " wrote " + base64url_decode(text) + " in stream " + base64url_decode(channel) , id);
        if (base64url_decode(channel) == "#bitmatcher")
          frontendConnections1.newInput(name, text);
        if (base64url_decode(channel) == "#jhkrueger")
          frontendConnections2.newInput(name, text);
      } else {
        sendMessage("Name Missing from message");
      }
      return;
    }
  } catch (const MessageException& ) {
  }
  sendMessage("Something went wrong with the message:" + message, id);
}

FrontendServer::FrontendServer(uint16_t port) :
  Server(port)
{
}

void FrontendServer::newInput(const std::string& name, const std::string& text) {
  Encoder e{char(1)};
  e.add(name);
  e.add(text);
  sendMessage(e.getEncodedMessage());
}

std::string FrontendServer::getTimeStr() const {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

std::string FrontendServer::clean(const std::string& message) const {
  static const std::string safeChars = "abcdefghijklmonqrstuvwxyzABCDEFGHIJKLMNOPRSTUVWXYZ1234567890ß+-*,.;:-_<># !?";

  std::string safeString{""};
  for (size_t i = 0;i<message.size();++i) {
    if ( safeChars.find(message[i]) == std::string::npos ) {
      safeString += "_";
    } else {
      safeString += message[i];
    }
  }
  return safeString;
}

void FrontendServer::handleClientConnection(uint32_t id, const std::string& address, uint16_t port) {
  connectionInfos[id] = ConnectionInfo{id, address, port};
  std::cout << getTimeStr() << " New connection to " << address << ":" << port << " established." << std::endl;
}

void FrontendServer::handleClientDisconnection(uint32_t id)  {
  std::cout << getTimeStr() << " Connection to " << connectionInfos[id].address << ":" << connectionInfos[id].port << " terminated" << std::endl;
  connectionInfos.erase(id);
}

void FrontendServer::handleClientMessage(uint32_t id, const std::string& message) {
  std::cout << getTimeStr() << " Received " << clean(message) << std::endl;
}
