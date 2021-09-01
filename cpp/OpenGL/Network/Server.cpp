#include <algorithm>
#include <bitset>
#include <climits>
#include <array>

#include "Server.h"

#include "StringTools.h"
#include <SHA1.h>

constexpr uint64_t MAX_PAYLOAD_SIZE = 1024*1024*1024;

BaseClientConnection::BaseClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key,
                                           uint32_t timeout, ErrorFunction errorFunction) :
  connectionSocket(connectionSocket),
  id(id),
  key(key),
  timeout(timeout),
  lastResult{DataResult::NO_DATA},
  errorFunction{errorFunction}
{
  sendThread = std::thread(&BaseClientConnection::sendFunc, this);
}

BaseClientConnection::~BaseClientConnection() {
  continueRunning = false;
  sendThread.join();

  try {
    if (connectionSocket && connectionSocket->IsConnected()) {
      connectionSocket->Close();
    }
    delete connectionSocket;
    connectionSocket = nullptr;
  } catch (SocketException const&  ) {
  }
}

bool BaseClientConnection::isConnected() {
  return connectionSocket && connectionSocket->IsConnected();
}

void BaseClientConnection::enqueueMessage(const std::string& m) {
  messageQueueLock.lock();
  messageQueue.push(m);
  messageQueueLock.unlock();
}

void BaseClientConnection::enqueueMessage(const std::vector<uint8_t>& m) {
  messageQueueLock.lock();
  messageQueue.push(m); //-> msvc-build + /GS wirft hier _gs_security-exception .. leider nur manchmal, grund noch unbekannt
  messageQueueLock.unlock();
}

void BaseClientConnection::sendFunc() {
  while (continueRunning) {
    if (!messageQueue.empty() && handshakeComplete) {
      
      messageQueueLock.lock();
              
      const auto frontElement = std::move(messageQueue.front());
      
      if (std::holds_alternative<std::string>(frontElement)) {
        const std::string front = std::get<std::string>(frontElement);
        messageQueue.pop();
        messageQueueLock.unlock();
        try {
			if (continueRunning) //dtor - join.
			{
				sendMessage(front);
			}
        } catch (SocketException const& e) {
          std::stringstream ss;
          ss << "sendFunc SocketException: " << e.what();
          errorFunction(ss.str());
          continue;
        } catch (AESException const& e) {
          std::stringstream ss;
          ss << "encryption error: " << e.what();
          errorFunction(ss.str());
          continue;
        }

      } else {
        const std::vector<uint8_t> front = std::get<std::vector<uint8_t>>(frontElement);
        messageQueue.pop();
        messageQueueLock.unlock();
        try {
			if (continueRunning) //dtor - join.
			{ 
				sendMessage(front);
			}
        } catch (SocketException const& e) {
          std::stringstream ss;
          ss << "sendFunc SocketException: " << e.what();
          errorFunction(ss.str());
          continue;
        } catch (AESException const& e) {
          std::stringstream ss;
          ss << "encryption error: " << e.what();
          errorFunction(ss.str());
          continue;
        }
      }
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

DataResult BaseClientConnection::checkData() {
  int8_t data[2048];
  const uint32_t bytes = connectionSocket->ReceiveData(data, 2048, 1);
  if (bytes > 0 || lastResult != DataResult::NO_DATA) {
    lastResult = handleIncommingData(data, bytes);
  } else {
    lastResult = DataResult::NO_DATA;
  }
  return lastResult;
}

std::string BaseClientConnection::getPeerAddress() const {
  if (connectionSocket && connectionSocket->IsConnected()) {
    return connectionSocket->GetPeerAddress();
  } else {
    return "";
  }
}
uint16_t BaseClientConnection::getPeerPort() const {
  if (connectionSocket && connectionSocket->IsConnected()) {
    return connectionSocket->GetPeerPort();
  } else {
    return 0;
  }
}

SizedClientConnection::SizedClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction) :
  BaseClientConnection(connectionSocket, id, key, timeout, errorFunction),
  crypt(nullptr),
  sendCrypt(nullptr)
{
  handshakeComplete = true;
}

DataResult SizedClientConnection::checkData() {
  int8_t data[2048];
  const uint32_t maxSize = std::clamp<uint32_t>(messageLength, 4, 2048);
  const uint32_t bytes = connectionSocket->ReceiveData(data, maxSize, 1);
  if (bytes > 0) {
    return handleIncommingData(data, bytes);
  }
  return DataResult::NO_DATA;
}

void SizedClientConnection::sendMessage(const std::string& message) {
  if (!key.empty()) {
    if (!sendCrypt) {
      AESCrypt tempCrypt("1234567890123456",key);
      std::string iv = AESCrypt::genIVString();
      std::string initMessage = tempCrypt.encryptString(genHandshake(iv, key));
      sendCrypt = std::make_unique<AESCrypt>(iv,key);
      sendRawMessage(initMessage);
    }
    sendRawMessage(sendCrypt->encryptString(message));
  } else {
    sendRawMessage(message);
  }
}

DataResult SizedClientConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
  if (bytes > 0) receivedBytes.insert(receivedBytes.end(), data, data+bytes);

  if (messageLength == 0) {
    if (receivedBytes.size() >= 4) {
      messageLength = *((uint32_t*)receivedBytes.data());
    } else {
      return DataResult::NO_DATA;
    }
  }
  
  if (receivedBytes.size() >= messageLength+4) {
    std::ostringstream os;
    for (size_t i = 4;i<messageLength+4;++i) {
      os << receivedBytes[i];
    }
    receivedBytes.erase(receivedBytes.begin(), receivedBytes.begin() + messageLength+4);
    messageLength = 0;
    
    if (key.empty()) {
      strData = os.str();
      return DataResult::STRING_DATA;
    } else {
      if (crypt) {
        strData = crypt->decryptString(os.str());
        return DataResult::STRING_DATA;
      } else {
        AESCrypt tempCrypt("1234567890123456",key);
        const std::string firstMessage = tempCrypt.decryptString(os.str());
        const std::string iv = getIVFromHandshake(firstMessage, key);
        crypt = std::make_unique<AESCrypt>(iv,key);
        return DataResult::NO_DATA;
      }
    }
  }
  return DataResult::NO_DATA;
}

void SizedClientConnection::sendRawMessage(const int8_t* rawData, uint32_t size) {
  uint32_t currentBytes = 0;
  uint32_t totalBytes = 0;

  try {
    do {
      currentBytes = connectionSocket->SendData(rawData + totalBytes, size-totalBytes, timeout);
      totalBytes += currentBytes;
    } while (currentBytes > 0 && totalBytes < size);

    if (currentBytes == 0 && totalBytes < size) {
      std::stringstream ss;
      ss << "lost data while trying to send " << size << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")";
      errorFunction(ss.str());
    }
  } catch (SocketException const&  ) {
  }
}

std::vector<uint8_t> SizedClientConnection::intToVec(uint32_t i) const {
  std::vector<uint8_t> data(4);
  data[0] = i%256; i /= 256;
  data[1] = i%256; i /= 256;
  data[2] = i%256; i /= 256;
  data[3] = i;
  return data;
}

void SizedClientConnection::sendRawMessage(std::vector<int8_t> rawData) {
  uint32_t l = uint32_t(rawData.size());
  if (l != rawData.size()) {
    errorFunction("lost data truncating long message");
  }
  
  std::vector<uint8_t> data = intToVec(l);

  sendRawMessage((int8_t*)data.data(), 4);
  sendRawMessage(rawData.data(), l);
}


void SizedClientConnection::sendRawMessage(std::string message) {
  const uint32_t l = uint32_t(message.length());
  if (l != message.length()) {
    errorFunction("lost data truncating long message");
  }

  std::vector<uint8_t> data = intToVec(l);
  sendRawMessage((int8_t*)data.data(), 4);
  const int8_t* cStr = (int8_t*)(message.c_str());
  sendRawMessage(cStr, l);
}


HttpClientConnection::HttpClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction) :
  BaseClientConnection(connectionSocket, id, key, timeout, errorFunction)
{
  handshakeComplete = true;
}
  
DataResult HttpClientConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
  if (receivedBytes.size() > MAX_PAYLOAD_SIZE) {
    receivedBytes.clear();
  }

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
        strData = ss.str();
        return DataResult::STRING_DATA;
      }
    }
  }
  return DataResult::NO_DATA;
}


HTTPRequest HttpClientConnection::parseHTTPRequest(const std::string& initialMessage) {
  std::vector<std::string> lines = tokenize(initialMessage, CRLF());
  
  if (lines.empty()) return {};
  
  HTTPRequest result;
  std::vector<std::string> values = tokenize(lines[0], " ");
  if (values.size() != 3) return {};
  
  result.name = trim(values[0]);
  result.target = trim(values[1]);
  result.version = trim(values[2]);
  //CHECK: Value könnte doppelpunkte enthalten?    
  for (size_t i = 1;i<lines.size();++i) {
    std::vector<std::string> values = tokenize(lines[i], ":");
    if (values.size() == 2) {
      result.parameters[trim(toLower(values[0]))] = trim(values[1]);
    }
  }
  return result;
}

void HttpClientConnection::sendData(const std::vector<uint8_t>& message) {
  uint32_t currentBytes = 0;
  uint32_t totalBytes = 0;

  try {
    do {
      currentBytes = connectionSocket->SendData((int8_t*)(message.data()) + totalBytes, uint32_t(message.size())-totalBytes, timeout);
      totalBytes += currentBytes;
    } while (currentBytes > 0 && totalBytes < message.size());

    if (currentBytes == 0 && totalBytes < message.size()) {
      std::stringstream ss;
      ss << "lost data while trying to send " << message.size() << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")";
      errorFunction(ss.str());
    }
  } catch (SocketException const&  ) {
  }
}

void HttpClientConnection::sendString(const std::string& message) {
  uint32_t currentBytes = 0;
  uint32_t totalBytes = 0;

  try {
    do {
      currentBytes = connectionSocket->SendData((int8_t*)(message.c_str()) + totalBytes, uint32_t(message.length())-totalBytes, timeout);
      totalBytes += currentBytes;
    } while (currentBytes > 0 && totalBytes < message.length());

    if (currentBytes == 0 && totalBytes < message.length()) {
      std::stringstream ss;
      ss << "lost data while trying to send " << message.length() << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")";
      errorFunction(ss.str());
    }
  } catch (SocketException const&  ) {
  }
}
  
void HttpClientConnection::sendMessage(const std::string& message) {
  std::stringstream ss;
  ss << "HTTP/1.1 200 OK" << CRLF()
     << "Server: LNC-Server" << CRLF()
     << "Accept-Ranges: bytes" << CRLF()
     << "Content-Type: text/plain" << CRLF() << CRLF();
  ss << message;
  sendString(ss.str());
  try {
    connectionSocket->Close();
  } catch (SocketException const&  ) {
  }
}

WebSocketConnection::WebSocketConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout, ErrorFunction errorFunction) :
  HttpClientConnection(connectionSocket, id, key, timeout, errorFunction),
  currentOpcode(0x0)
{
  handshakeComplete = false;
}

WebSocketConnection::~WebSocketConnection() {
  closeWebsocket(CloseReason::NormalClosure);
}

DataResult WebSocketConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
  if (receivedBytes.size() > MAX_PAYLOAD_SIZE) {
    closeWebsocket(CloseReason::MessageTooBig);
    return DataResult::NO_DATA;
  }

  if (handshakeComplete) {
    if (bytes > 0) receivedBytes.insert(receivedBytes.end(), data, data+bytes);
    return handleFrame();
  } else {
    if (bytes > 0) receivedBytes.insert(receivedBytes.end(), data, data+bytes);
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

void WebSocketConnection::sendMessage(const std::string& message) {
  HttpClientConnection::sendData(genFrame(message));
  HttpClientConnection::sendString(message);
}

void WebSocketConnection::sendMessage(const std::vector<uint8_t>& message) {
  HttpClientConnection::sendData(genFrame(message));
  HttpClientConnection::sendData(message);
}

void WebSocketConnection::closeWebsocket(uint16_t reasonCode) {
  const uint16_t iReason = (isBigEndian()) ? swapEndian(reasonCode) : reasonCode;
  
  HttpClientConnection::sendData(genFrame(2,0x8));
  std::vector<uint8_t> data{
    uint8_t((iReason >> 8) & 0xFF),
    uint8_t((iReason >> 0) & 0xFF)
  };
  HttpClientConnection::sendData(data);
  try {
    connectionSocket->Close();
  } catch (SocketException const&  ) {
  }
}

void WebSocketConnection::closeWebsocket(CloseReason reason) {
  closeWebsocket(uint16_t(reason));
}

void WebSocketConnection::handleHandshake(const std::string& initialMessage) {
  HTTPRequest request = parseHTTPRequest(initialMessage);
  if (request.name == "GET" &&
      toLower(request.parameters["upgrade"]) == "websocket") {
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

void WebSocketConnection::unmask(size_t nextByte, size_t payloadLength, const std::array<uint8_t, 4>& mask) {
  for (size_t i = 0;i<payloadLength;++i) {
    receivedBytes[i+nextByte] = receivedBytes[i+nextByte] ^ mask[i%4];
  }
}

DataResult WebSocketConnection::generateResult(bool finalFragment, size_t nextByte, uint64_t payloadLength) {
  DataResult result;
  if (!finalFragment) {
    if (fragmentedBytes.size() > std::numeric_limits<size_t>::max() - payloadLength) {
      errorFunction("Message too big");
      closeWebsocket(CloseReason::MessageTooBig);
      return DataResult::NO_DATA;
    }
    fragmentedData = true;
    fragmentedBytes.insert(fragmentedBytes.end(), receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength));
    result = DataResult::NO_DATA;
  } else {
    if (fragmentedData) {
      if (fragmentedBytes.size() > std::numeric_limits<size_t>::max() - payloadLength) {
        errorFunction("Message too big");
        closeWebsocket(CloseReason::MessageTooBig);
        return DataResult::NO_DATA;
      }
      fragmentedBytes.insert(fragmentedBytes.end(), receivedBytes.begin()+long(nextByte), receivedBytes.begin()+long(nextByte+payloadLength));
      if (isBinary) {
        binData = std::move(fragmentedBytes);
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
    
    if (currentOpcode > 7) {
      protocolDataID = currentOpcode;
      switch (currentOpcode) {
        case 0x8:
          result = DataResult::NO_DATA;
          closeWebsocket(extractReasonCode());
          return result;
          break;
        case 0x9:
          result = DataResult::NO_DATA;
          sendPong();
          break;
        case 0xA:
          result = DataResult::NO_DATA;
          break;
        default:
          result = DataResult::PROTOCOL_DATA;
          break;
      }
    }

  }
  receivedBytes.erase(receivedBytes.begin(), receivedBytes.begin()+long(nextByte+payloadLength));
  return result;
}

uint16_t WebSocketConnection::extractReasonCode() {
  if (binData.size() >= 2) {
    return (uint16_t(binData[0]) >> 8) + binData[1];
  } else {
    return uint16_t(CloseReason::NormalClosure);
  }
}

void WebSocketConnection::sendPong() {
  HttpClientConnection::sendData(genFrame(binData.size(), 0x0A));
  HttpClientConnection::sendData(binData);
}


DataResult WebSocketConnection::handleFrame() {
  if (receivedBytes.size() < 6) {
    return DataResult::NO_DATA;
  }
  
  const std::bitset<8> firstByte{receivedBytes[0]};
  const std::bitset<8> secondByte{receivedBytes[1]};
  
  const bool finalFragment{firstByte[7]};
  const bool isMasked{secondByte[7]};
  const uint8_t opcode = receivedBytes[0] & 0b00001111;

  if (!isMasked) {
    errorFunction("ProtocolError");
    closeWebsocket(CloseReason::ProtocolError);
    return DataResult::NO_DATA;
  }

  if (opcode != 0) {
    currentOpcode = opcode;
    isBinary = (opcode != 0x1);
  }
  
  size_t nextByte{2};
  uint64_t payloadLength = receivedBytes[1] & 0b01111111;
  
  if (payloadLength == 126) {
    payloadLength = ((uint64_t)receivedBytes[2] << 8) | (uint64_t)receivedBytes[3];
    nextByte += 2;
  } else if (payloadLength == 127) {
    if (receivedBytes.size() < 10) {
      return DataResult::NO_DATA;
    }

    payloadLength = ((uint64_t)receivedBytes[2] << 56) | ((uint64_t)receivedBytes[3] << 48) |
                    ((uint64_t)receivedBytes[4] << 40) | ((uint64_t)receivedBytes[5] << 32) |
                    ((uint64_t)receivedBytes[6] << 24) | ((uint64_t)receivedBytes[7] << 16) |
                    ((uint64_t)receivedBytes[8] << 8)  | (uint64_t)receivedBytes[9];
    nextByte += 8;
  }
  
  if (isBigEndian()) payloadLength = swapEndian(payloadLength);
  
  if (payloadLength > MAX_PAYLOAD_SIZE) {
    errorFunction("Message too big");
    closeWebsocket(CloseReason::MessageTooBig);
    return DataResult::NO_DATA;
  }
  
  // this code is here just for safety reasons, normally the previous line
  // will bail on large frames already
  if (payloadLength > std::numeric_limits<size_t>::max() - (nextByte+4)) {
    errorFunction("Message too big");
    closeWebsocket(CloseReason::MessageTooBig);
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

std::vector<uint8_t> WebSocketConnection::genFrame(uint64_t s, uint8_t code) {
  if (isBigEndian()) s = swapEndian(s);
  
  std::vector<uint8_t> frame(2);
  
  frame[0] = 1 << 7 |
             0 << 6 |
             0 << 5 |
             0 << 4 |
             (code & 0b00001111);

  if (s < 126) {
    frame[1] = 0 << 7 |
               uint8_t(s & 0b01111111);
  } else {
    if (s <= (1 << 16)) {
      frame[1] = 0 << 7 | 126;
    } else {
      frame[1] = 0 << 7 | 127;
      frame.push_back(uint8_t((s >> 56) & 0xFF));
      frame.push_back(uint8_t((s >> 48) & 0xFF));
      frame.push_back(uint8_t((s >> 40) & 0xFF));
      frame.push_back(uint8_t((s >> 32) & 0xFF));
      frame.push_back(uint8_t((s >> 24) & 0xFF));
      frame.push_back(uint8_t((s >> 16) & 0xFF));
    }
    frame.push_back(uint8_t((s >> 8) & 0xFF));
    frame.push_back(uint8_t((s >> 0) & 0xFF));
  }
  
  return frame;
}

std::vector<uint8_t> WebSocketConnection::genFrame(const std::string& message) {
  return genFrame(message.length(), 0x01);
}

std::vector<uint8_t> WebSocketConnection::genFrame(const std::vector<uint8_t>& message) {
  return genFrame(message.size(), 0x02);
}
