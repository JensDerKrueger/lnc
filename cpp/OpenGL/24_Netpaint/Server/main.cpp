#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <Server.h>
#include <Image.h>
#include <bmp.h>

#include "../PainterCommon.h"

class MyServer : public Server {
public:
  
  MyServer(short port, bool skipMousePosTransfer, bool recordInteraction) :
    Server(port),
    skipMousePosTransfer(skipMousePosTransfer),
    recordInteraction(recordInteraction)
  {
    try {
      image = BMP::load("artwork.bmp");
      std::cout << "Resuming session" << std::endl;
    } catch(const BMP::BMPException&) {
      for (size_t i = 0;i<image.data.size();i+=4) {
        image.data[i+0] = 0;
        image.data[i+1] = 0;
        image.data[i+2] = 128;
        image.data[i+3] = 255;
      }
      std::cout << "Starting new session" << std::endl;
    }
    
    if (recordInteraction) {
      BMP::save("recording.bmp", image);
      recordFile.open("recording.csv");
    }
  }
  
  ~MyServer() {
    BMP::save("artwork.bmp", image);
    
    if (recordInteraction) {
      recordFile.close();
    }
  }

  virtual void handleClientConnection(uint32_t id) override {
    InitPayload l(image, clientInfo);
    sendMessage(l.toString(), id);
  }
  
  virtual void handleClientDisconnection(uint32_t id) override {
    ciMutex.lock();
    for (size_t i = 0;i<clientInfo.size();++i) {
      if (clientInfo[i].id == id) {
        clientInfo.erase(clientInfo.begin()+i);
        ciMutex.unlock();
        LostUserPayload l;
        l.userID = id;
        sendMessage(l.toString(), id, true);
        
        if (recordInteraction) {
          recordFile << "drop;" << l.userID << "\n";
        }

        return;
      }
    }
    ciMutex.unlock();
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    PayloadType pt = identifyString(message);
    ciMutex.lock();
    switch (pt) {
      case PayloadType::MousePosPayload : {
        if (skipMousePosTransfer) break;
        
        MousePosPayload l(message);
        l.userID = id;
        const std::string message = l.toString();
        
        for (const auto& c : clientInfo) {
          if (c.id == id || !c.fastCursorUpdates) continue;
          sendMessage(message, c.id);
        }
        
        break;
      }
      case PayloadType::ConnectPayload  : {
        ConnectPayload r(message);
        r.userID = id;
        clientInfo.push_back({{id, r.name, r.color, {0,0}}, r.fastCursorUpdates});
        NewUserPayload l(r.name, r.color);
        l.userID = id;
        sendMessage(l.toString(), id, true);

        if (recordInteraction) {
          recordFile << "new;" << l.userID << ";" << l.name << ";" << l.color << "\n";
        }

        break;
      }
      case PayloadType::CanvasUpdatePayload  : {
        CanvasUpdatePayload l(message);
        l.userID = id;

        if (l.pos.x() < 0 || uint32_t(l.pos.x()) >= image.width) break;
        if (l.pos.y() < 0 || uint32_t(l.pos.y()) >= image.height) break;
        
        if (recordInteraction) {
          recordFile << "paint;" << l.userID << ";" << l.pos << ";" << l.color << "\n";
        }

        
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
  std::vector<ClientInfoServerSide> clientInfo;
  std::mutex ciMutex;
  Image image;
  bool skipMousePosTransfer;
  bool recordInteraction;
  std::ofstream recordFile;
};


int main(int argc, char ** argv) {
  bool skipMousePosTransfer{false};
  bool recordInteraction{false};
  for (int i = 1;i<argc;++i) {
    switch (argv[i][0]) {
      case 's' :
        std::cout << "Skipping live cursor movement" << std::endl;
        skipMousePosTransfer = true;
        break;
      case 'r' :
        std::cout << "Recording enabled" << std::endl;
        recordInteraction = true;
        break;
    }
  }
  
  MyServer s{serverPort, skipMousePosTransfer, recordInteraction};
  s.start();
  std::cout << "starting ...";
  while (s.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << std::endl;
  if (s.isOK()) {
    std::cout << "running ..." << std::endl;
    std::string user{""};
    do {
      std::cin >> user;
    } while (user != "q");

    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
