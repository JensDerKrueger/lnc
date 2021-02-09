#pragma once

#include "NetCommon.h"

class Client {
public:
  Client(const std::string& address, short port, const std::string& key="", uint32_t timeout = 100);
  virtual ~Client();
  
  bool isConnecting() const {return connecting;}
  bool isOK() const {return ok;}
    
  void sendMessage(const std::string& message);
  virtual void handleServerMessage(const std::string& message) {};
  virtual void handleNewConnection() {};

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
  std::unique_ptr<AESCrypt> crypt;
  std::unique_ptr<AESCrypt> receiveCrypt;
  std::string key;

  
  std::vector<std::string> sendMessages;
  std::mutex sendMessageMutex;
    
  std::shared_ptr<TCPSocket> connection;
  
  void shutdownClient();
  void clientFunc();
  
  std::string handleIncommingData(int8_t* data, uint32_t bytes);
    
  void sendRawMessage(std::string message, uint32_t timeout);
  void sendRawMessage(std::vector<int8_t> rawData, uint32_t timeout);
  void sendRawMessage(const int8_t* rawData, uint32_t size, uint32_t timeout);
  std::vector<uint8_t> intToVec(uint32_t i) const;

};
