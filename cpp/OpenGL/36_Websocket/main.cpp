#include <iostream>

#include <Server.h>
#include <StringTools.h>
#include <SHA1.h>


class MyClientConnection : public BaseClientConnection {
public:
  MyClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  BaseClientConnection(connectionSocket, id, key, timeout) {}
  virtual ~MyClientConnection() {}
  
protected:
  bool handshakeComplete{false};
  std::vector<int8_t> receivedBytes;
  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) override {
    if (handshakeComplete) {
      
    } else {
      receivedBytes.insert(receivedBytes.end(), data, data+bytes);
      if (receivedBytes.size() > 3) {
        for (uint32_t i = 0;i<receivedBytes.size()-3;++i) {
          if ((int)receivedBytes[i] == 13 && (int)receivedBytes[i+1] == 10 &&
              (int)receivedBytes[i+2] == 13 && (int)receivedBytes[i+3] == 10) {
            std::stringstream ss;
            if (i > 0) {
              for (uint32_t j = 0;j<i;++j) {
                ss << receivedBytes[j];
              }
            }
            receivedBytes.erase(receivedBytes.begin(), receivedBytes.begin()+long(i+4));
            handleHandshake(ss.str());
            return "";
          }
        }
      }
    }
    
    return "";
  }
  
  virtual void sendMessage(std::string message) override {
  }

private:
  
 
  void handleHandshake(const std::string& initialMessage) {
    std::vector<std::string> lines = tokenize(initialMessage, "\r\n");
    
    for (const std::string& line: lines) {
      std::cout << "->" << line << "<-" << std::endl;
      
      std::vector<std::string> values = tokenize(line, ":");
      if (values.size() != 2) continue;
      
      if ("sec-websocket-key" == toLower(values[0])) {
        std::cout << "  ->" << trim(values[1]) << "<-" << std::endl;
      }
    }
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

// 258EAFA5-E914-47DA-95CA-C5AB0DC85B11

