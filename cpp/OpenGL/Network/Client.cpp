#include <sstream>
#include <iostream>

#include "Client.h"

Client::Client(const std::string& address, short port, const std::string& key, uint32_t timeout) :
  address{address},
  port{port},
  timeout{timeout},
  crypt(nullptr),
  receiveCrypt(nullptr),
  key(key)
{
  clientThread = std::thread(&Client::clientFunc, this);
}

Client::~Client() {
  continueRunning = false;
  clientThread.join();
}

void Client::sendMessage(const std::string& message) {
  sendMessageMutex.lock();
  sendMessages.push_back(message);
  sendMessageMutex.unlock();
}

void Client::shutdownClient() {
  connecting = false;
  if (connection) {
    try {
      connection->Close();
    } catch (SocketException const&  ) {
    }
  }
  ok = false;
}

std::string Client::handleIncommingData(int8_t* data, uint32_t bytes) {
  
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
      if (receiveCrypt) {
        return receiveCrypt->decryptString(os.str());
      } else {
        AESCrypt tempCrypt("1234567890123456",key);
        const std::string firstMessage = tempCrypt.decryptString(os.str());
        const std::string iv = getIVFromHandshake(firstMessage, key);
        receiveCrypt = std::make_unique<AESCrypt>(iv,key);
        return "";
      }
    }

  }
  
  return "";
}


void Client::sendRawMessage(const int8_t* rawData, uint32_t size) {
  uint32_t currentBytes = 0;
  uint32_t totalBytes = 0;

  try {
    do {
      currentBytes = connection->SendData(rawData + totalBytes, size-totalBytes, timeout);
      totalBytes += currentBytes;
    } while (currentBytes > 0 && totalBytes < size);

    if (currentBytes == 0 && totalBytes < size) {
      std::cerr << "lost data while trying to send " << size << " (actually send:" << totalBytes << ", timeout:" <<  timeout << ")" << std::endl;
    }
  } catch (SocketException const&  ) {
  }
}

std::vector<uint8_t> Client::intToVec(uint32_t i) const {
  std::vector<uint8_t> data(4);
  data[0] = i%256; i /= 256;
  data[1] = i%256; i /= 256;
  data[2] = i%256; i /= 256;
  data[3] = i;
  return data;
}


void Client::sendRawMessage(std::vector<int8_t> rawData) {
  uint32_t l = uint32_t(rawData.size());
  if (l != rawData.size()) {
    std::cerr << "lost data truncating long message" << std::endl;
  }
  
  std::vector<uint8_t> data = intToVec(l);

  sendRawMessage((int8_t*)data.data(), 4);
  sendRawMessage(rawData.data(), l);
}


void Client::sendRawMessage(std::string message) {
  const uint32_t l = uint32_t(message.length());
  if (l != message.length()) {
    std::cerr << "lost data truncating long message" << std::endl;
  }

  std::vector<uint8_t> data = intToVec(l);
  sendRawMessage((int8_t*)data.data(), 4);
  const int8_t* cStr = (int8_t*)(message.c_str());
  sendRawMessage(cStr, l);
}

void Client::clientFunc() {
  while (continueRunning) {
    
    connecting = true;
    crypt = nullptr;
    receiveCrypt = nullptr;
    
    if (connection && connection->IsConnected()) {
      try {
        connection->Close();
      } catch (SocketException const& ) {
      }
    }
    
    connection = std::make_shared<TCPSocket>();
    connection->SetNonBlocking(timeout == INFINITE_TIMEOUT ? false : true);
    connection->SetNoDelay(true);
    connection->SetKeepalive(false);
    connection->SetNoSigPipe(true);
    NetworkAddress nwaddress(address, port);

    try {
      while (continueRunning && !connection->Connect(nwaddress, timeout)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10) );
      }
    } catch (SocketException const&) {
      continue;
    }
    
    if (!continueRunning) break;

    handleNewConnection();
    ok = true;
    connecting = false;

    // send/receive data loop
    try {
      while (continueRunning && connection->IsConnected()) {
        bool receivedData{ false };
        try {
          int8_t data[2048];
          const uint32_t maxSize = std::min<uint32_t>(std::max<uint32_t>(4,messageLength),2048);
          const uint32_t bytes = connection->ReceiveData(data, maxSize, 1);
          if (bytes > 0) {
            receivedData = true;
            std::string message = handleIncommingData(data, bytes);
            if (!message.empty() && continueRunning) handleServerMessage(message);
          }
        } catch (SocketException const& ) {
        }

        sendMessageMutex.lock();
        if (!sendMessages.empty()) {
          
          std::string message = sendMessages[0];
          if (key.empty()) {
            sendMessages.erase(sendMessages.begin(), sendMessages.begin()+1);
          } else {
            if (crypt) {
              message = crypt->encryptString(message);
              sendMessages.erase(sendMessages.begin(), sendMessages.begin()+1);
            } else {
              AESCrypt tempCrypt("1234567890123456",key);
              std::string iv = AESCrypt::genIVString();
              message = tempCrypt.encryptString(genHandshake(iv, key));
              crypt = std::make_unique<AESCrypt>(iv,key);
            }
          }
          
          sendRawMessage(message);
           
        } else {
          if (!receivedData) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        sendMessageMutex.unlock();
      }
    } catch (SocketException const& ) {
      continue;
    }
  }
  
  shutdownClient();
}

size_t Client::cueSize() {
  sendMessageMutex.lock();
  size_t s = sendMessages.size();
  sendMessageMutex.unlock();
  return s;
}
