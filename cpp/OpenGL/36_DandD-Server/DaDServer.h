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
  DaDServer(uint16_t port);
  virtual ~DaDServer();
  void saveRealms() const;

protected:
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;
  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override;
  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override;
  virtual void handleClientDisconnection(uint32_t id) override;
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override;
  virtual void handleError(const std::string& message) override;

private:
  std::vector<std::shared_ptr<Realm>> realms;
  std::map<uint32_t, uint32_t> userRealms;
  std::map<uint32_t, size_t> existingRealms;
  mutable std::mutex realmMutex;

  virtual void printStats();
  void handlePaint(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  void handleClear(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  const std::vector<uint8_t> handleCursorPos(BinaryDecoder& dec, uint32_t realmID, uint32_t id);
  const std::vector<uint8_t> handleSwitchRealm(uint32_t realmID, uint32_t id);
  void removeCursor(uint32_t id, uint32_t realmID);
    
  bool activeRealm(uint32_t realmID, uint32_t id);

  void savePaintLayer(size_t layerIndex);
  void loadPaintLayer(size_t layerIndex);
};


