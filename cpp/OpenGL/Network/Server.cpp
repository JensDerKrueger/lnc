#include "Server.h"

BaseClientConnection::BaseClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  connectionSocket(connectionSocket),
  id(id),
  key(key),
  timeout(timeout)
{
  sendThread = std::thread(&SizedClientConnection::sendFunc, this);
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

void BaseClientConnection::sendFunc() {
  while (continueRunning) {
    try {
      if (!messageQueue.empty()) {
        messageQueueLock.lock();
        std::string front = messageQueue.front();
        messageQueue.pop();
        messageQueueLock.unlock();
        sendMessage(front);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    } catch (SocketException const& e) {
      std::cerr << "sendFunc SocketException: " << e.what() << std::endl;
      continue;
    } catch (AESException const& e) {
      std::cerr << "encryption error: " << e.what() << std::endl;
      continue;
    }
  }
}

std::string BaseClientConnection::checkData() {
  int8_t data[2048];
  const uint32_t bytes = connectionSocket->ReceiveData(data, 2048, 1);
  if (bytes > 0) {
    return handleIncommingData(data, bytes);
  }
  return "";
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

SizedClientConnection::SizedClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  BaseClientConnection(connectionSocket, id, key, timeout),
  crypt(nullptr),
  sendCrypt(nullptr)
{
}

std::string SizedClientConnection::checkData() {
  int8_t data[2048];
  const uint32_t maxSize = std::min<uint32_t>(std::max<uint32_t>(4,messageLength),2048);
  const uint32_t bytes = connectionSocket->ReceiveData(data, maxSize, 1);
  if (bytes > 0) {
    return handleIncommingData(data, bytes);
  }
  return "";
}

void SizedClientConnection::sendMessage(std::string message) {
  if (!key.empty()) {
    if (sendCrypt) {
      message = sendCrypt->encryptString(message);
    } else {
      AESCrypt tempCrypt("1234567890123456",key);
      std::string iv = AESCrypt::genIVString();
      std::string initMessage = tempCrypt.encryptString(genHandshake(iv, key));
      sendCrypt = std::make_unique<AESCrypt>(iv,key);
      sendRawMessage(initMessage);
      message = sendCrypt->encryptString(message);
    }
  }
  sendRawMessage(message);
}

std::string SizedClientConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
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
    
    if (key.empty()) {
      return os.str();
    } else {
      if (crypt) {
        return crypt->decryptString(os.str());
      } else {
        AESCrypt tempCrypt("1234567890123456",key);
        const std::string firstMessage = tempCrypt.decryptString(os.str());
        const std::string iv = getIVFromHandshake(firstMessage, key);
        crypt = std::make_unique<AESCrypt>(iv,key);
        return "";
      }
    }
  }
  return "";
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
      std::cerr << "lost data while trying to send " << size << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")" << std::endl;
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
    std::cerr << "lost data truncating long message" << std::endl;
  }
  
  std::vector<uint8_t> data = intToVec(l);

  sendRawMessage((int8_t*)data.data(), 4);
  sendRawMessage(rawData.data(), l);
}


void SizedClientConnection::sendRawMessage(std::string message) {
  const uint32_t l = uint32_t(message.length());
  if (l != message.length()) {
    std::cerr << "lost data truncating long message" << std::endl;
  }

  std::vector<uint8_t> data = intToVec(l);
  sendRawMessage((int8_t*)data.data(), 4);
  const int8_t* cStr = (int8_t*)(message.c_str());
  sendRawMessage(cStr, l);
}


HttpClientConnection::HttpClientConnection(TCPSocket* connectionSocket, uint32_t id, const std::string& key, uint32_t timeout) :
  BaseClientConnection(connectionSocket, id, key, timeout)
{
}
  
std::string HttpClientConnection::handleIncommingData(int8_t* data, uint32_t bytes) {
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

void HttpClientConnection::sendMessage(std::string message) {
  
  std::stringstream ss;
  
  // write header
  ss << "HTTP/1.1 200 OK\n\r"
     << "Server: LNC-Server\n\r"
     << "Accept-Ranges: bytes\n\r"
     << "Content-Type: text/plain\n\r\n\r";
  
  ss << message;

  message = ss.str();
  
  uint32_t currentBytes = 0;
  uint32_t totalBytes = 0;

  try {
    do {
      currentBytes = connectionSocket->SendData((int8_t*)(message.c_str()) + totalBytes, uint32_t(message.length())-totalBytes, timeout);
      totalBytes += currentBytes;
    } while (currentBytes > 0 && totalBytes < message.length());

    if (currentBytes == 0 && totalBytes < message.length()) {
      std::cerr << "lost data while trying to send " << message.length() << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")" << std::endl;
    }
  } catch (SocketException const&  ) {
  }
  
  connectionSocket->Close();
}
