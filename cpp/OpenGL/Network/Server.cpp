#include <sstream>
#include <iostream>

#include "Server.h"

size_t ClientConnection::idCounter = 1;

ClientConnection::ClientConnection(TCPSocket* connectionSocket) :
  connectionSocket(connectionSocket),
  id(idCounter++)
{
}

ClientConnection::~ClientConnection() {
  if (connectionSocket && connectionSocket->IsConnected()) {
    connectionSocket->Close();
  }
  delete connectionSocket;
}

bool ClientConnection::isConnected() {
  return connectionSocket && connectionSocket->IsConnected();
}

std::string ClientConnection::checkData() {
  int8_t data[2048];
  const uint32_t maxSize = std::min<uint32_t>(std::max<uint32_t>(4,messageLength),2048);
  const uint32_t bytes = connectionSocket->ReceiveData(data, maxSize, 1);
  if (bytes > 0) {
    return handleIncommingData(data, bytes);
  }
  return "";
}


void ClientConnection::sendMessage(std::string message, uint32_t timeout) {
  uint32_t l = uint32_t(message.length());
  if (l < message.length()) message.resize(l); // could create multiple messages
                                               // instead of simply truncating string

  std::vector<uint8_t> data(4+l);

  data[0] = l%256; l /= 256;
  data[1] = l%256; l /= 256;
  data[2] = l%256; l /= 256;
  data[3] = l;
  
  size_t j = 4;
  for (const int8_t c : message) {
    data[j++] = c;
  }
            
  connectionSocket->SendData((int8_t*)data.data(), uint32_t(data.size()), timeout);
}

std::string ClientConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
  recievedBytes.insert(recievedBytes.end(), data, data+bytes);

  if (messageLength == 0) {
    if (recievedBytes.size() >= 4) {
      messageLength = *((uint32_t*)recievedBytes.data());
    } else {
      return "";
    }
  }
  
  if (recievedBytes.size() >= messageLength+4) {
    std::ostringstream os;
    for (size_t i = 4;i<messageLength+4;++i) {
      os << recievedBytes[i];
    }
    recievedBytes.erase(recievedBytes.begin(), recievedBytes.begin() + messageLength+4);
    messageLength = 0;
    return os.str();
  }
  
  return "";
}


Server::Server(short port, uint32_t timeout) :
  port{port},
  timeout{timeout}
{
  connectionThread = std::thread(&Server::serverFunc, this);
  clientThread = std::thread(&Server::clientFunc, this);
}

Server::~Server() {
  continueRunning = false;
  connectionThread.join();
  clientThread.join();
}

  
void Server::shutdownServer() {
  starting = false;
  if (serverSocket) {
    try {
      serverSocket->Close();
    } catch (SocketException const&  ) {
    }
  }
  ok = false;
}

void Server::sendMessage(const std::string& message, size_t id, bool invertID) {
  clientVecMutex.lock();
  for (size_t i = 0;i<clientConnections.size();++i) {
    if (!clientConnections[i]->isConnected()) {
      clientConnections.erase(clientConnections.begin() + i);
      continue;
    }
    if (id == 0 ||
        (!invertID && clientConnections[i]->getID() != id) ||
        (invertID && clientConnections[i]->getID() != id))
      clientConnections[i]->sendMessage(message, timeout);
  }
  clientVecMutex.unlock();
}


void Server::clientFunc() {
  while (continueRunning) {
    clientVecMutex.lock();
    for (size_t i = 0;i<clientConnections.size();++i) {
      
      // remove clients that have disconnected
      if (!clientConnections[i]->isConnected()) {
        clientConnections.erase(clientConnections.begin() + i);
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
        clientConnections.erase(clientConnections.begin() + i);
        continue;
      }
      
      if (!continueRunning) break;
    }
    clientVecMutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void Server::serverFunc() {
  // open server port
  while (continueRunning) {
    starting= true;
    try {
      serverSocket = std::make_shared<TCPServer>();
      serverSocket->SetNonBlocking(timeout == INFINITE_TIMEOUT ? false : true);
      serverSocket->SetNoDelay(false);
      serverSocket->SetReuseAddress(true);
      serverSocket->Bind(NetworkAddress(NetworkAddress::Any, port));
      serverSocket->Listen();
      serverSocket->GetLocalPort();
    } catch (SocketException const& e) {

      std::stringstream ss;
      ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
      std::cout << ss.str() << std::endl;

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
        clientConnections.push_back(std::make_shared<ClientConnection>(connectionSocket));
        clientVecMutex.unlock();
      } catch (SocketException const& ) {
      }
      
    }
  }
  shutdownServer();
}

std::vector<uint32_t> Server::getValidIDs() {
  
  clientVecMutex.lock();
  std::vector<uint32_t> ids(clientConnections.size());
  for (size_t i = 0;i<clientConnections.size();++i) {
    ids[i] = clientConnections[i]->getID();
  }
  clientVecMutex.unlock();

  return ids;
}
