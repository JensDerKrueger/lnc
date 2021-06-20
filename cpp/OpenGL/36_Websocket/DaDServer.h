#pragma once

#include <vector>
#include <string>
#include <thread>
#include <map>

#include <Server.h>
#include <bmp.h>

#include "Realm.h"

class DaDServerException : public std::exception {
  public:
  DaDServerException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};


class DaDServer : public Server<WebSocketConnection> {
public:
  DaDServer(uint16_t port, uint16_t canvasWidth, uint16_t canvasHeight,
             const std::vector<std::string>& layerImages);
  virtual ~DaDServer();
  void savePaintLayers();

protected:
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;
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
  std::vector<std::string> layerImages;
  
  std::map<uint32_t, uint32_t> realmMapping;

  virtual void printStats();
  void handlePaint(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  void handleClear(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  void handlePos(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  void activeRealm(uint32_t realmID, uint32_t id);

  void savePaintLayer(size_t layerIndex);
  void loadPaintLayer(size_t layerIndex);

  size_t initMessageHeaderSize() const;
  size_t initMessageLayerSize() const;  
};


