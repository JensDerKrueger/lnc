#include <iostream>
#include <charconv>

#include <Server.h>
#include <StringTools.h>
#include <SHA1.h>


class MyClientConnection : public HttpClientConnection {
public:
  MyClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  HttpClientConnection(connectionSocket, id, key, timeout) {}
  virtual ~MyClientConnection() {}
  
protected:
  bool handshakeComplete{false};
  std::vector<int8_t> receivedBytes;
  virtual std::string handleIncommingData(int8_t* data, uint32_t bytes) override {
    if (handshakeComplete) {
      for (size_t i = 0;i<bytes;++i) {
        std::cout << int(((uint8_t*)data)[i]) << " ";
      }
      std::cout << std::endl;
    } else {
      if (receivedBytes.size() > 1024*1204) receivedBytes.clear();
      receivedBytes.insert(receivedBytes.end(), data, data+bytes);
      if (receivedBytes.size() > 3) {
        for (uint32_t i = 0;i<receivedBytes.size()-3;++i) {
          if (receivedBytes[i] == 13 && receivedBytes[i+1] == 10 &&
              receivedBytes[i+2] == 13 && receivedBytes[i+3] == 10) {
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
    HTTPRequest request = parseHTTPRequest(initialMessage);

    if (request.name == "GET" &&
        toLower(request.parameters["upgrade"]) == "websocket" &&
        toLower(request.parameters["connection"]) == "upgrade") {

      const std::string challengeResponse = base64_encode(sha1(request.parameters["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
    
      std::stringstream ss;
      ss << "HTTP/1.1 101 Switching Protocols" << CRLF()
         << "Upgrade: websocket" << CRLF()
         << "Connection: Upgrade" << CRLF()
         << "Sec-WebSocket-Accept: " << challengeResponse << CRLF()
         << CRLF();
      
      sendString(ss.str());
      handshakeComplete = true;
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

