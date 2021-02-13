#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <Client.h>
#include <GLApp.h>
#include <Rand.h>
#include <FontRenderer.h>

#include "../PainterCommon.h"

class MyClient : public Client {
public:
  MyClient(const std::string& address, short port, const std::string& name);

  void moveMouse(uint32_t userID, const Vec2& pos);
  void addMouse(uint32_t userID, const std::string& name, const Vec4& color);
  void removeMouse(uint32_t userID);
  void initDataFromServer(const Image& serverImage, const std::vector<ClientInfo>& mi);
  virtual void handleNewConnection() override;
  virtual void handleServerMessage(const std::string& message) override;
  void setMousePos(const Vec2& normPos);
  void paint(const Vec2i& pos);
  const std::vector<ClientInfoClientSide>& getClientInfos() const ;
  const Image& getImage();
  void lockData();
  void unlockData();
  Vec4 getColor() const;
  bool isValid() const;
  void setColor(const Vec4& color);
    
  static FontRenderer fr;

private:
  bool rendererLock{false};
  std::mutex miMutex;
  std::vector<ClientInfoClientSide> clientInfo;
  std::string name;
  Vec4 color;
  bool initComplete{false};
  Image image{imageWidth,imageHeight};

  void paint(const Vec4& color, const Vec2i& pos);
  
};
