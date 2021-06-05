#include <iostream>

#include <Server.h>


class MyClientConnection : public BaseClientConnection {
public:
  MyClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  BaseClientConnection(connectionSocket, id, key, timeout) {}
  virtual ~MyClientConnection() {}
  
protected:
  std::vector<int8_t> recievedBytes;
  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) override {
    recievedBytes.insert(recievedBytes.end(), data, data+bytes);
    if (recievedBytes.size() > 3) {
      for (uint32_t i = 0;i<recievedBytes.size()-3;++i) {
        if ((int)recievedBytes[i] == 13 && (int)recievedBytes[i+1] == 10 &&
            (int)recievedBytes[i+2] == 13 && (int)recievedBytes[i+3] == 10) {
          std::stringstream ss;
          if (i > 0) {
            for (uint32_t j = 0;j<i;++j) {
              ss << recievedBytes[j];
            }
          }
          recievedBytes.erase(recievedBytes.begin(), recievedBytes.begin()+long(i+4));
          return ss.str();
        }
      }
    }
    return "";
  }
  
  virtual void sendMessage(std::string message) override {
  }

};

class EchoServer : public Server<MyClientConnection> {
public:
  EchoServer(uint16_t port) :
  Server(port)
  {
  }
  
  virtual ~EchoServer() {
    
  }
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    std::cout << message << std::flush;
  }
};


int main(int argc, char ** argv) {
  EchoServer s(8899);
  s.start();
  
  while (true) {
    
  }
  
  return EXIT_SUCCESS;
}
