#pragma once

#include <queue>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <variant>
#include <functional>

#include "NetCommon.h"

#undef NO_DATA

enum class DataResult {
  NO_DATA,
  STRING_DATA,
  BINARY_DATA,
  PROTOCOL_DATA
};

typedef std::function<void(const std::string&)> ErrorFunction;

class BaseClientConnection {
public:
  BaseClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction);
  virtual ~BaseClientConnection();
  
  bool isConnected();
  uint32_t getID() const {return id;}

  virtual DataResult checkData();
  void enqueueMessage(const std::string& m);
  void enqueueMessage(const std::vector<uint8_t>& m);

  std::string getPeerAddress() const;
  uint16_t getPeerPort() const;
  
  std::string strData;
  std::vector<uint8_t> binData;
  uint32_t protocolDataID;

protected:
  TCPSocket* connectionSocket;
  uint32_t id;
  std::string key;
  uint32_t timeout;
  DataResult lastResult;
  bool handshakeComplete{false};
  
  ErrorFunction errorFunction;

  virtual DataResult handleIncommingData(int8_t* data, uint32_t bytes) = 0;
  virtual void sendMessage(const std::string& message) = 0;
  virtual void sendMessage(const std::vector<uint8_t>& message) = 0;
  
private:
  std::mutex messageQueueLock;
  std::queue<std::variant<std::string, std::vector<uint8_t>>> messageQueue;

  bool continueRunning{true};
  std::thread sendThread;

  void sendFunc();
};

struct HTTPRequest {
  std::string name{""};
  std::string target{""};
  std::string version{""};
  std::map<std::string, std::string> parameters;
};

class HttpClientConnection : public BaseClientConnection {
public:
  HttpClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction);
  virtual ~HttpClientConnection() {}

  static HTTPRequest parseHTTPRequest(const std::string& initialMessage);

protected:
  std::vector<int8_t> receivedBytes;
  
  virtual DataResult handleIncommingData(int8_t* data, uint32_t bytes) override;
  virtual void sendMessage(const std::string& message) override;
  virtual void sendMessage(const std::vector<uint8_t>& message) override {
    sendMessage(base64_encode(message));
  }

  static std::string CRLF() {return std::string(1,char(13)) + std::string(1,char(10));}
  void sendString(const std::string& message);
  void sendData(const std::vector<uint8_t>& message);
};

class SizedClientConnection : public BaseClientConnection {
public:
  SizedClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction);
  virtual ~SizedClientConnection() {}

  virtual DataResult checkData() override;
  
protected:
  virtual DataResult handleIncommingData(int8_t* data, uint32_t bytes) override;
  virtual void sendMessage(const std::string& message) override;
  virtual void sendMessage(const std::vector<uint8_t>& message) override {
    sendMessage(base64_encode(message));
  }
  
private:
  uint32_t messageLength{0};
  std::vector<int8_t> receivedBytes;
  std::unique_ptr<AESCrypt> crypt;
  std::unique_ptr<AESCrypt> sendCrypt;

  void sendRawMessage(std::string message);
  void sendRawMessage(std::vector<int8_t> rawData);
  void sendRawMessage(const int8_t* rawData, uint32_t size);
  std::vector<uint8_t> intToVec(uint32_t i) const;
};

  
class WebSocketConnection : public HttpClientConnection {
public:
  enum class CloseReason {
    NormalClosure = 1000,
    GoingAway = 1001,
    ProtocolError = 1002,
    UnsupportedData = 1003,
    NoStatusRcvd = 1005,
    AbnormalClosure = 1006,
    InvalidFramePayloadData = 1007,
    PolicyViolation = 1008,
    MessageTooBig = 1009,
    MandatoryExt = 1010,
    InternalServerError = 1011,
    TLSHandshake = 1015
  };

  WebSocketConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction);
  virtual ~WebSocketConnection();
  
protected:
  uint8_t currentOpcode;
  bool fragmentedData{false};
  bool isBinary{false};
  std::vector<uint8_t> receivedBytes;
  std::vector<uint8_t> fragmentedBytes;
  
  virtual DataResult handleIncommingData(int8_t* data, uint32_t bytes) override;
  virtual void sendMessage(const std::string& message) override;
  virtual void sendMessage(const std::vector<uint8_t>& message) override;

private:
  void handleHandshake(const std::string& initialMessage);
  void unmask(size_t nextByte, size_t payloadLength, const std::array<uint8_t, 4>& mask);
  DataResult generateResult(bool finalFragment, size_t nextByte, uint64_t payloadLength);
  DataResult handleFrame();
  static std::vector<uint8_t> genFrame(uint64_t s, uint8_t code);
  static std::vector<uint8_t> genFrame(const std::string& message);
  static std::vector<uint8_t> genFrame(const std::vector<uint8_t>& message);
  void closeWebsocket(CloseReason reason);
  void closeWebsocket(uint16_t reasonCode);
  
  void sendPong();
  uint16_t extractReasonCode();

};


template <class T = SizedClientConnection>
class Server {
public:
  Server(uint16_t port, const std::string& key="", uint32_t timeout = 5000);
  virtual ~Server();
  void start();
  
  bool isStarting() const {return starting;}
  bool isOK() const {return ok;}
  
  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) {};
  virtual void handleClientMessage(uint32_t id, const std::string& message) = 0;
  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) {
    handleClientMessage(id, std::string(message.begin(),message.end()));
  }
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) {}

  virtual void handleClientDisconnection(uint32_t id) {}
  virtual void handleError(const std::string& message) {}
  
  void sendMessage(const std::string& message, uint32_t id=0, bool invertID=false);
  void sendMessage(const std::vector<uint8_t>& message, uint32_t id=0, bool invertID=false);
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
  std::mutex clientVecMutex;
   
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
void Server<T>::sendMessage(const std::vector<uint8_t>& message, uint32_t id, bool invertID) {
  const std::scoped_lock<std::mutex> lock(clientVecMutex);
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (id == 0 ||
        (!invertID && clientConnections[i]->getID() == id) ||
        (invertID && clientConnections[i]->getID() != id)) {
      clientConnections[i]->enqueueMessage(message);
    }
  }
}


template <class T>
void Server<T>::sendMessage(const std::string& message, uint32_t id, bool invertID) {
  const std::scoped_lock<std::mutex> lock(clientVecMutex);
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (id == 0 ||
        (!invertID && clientConnections[i]->getID() == id) ||
        (invertID && clientConnections[i]->getID() != id)) {
      clientConnections[i]->enqueueMessage(message);
    }
  }
}

template <class T>
void Server<T>::removeClient(size_t i) {
  const std::scoped_lock<std::mutex> lock(clientVecMutex);
  const uint32_t cid = clientConnections[i]->getID();
  clientConnections.erase(clientConnections.begin() + long(i));
  handleClientDisconnection(cid);
}

template <class T>
void Server<T>::clientFunc() {
  while (continueRunning) {
    for (size_t i = 0;i<clientConnections.size();++i) {
      
      // remove clients that have disconnected
      try {
        if (!clientConnections[i]->isConnected()) {
          removeClient(i);
          continue;
        }
      } catch (const SocketException&) {
      }
      
      DataResult message = DataResult::NO_DATA;
      try {
        message = clientConnections[i]->checkData();
      } catch (SocketException const& ) {
        removeClient(i);
        continue;
      }

      try {
        if (message != DataResult::NO_DATA) {
          switch (message) {
            case DataResult::STRING_DATA:
              handleClientMessage(clientConnections[i]->getID(), std::move(clientConnections[i]->strData));
              break;
            case DataResult::BINARY_DATA:
              handleClientMessage(clientConnections[i]->getID(), std::move(clientConnections[i]->binData));
              break;
            case DataResult::PROTOCOL_DATA:
              handleProtocolMessage(clientConnections[i]->getID(), clientConnections[i]->protocolDataID, std::move(clientConnections[i]->binData));
              break;
            case DataResult::NO_DATA:
              // never hit, silnce warning
              break;
          }
        }
      } catch (SocketException const& ) {
        removeClient(i);
        continue;
      } catch (AESException const& e) {
        std::stringstream ss;
        ss << "encryption error: " << e.what() << std::endl;
        handleError(ss.str());
        removeClient(i);
        continue;
      }
      if (!continueRunning) break;
    }
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
      handleError(ss.str());

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

        ++lastClientId;
        clientVecMutex.lock();
        
        auto delegate = std::bind(&Server::handleError, this, std::placeholders::_1);
        clientConnections.push_back(std::make_shared<T>(connectionSocket, lastClientId, key, timeout, delegate));
        clientVecMutex.unlock();
        try {
          handleClientConnection(lastClientId, connectionSocket->GetPeerAddress(), connectionSocket->GetPeerPort());
        } catch (SocketException const& ) {
        }
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
  const std::scoped_lock<std::mutex> lock(clientVecMutex);
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (clientConnections[i]->getID() == id) {
      clientConnections.erase(clientConnections.begin() + long(i));
      return;
    }
  }
}
