#pragma once

#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include "Sockets.h"

class Client {
public:
  Client(const std::string& address, short port, uint32_t timeout = 100);
  virtual ~Client();
  
  bool isConnecting() const {return connecting;}
  bool isOK() const {return ok;}
    
  void sendMessage(const std::string& message);
  virtual void handleServerMessage(const std::string& message) {};
  
  size_t cueSize();
  
private:
  std::string address;
  short port;
  uint32_t timeout;
  
  bool ok{false};
  bool connecting{true};
  bool continueRunning{true};
  std::thread clientThread;

  uint32_t messageLength{0};
  std::vector<int8_t> recievedBytes;

  
  std::vector<std::string> sendMessages;
  std::mutex sendMessageMutex;
    
  std::shared_ptr<TCPSocket> connection;
  
  void shutdownClient();
  void clientFunc();
  
  std::string handleIncommingData(int8_t* data, uint32_t bytes);
    
};
