#include <iostream>
#include <charconv>
#include <array>

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
  bool fragmentedData{false};
  bool isBinary{false};
  std::vector<uint8_t> receivedBytes;
  std::vector<uint8_t> fragmentedBytes;
  
  virtual DataResult handleIncommingData(int8_t* data, uint32_t bytes) override {
    if (handshakeComplete) {
      receivedBytes.insert(receivedBytes.end(), data, data+bytes);
      return handleFrame();
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
            return DataResult::NO_DATA;
          }
        }
      }
    }
    return DataResult::NO_DATA;
  }
  
  virtual void sendMessage(std::string message) override {
    HttpClientConnection::sendString(genFrame(message));
    HttpClientConnection::sendString(message);
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
  
  void unmask(size_t nextByte, size_t payloadLength, const std::array<uint8_t, 4>& mask) {
    for (size_t i = 0;i<payloadLength;++i) {
      receivedBytes[i+nextByte] = receivedBytes[i+nextByte] ^ mask[i%4];
    }
  }
  
  DataResult generateResult(bool finalFragment, size_t nextByte, uint64_t payloadLength) {
    DataResult result;
    if (!finalFragment) {
      if (fragmentedBytes.size() > std::numeric_limits<size_t>::max() - payloadLength) {
        connectionSocket->Close();
        return DataResult::NO_DATA;
      }
      fragmentedData = true;
      fragmentedBytes.insert(fragmentedBytes.end(), receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength));
      result = DataResult::NO_DATA;
    } else {
      if (fragmentedData) {
        if (fragmentedBytes.size() > std::numeric_limits<size_t>::max() - payloadLength) {
          connectionSocket->Close();
          return DataResult::NO_DATA;
        }
        fragmentedBytes.insert(fragmentedBytes.end(), receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength));
        if (isBinary) {
          binData = std::vector<uint8_t>{receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength)};
          result = DataResult::BINARY_DATA;
        } else {
          strData = std::string{fragmentedBytes.begin(), fragmentedBytes.end()};
          result = DataResult::STRING_DATA;
        }
        fragmentedData = false;
        fragmentedBytes.clear();
      } else {
        if (isBinary) {
          binData = std::vector<uint8_t>{receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength)};
          result = DataResult::BINARY_DATA;
        } else {
          strData = std::string{receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength)};
          result = DataResult::STRING_DATA;
        }
      }
    }
    receivedBytes.erase(receivedBytes.begin(), receivedBytes.begin()+long(nextByte+payloadLength));
    return result;
  }
  
  DataResult handleFrame() {
    if (receivedBytes.size() < 6) {
      return DataResult::NO_DATA;
    }
    
    const std::bitset<8> firstByte{receivedBytes[0]};
    const std::bitset<8> secondByte{receivedBytes[1]};
    
    const bool finalFragment{firstByte[7]};
    const bool isMasked{secondByte[7]};
    const uint8_t opcode = receivedBytes[0] & 0b00001111;

    if (!isMasked) {
      connectionSocket->Close();
      return DataResult::NO_DATA;
    }

    switch (opcode) {
      case 0x0 : break;
      case 0x1 : isBinary = false; break;
      case 0x2 : isBinary = true; break;
    }
    
    size_t nextByte{2};
    uint64_t payloadLength = receivedBytes[1] & 0b01111111;
    if (payloadLength == 126) {
      payloadLength = ((uint64_t)receivedBytes[2] << 8) | (uint64_t)receivedBytes[3];
      nextByte += 2;
    } else if (payloadLength == 127) {
      payloadLength = ((uint64_t)receivedBytes[2] << 56) | ((uint64_t)receivedBytes[3] << 48) |
                      ((uint64_t)receivedBytes[4] << 40) | ((uint64_t)receivedBytes[5] << 32) |
                      ((uint64_t)receivedBytes[6] << 24) | ((uint64_t)receivedBytes[7] << 16) |
                      ((uint64_t)receivedBytes[8] << 8)  | (uint64_t)receivedBytes[9];
      nextByte += 8;
    }
    
    if (payloadLength > std::numeric_limits<size_t>::max() - (nextByte+4)) {
      connectionSocket->Close();
      return DataResult::NO_DATA;
    }
        
    if (receivedBytes.size() < nextByte+4+payloadLength) {
      return DataResult::NO_DATA;
    }
    
    const std::array<uint8_t, 4> mask {
      receivedBytes[nextByte+0],
      receivedBytes[nextByte+1],
      receivedBytes[nextByte+2],
      receivedBytes[nextByte+3]
    };
    nextByte += 4;
    
    unmask(nextByte, payloadLength, mask);
    return generateResult(finalFragment, nextByte, payloadLength);
  }

  std::string genFrame(const std::string& message) {
    // TODO
    return "";
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
    std::cout << "Str: " << message << std::endl;
  }

  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override {
    std::cout << "Bin: " << std::hex;
    for (size_t i = 0;i<message.size();++i) {
      std::cout << int(message[i]) << " ";
    }
    std::cout << std::dec << std::endl;
  }

};


int main(int argc, char ** argv) {
  EchoServer s(8899);
  s.start();
  
  while (true) {
    
  }
  
  return EXIT_SUCCESS;
}

