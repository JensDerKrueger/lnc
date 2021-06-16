#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <chrono>

#include <Server.h>
#include <bmp.h>

typedef std::chrono::high_resolution_clock Clock;
std::chrono::time_point<Clock> restorePointTime = Clock::now();

constexpr const uint16_t canvasWidth  = 1000;
constexpr const uint16_t canvasHeight = 1000;
constexpr const uint8_t  layer        = 1;

class EchoServer : public Server<WebSocketConnection> {
public:
  EchoServer(uint16_t port) :
  Server(port)
  {
    imageMessage.resize(size_t(canvasWidth)*size_t(canvasHeight)*4*size_t(layer)+5);
    imageMessage[0] = 0;
    imageMessage[1] = (canvasWidth >> 8) & 0xff;
    imageMessage[2] = canvasWidth & 0xff;
    imageMessage[3] = (canvasHeight >> 8) & 0xff;
    imageMessage[4] = canvasHeight & 0xff;
     
    loadPaintLayer("paint.bmp");
  }
  
  virtual ~EchoServer() {
    savePaintLayer("paint.bmp");
  }
  
  void savePaintLayer(const std::string& filename) {
    std::cout << "Saving paint" << std::endl;
    imageLock.lock();
    std::vector<uint8_t> data{imageMessage.begin()+long(5), imageMessage.begin()+long(5+size_t(canvasWidth)*size_t(canvasHeight)*4)};
    imageLock.unlock();

    try {
      BMP::save(filename, Image(canvasWidth, canvasHeight, 4, data));
      std::cout << "Paint saved" << std::endl;
    } catch (...) {
      std::cerr << "Paint could not be saved" << std::endl;
    }
  }

  void loadPaintLayer(const std::string& filename) {
    std::cout << "Loading paint" << std::endl;
    try {
      Image p = BMP::load(filename);
      if (canvasWidth == p.width && canvasHeight == p.height && 4 == p.componentCount) {

        for (size_t i = 0;i<size_t(canvasWidth)*size_t(canvasHeight)*4;++i) {
          imageMessage[5+i] = p.data[i];
        }
        
        std::cout << "Paint Loaded" << std::endl;
      } else {
        throw BMP::BMPException("Invalid Image dimensions");
      }
    } catch (...) {
      std::fill(imageMessage.begin()+5, imageMessage.end(), 255);
      std::cerr << "Paint could not be loaded" << std::endl;
    }
  }

  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    std::cerr << "Error: Client (id:" << id << ") send a string message" << std::endl;
  }

  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override {
    imageLock.lock();
    paint(message);
    imageLock.unlock();
    sendMessage(message);
  }

  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override {
    std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
    printStats();
    sendMessage(imageMessage,id);
  }

  virtual void handleClientDisconnection(uint32_t id) override {
    std::cout << "Client (id:" << id << ") disconnected" << std::endl;
    printStats();
  }
  
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override {
    std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
  }

  virtual void printStats() {
    std::cout << "A total of " << getValidIDs().size() << " clients are currently connected." << std::endl;
  }
  
  virtual void handleError(const std::string& message) override {
    std::cerr << "Error: " << message << std::endl;
  }
  
private:
  std::vector<uint8_t> imageMessage;
  
  static uint16_t to16Bit(const std::vector<uint8_t>& message, size_t index) {
    return uint16_t(uint16_t(message[index]) << 8) | message[index+1];
  }
  
  void paint(const std::vector<uint8_t>& message) {
    if (message.size() == 10 || message[0] == 1) {
      const uint16_t brushCenterPosX = to16Bit(message, 1);
      const uint16_t brushCenterPosY = to16Bit(message, 3);
      const uint8_t r = message[5];
      const uint8_t g = message[6];
      const uint8_t b = message[7];
      const uint16_t brushSize = std::clamp<uint16_t>(to16Bit(message, 8),1,20);
      for (int32_t y = 0;y<brushSize;++y) {
        for (int32_t x = 0;x<brushSize;++x) {
          const int32_t posX = brushCenterPosX+x;
          const int32_t posY = brushCenterPosY+y;
          if (posX >= 0 && posX < canvasWidth &&
              posY >= 0 && posY < canvasHeight) {
            const size_t serialPos = size_t((posX + posY * canvasWidth)*4+5);
            imageMessage[serialPos+0] = r;
            imageMessage[serialPos+1] = g;
            imageMessage[serialPos+2] = b;
            imageMessage[serialPos+3] = 255;
          }
        }
      }
    }
  }
  
private:
  std::mutex imageLock;

};


static std::string getAnswer() {
  std::string answer;
  std::cout << "> " << std::flush;
  std::cin >> answer;
  return answer;
}

int main(int argc, char ** argv) {
  EchoServer server(2000);
  server.start();

  std::cout << "Starting ";
  while (server.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (server.isOK()) {
    std::cout << "\nRunning" << std::endl;

    std::string answer;
    std::future<std::string> future = std::async(getAnswer);
    
    do {
      std::chrono::milliseconds timeout(10);
      if (future.wait_for(timeout) == std::future_status::ready) {
        answer = future.get();
        if (answer != "q") future = std::async(getAnswer);
      }
      
      auto currentTime = Clock::now();
      if (std::chrono::duration_cast<std::chrono::minutes>(currentTime-restorePointTime).count() > 60) {
        restorePointTime = currentTime;
        server.savePaintLayer("paint.bmp");
      }
      
    } while (answer != "q");
    std::cout << "Shutting down server ..." << std::endl;
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
  
}

