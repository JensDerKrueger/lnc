#pragma once

#include <vector>
#include <string>
#include <thread>

#include <Server.h>
#include <bmp.h>

class EchoServerException : public std::exception {
  public:
  EchoServerException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};


class EchoServer : public Server<WebSocketConnection> {
public:
  EchoServer(uint16_t port, uint16_t canvasWidth, uint16_t canvasHeight, uint8_t layerCount);
  virtual ~EchoServer();
  
  void savePaintLayer(const std::string& filename);
  void loadPaintLayer(const std::string& filename);

protected:
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;
  std::vector<uint8_t> extracted(const std::vector<uint8_t> &message);
  
  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override;
  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override;
  virtual void handleClientDisconnection(uint32_t id) override;
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override;
  virtual void handleError(const std::string& message) override;

private:
  std::vector<uint8_t> imageMessage;
  std::mutex imageLock;
  uint16_t canvasWidth;
  uint16_t canvasHeight;
  uint8_t  layerCount;

  virtual void printStats();
  static uint16_t to16Bit(const std::vector<uint8_t>& message, size_t index);
  void paint(const std::vector<uint8_t>& message);
  std::vector<uint8_t> fixPaintMessage(const std::vector<uint8_t> &message);
};


