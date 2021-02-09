#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "Server.h"
#include "Image.h"

#include "../PainterCommon.h"

class MyServer : public Server {
public:
  
  MyServer(short port) : Server(port, "asdn932lwnmflj23") {
    for (size_t i = 0;i<image.data.size();i+=4) {
      image.data[i+0] = 0;
      image.data[i+1] = 0;
      image.data[i+2] = 128;
      image.data[i+3] = 255;
    }
  }

  virtual void handleClientConnection(size_t id) override {
    InitPayload l(image, mouseInfo);
    sendMessage(l.toString(), id);
  }
  
  virtual void handleClientDisconnection(size_t id) override {
    ciMutex.lock();
    for (size_t i = 0;i<mouseInfo.size();++i) {
      if (mouseInfo[i].id == id) {
        mouseInfo.erase(mouseInfo.begin()+i);
        ciMutex.unlock();
        LostUserPayload l;
        l.userID = id;
        sendMessage(l.toString(), id, true);
        break;
      }
    }
    ciMutex.unlock();
  }
  
  std::optional<MouseInfo> getMouseInfo(size_t id) {
    for (auto& c : mouseInfo) {
      if (c.id == id) {
        return c;
      }
    }
    return {};
  }

  virtual void handleClientMessage(size_t id, const std::string& message) override {
    PayloadType pt = identifyString(message);
    ciMutex.lock();
    switch (pt) {
      case PayloadType::MousePosPayload : {
        MousePosPayload l(message);
        l.userID = id;
        sendMessage(l.toString(), id, true);
        break;
      }
      case PayloadType::NewUserPayload  : {
        NewUserPayload l(message);
        l.userID = id;
        mouseInfo.push_back({id, l.name, l.color, {0,0}});
        sendMessage(l.toString(), id, true);
        break;
      }
      case PayloadType::CanvasUpdatePayload  : {
        CanvasUpdatePayload l(message);
        l.userID = id;

        image.setNormalizedValue(l.pos.x(),l.pos.y(),0,l.color.x());
        image.setNormalizedValue(l.pos.x(),l.pos.y(),1,l.color.y());
        image.setNormalizedValue(l.pos.x(),l.pos.y(),2,l.color.z());
        image.setNormalizedValue(l.pos.x(),l.pos.y(),3,l.color.w());

        sendMessage(l.toString(), id, true);
        break;
      }
      default:
        break;
    };
    ciMutex.unlock();
  }
  
private:
  std::vector<MouseInfo> mouseInfo;
  std::mutex ciMutex;
  Image image{imageWidth,imageHeight};
};


int main(int argc, char ** argv) {
  MyServer s{11001};
  std::cout << "starting ...";
  while (s.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << std::endl;
  
  if (s.isOK()) {
    std::cout << "running ..." << std::endl;
    std::string test;
    std::cin >> test;
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
