#pragma once

#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>

#include "NetCommon.h"

class BaseClientConnection {
public:
  BaseClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout);
  virtual ~BaseClientConnection();
  
  bool isConnected();
  uint32_t getID() const {return id;}

  virtual std::string checkData();
  void enqueueMessage(const std::string& m);

protected:
  TCPSocket* connectionSocket;
  uint32_t id;
  std::string key;
  uint32_t timeout;

  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) = 0;
  virtual void sendMessage(std::string message) = 0;
  
private:
  std::mutex messageQueueLock;
  std::queue<std::string> messageQueue;

  bool continueRunning{true};
  std::thread sendThread;

  void sendFunc();
};


class HttpClientConnection : public BaseClientConnection {
public:
  HttpClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout);
  virtual ~HttpClientConnection() {}
  
protected:
  std::vector<int8_t> recievedBytes;
  
  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) override;
  virtual void sendMessage(std::string message) override;

};

class SizedClientConnection : public BaseClientConnection {
public:
  SizedClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout);
  virtual ~SizedClientConnection() {}

  virtual std::string checkData() override;
  
protected:
  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) override;
  virtual void sendMessage(std::string message) override;
  
private:
  uint32_t messageLength{0};
  std::vector<int8_t> recievedBytes;
  std::unique_ptr<AESCrypt> crypt;
  std::unique_ptr<AESCrypt> sendCrypt;

  void sendRawMessage(std::string message);
  void sendRawMessage(std::vector<int8_t> rawData);
  void sendRawMessage(const int8_t* rawData, uint32_t size);
  std::vector<uint8_t> intToVec(uint32_t i) const;
};

template <class T = SizedClientConnection>
class Server {
public:
  Server(uint16_t port, const std::string& key="", uint32_t timeout = 5000);
  virtual ~Server();
  void start();
  
  bool isStarting() const {return starting;}
  bool isOK() const {return ok;}
  
  virtual void handleClientConnection(uint32_t /* id */) {};
  virtual void handleClientMessage(uint32_t id, const std::string& message) = 0;
  virtual void handleClientDisconnection(uint32_t /* id */) {};
  
  void sendMessage(const std::string& message, uint32_t id=0, bool invertID=false);
  void closeConnection(uint32_t id);
  
  std::vector<uint32_t> getValidIDs();
  
private:
  uint16_t port;
  uint32_t timeout;
  uint32_t lastClientId{0};
  std::string key;
  
  bool ok{false};
  bool starting{true};
  bool continueRunning{true};
  std::thread connectionThread;
  std::thread clientThread;
  std::recursive_mutex clientVecMutex;
   
  std::shared_ptr<TCPServer> serverSocket;
  std::vector<std::shared_ptr<T>> clientConnections;
  
  void shutdownServer();
  void clientFunc();
  void serverFunc();
    
  void removeClient(size_t i);
    
};


template <class T>
Server<T>::Server(uint16_t port, const std::string& key, uint32_t timeout) :
  port{port},
  timeout{timeout},
  key{key}
{
}

template <class T>
Server<T>::~Server() {
  continueRunning = false;
  connectionThread.join();
  clientThread.join();
}

template <class T>
void Server<T>::start() {
  connectionThread = std::thread(&Server::serverFunc, this);
  clientThread = std::thread(&Server::clientFunc, this);
}

template <class T>
void Server<T>::shutdownServer() {
  starting = false;
  if (serverSocket) {
    try {
      serverSocket->Close();
    } catch (SocketException const&  ) {
    }
  }
  ok = false;
}

template <class T>
void Server<T>::sendMessage(const std::string& message, uint32_t id, bool invertID) {
  clientVecMutex.lock();
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (id == 0 ||
        (!invertID && clientConnections[i]->getID() == id) ||
        (invertID && clientConnections[i]->getID() != id)) {
      clientConnections[i]->enqueueMessage(message);
    }
  }
  clientVecMutex.unlock();
}

template <class T>
void Server<T>::removeClient(size_t i) {
  const uint32_t cid = clientConnections[i]->getID();
  clientConnections.erase(clientConnections.begin() + long(i));
  handleClientDisconnection(cid);
}

template <class T>
void Server<T>::clientFunc() {
  while (continueRunning) {
    clientVecMutex.lock();
    for (size_t i = 0;i<clientConnections.size();++i) {
      
      // remove clients that have disconnected
      if (!clientConnections[i]->isConnected()) {
        removeClient(i);
        continue;
      }
      
      try {
        std::string message = clientConnections[i]->checkData();
        if (!message.empty()) {
          clientVecMutex.unlock();
          handleClientMessage(clientConnections[i]->getID(), message);
          clientVecMutex.lock();
        }
      } catch (SocketException const& ) {
        removeClient(i);
        continue;
      } catch (AESException const& e) {
        std::cerr << "encryption error: " << e.what() << std::endl;
        removeClient(i);
        continue;
      }
      
      if (!continueRunning) break;
    }
    clientVecMutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

template <class T>
void Server<T>::serverFunc() {
  // open server port
  while (continueRunning) {
    starting= true;
    try {
      serverSocket = std::make_shared<TCPServer>();
      serverSocket->SetNonBlocking(timeout == INFINITE_TIMEOUT ? false : true);
      serverSocket->SetNoDelay(true);
      serverSocket->SetReuseAddress(true);
      serverSocket->SetNoSigPipe(true);
      serverSocket->Bind(NetworkAddress(NetworkAddress::Any, port));
      serverSocket->Listen();
      serverSocket->GetLocalPort();
    } catch (SocketException const& e) {

      std::stringstream ss;
      ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
      std::cerr << ss.str() << std::endl;

      shutdownServer();
      return;
    }
    ok = true;
    starting = false;
    
    // accept connections
    while (continueRunning) {

      try {
        TCPSocket* connectionSocket{nullptr};
        while (connectionSocket == nullptr && continueRunning) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          while (!serverSocket->AcceptNewConnection((ConnectionSocket**)&connectionSocket, timeout)) {
            if (!continueRunning) {
              if (connectionSocket) {
                connectionSocket->Close();
                delete connectionSocket;
                connectionSocket = nullptr;
              }
              shutdownServer();
              return;
            }
          }
        }
        
        try {
          // check if peer already dropped the connection
          if (!connectionSocket->IsConnected()) {
            connectionSocket->Close();
            delete connectionSocket;
            connectionSocket = nullptr;
            continue;
          }
        } catch (SocketException const& ) {
          delete connectionSocket;
          connectionSocket = nullptr;
          continue;
        }

        clientVecMutex.lock();
        ++lastClientId;
        clientConnections.push_back(std::make_shared<T>(connectionSocket, lastClientId, key, timeout));
        try {
          handleClientConnection(lastClientId);
        } catch (SocketException const& ) {
        }
        clientVecMutex.unlock();
      } catch (SocketException const& ) {
      }
    }
  }
  shutdownServer();
}

template <class T>
std::vector<uint32_t> Server<T>::getValidIDs() {
  const std::scoped_lock<std::mutex> lock(clientVecMutex);

  std::vector<uint32_t> ids(clientConnections.size());
  for (size_t i = 0;i<clientConnections.size();++i) {
    ids[i] = uint32_t(clientConnections[i]->getID());
  }
  return ids;
}

template <class T>
void Server<T>::closeConnection(uint32_t id) {
  clientVecMutex.lock();
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (clientConnections[i]->getID() == id) {
      clientConnections.erase(clientConnections.begin() + long(i));
      clientVecMutex.unlock();
      return;
    }
  }
  clientVecMutex.unlock();
}
