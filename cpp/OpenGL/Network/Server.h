#pragma once

#include "NetCommon.h"

class ClientConnection {
public:
  ClientConnection(TCPSocket* connectionSocket, size_t id, const std::string& key);
  virtual ~ClientConnection();
  bool isConnected();
  std::string checkData();
  
  size_t getID() const {return id;}
  
  void sendMessage(std::string message, uint32_t timeout);

  
private:
  TCPSocket* connectionSocket;
  size_t id;
  std::string message{""};
  uint32_t messageLength{0};
  std::vector<int8_t> recievedBytes;
  std::unique_ptr<AESCrypt> crypt;
  std::unique_ptr<AESCrypt> sendCrypt;
  std::string key;
  
  std::string handleIncommingData(int8_t* data, uint32_t bytes);
  
  void sendRawMessage(std::string message, uint32_t timeout);

};

class Server {
public:
  Server(short port, const std::string& key="", uint32_t timeout = 5000);
  virtual ~Server();
  
  bool isStarting() const {return starting;}
  bool isOK() const {return ok;}
  
  virtual void handleClientConnection(size_t id) {};
  virtual void handleClientMessage(size_t id, const std::string& message) = 0;
  virtual void handleClientDisconnection(size_t id) {};
  
  void sendMessage(const std::string& message, size_t id=0, bool invertID=false);
  
  std::vector<uint32_t> getValidIDs();
  
private:
  short port;
  uint32_t timeout;
  size_t id = 0;
  std::string key;
  
  bool ok{false};
  bool starting{true};
  bool continueRunning{true};
  std::thread connectionThread;
  std::thread clientThread;
  std::recursive_mutex clientVecMutex;
   
  std::shared_ptr<TCPServer> serverSocket;
  std::vector<std::shared_ptr<ClientConnection>> clientConnections;
  
  void shutdownServer();
  void clientFunc();
  void serverFunc();
    
  void removeClient(size_t i);
  
};
