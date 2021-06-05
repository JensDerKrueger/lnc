#include <iostream>
#include <cstring>
#include <algorithm>
#include <cctype>

#include <Server.h>
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
  
  std::string trimLeft(const std::string& input) {
    std::string result = input;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {return !std::isspace(ch);}));
    return result;
  }

  std::string trimRight(const std::string& input) {
    std::string result = input;
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
    return result;
  }

  std::string trim(const std::string& input) {
    return trimLeft(trimRight(input));
  }
  
  std::string toLower(const std::string& input) {
    std::string result = input;
    std::transform(input.begin(), input.end(), result.begin(), [](int c) { return std::tolower(c); });
    return result;
  }

  std::string toUpper(const std::string& input) {
    std::string result = input;
    std::transform(input.begin(), input.end(), result.begin(), [](int c) { return std::toupper(c); });
    return result;
  }
  
  std::vector<std::string> tokenizer(const std::string& input, const std::string& delim) {
    char* cstring = strdup(input.c_str());
    char* token = std::strtok(cstring, delim.c_str());
    std::vector<std::string> result;
    while (token) {
      result.push_back(token);
      token = std::strtok(nullptr,  delim.c_str());
    }
    free(cstring);
    return result;
  }
  
  void handleHandshake(const std::string& initialMessage) {
    std::vector<std::string> lines = tokenizer(initialMessage, "\r\n");
    
    for (const std::string& line: lines) {
      std::cout << "->" << line << "<-" << std::endl;
      
      std::vector<std::string> values = tokenizer(line, ":");
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

