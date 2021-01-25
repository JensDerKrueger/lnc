#pragma once

#include <thread>
#include <mutex>
#include <vector>

#include "Sockets.h"

class ClientConnection {
public:
  ClientConnection(TCPSocket* connectionSocket);
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
  static inline size_t idCounter = 1;
  
  std::string handleIncommingData(int8_t* data, uint32_t bytes);
};

class Server {
public:
  Server(short port, uint32_t timeout = 100);
  virtual ~Server();
  
  bool isStarting() const {return starting;}
  bool isOK() const {return ok;}
    
  virtual void handleClientMessage(size_t id, const std::string& message) = 0;
  
  void sendMessage(const std::string& message, size_t id=0, bool invertID=false);
  
private:
  short port;
  uint32_t timeout;
  
  bool ok{false};
  bool starting{true};
  bool continueRunning{true};
  std::thread connectionThread;
  std::thread clientThread;
  std::mutex clientVecMutex;
   
  std::shared_ptr<TCPServer> serverSocket;
  std::vector<std::shared_ptr<ClientConnection>> clientConnections;
  
  void shutdownServer();
  void clientFunc();
  void serverFunc();
    
};
