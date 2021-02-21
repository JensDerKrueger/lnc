#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <exception>
#include <Server.h>
#include <Image.h>
#include <bmp.h>

#include "../PainterCommon.h"

typedef std::chrono::high_resolution_clock Clock;

static bool fexists(const std::string& filename) {
  std::ifstream ifile(filename);
  return (bool)ifile;
}

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
      if (!fexists("recording.bmp")) BMP::save("recording.bmp", image);
      recordFile.open("recording.csv",std::ios_base::app);
      recordFile << "start\n";
    }
  }
  
  virtual ~MyServer() {
    saveArt();
    if (recordInteraction) {
      recordFile.close();
    }
  }
  
  void saveArt() {
    BMP::save("artwork.bmp", image);
    if (recordInteraction) recordFile.flush();
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
          recordedEvents++;
        }

        return;
      }
    }
    ciMutex.unlock();
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    
    auto currentTime = Clock::now();
    if ( std::chrono::duration_cast<std::chrono::seconds>(currentTime-lastime).count() > 60 ) {
      saveArt();
      lastime = currentTime;
    }
    
    ciMutex.lock();
    PayloadType pt = identifyString(message);
    switch (pt) {
      case PayloadType::MousePosPayload : {
        if (skipMousePosTransfer) break;
        
        MousePosPayload l(message);
        l.userID = id;
        const std::string targetMessage = l.toString();
        
        for (auto& c : clientInfo) {
          if (c.id == id) c.pos = l.mousePos;
          if (c.id == id || !c.fastCursorUpdates) continue;
          sendMessage(targetMessage, c.id);
        }
        
        break;
      }
      case PayloadType::ConnectPayload  : {
        ConnectPayload r(message);
        r.userID = id;
        ClientInfoServerSide ci{id, r.name, r.color, {0,0}, r.fastCursorUpdates};
        clientInfo.push_back(ci);
        
        NewUserPayload l(ci.name, ci.color);
        l.userID = id;
        sendMessage(l.toString(), id, true);

        if (recordInteraction) {
          recordFile << "new;" << l.userID << ";" << l.name << ";" << l.color << "\n";
          recordedEvents++;
        }

        break;
      }
      case PayloadType::CanvasUpdatePayload  : {
        CanvasUpdatePayload l(message);
        l.userID = id;

        for (auto& c : clientInfo) {
          if (c.id == id) c.pos = Vec2(float(l.pos.x()) / image.width, float(l.pos.x()) / image.height);
        }


        if (l.pos.x() < 0 || uint32_t(l.pos.x()) >= image.width) break;
        if (l.pos.y() < 0 || uint32_t(l.pos.y()) >= image.height) break;
        
        if (recordInteraction) {
          recordFile << "paint;" << l.userID << ";" << l.pos << ";" << l.color << "\n";
          recordedEvents++;
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
  
  std::vector<ClientInfoServerSide> getClientInfo() {
    std::vector<ClientInfoServerSide> result;
    ciMutex.lock();
    result = clientInfo;
    ciMutex.unlock();
    return result;
  }
  
  void setSkipMousePosTransfer(bool newSkipMousePosTransfer) {
    skipMousePosTransfer = newSkipMousePosTransfer;
  }

  bool getSkipMousePosTransfer() const {
    return skipMousePosTransfer;
  }

  
  size_t getRecordedEvents() const {
    return recordedEvents;
  }
  
  int64_t getSecondsSinceLastBackup() const {
    auto currentTime = Clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime-lastime).count();
  }
  
  int64_t uptime() const {
    auto currentTime = Clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime-startTime).count();
  }

  
private:
  std::vector<ClientInfoServerSide> clientInfo;
  std::mutex ciMutex;
  Image image;
  bool skipMousePosTransfer;
  bool recordInteraction;
  std::ofstream recordFile;
  size_t recordedEvents{0};
  
  std::chrono::time_point<Clock> lastime = Clock::now();
  std::chrono::time_point<Clock> startTime = Clock::now();
};

static void globalExceptionHandler () {
  std::cerr << "terminate called after throwing an instance of ";
  try
  {
      std::rethrow_exception(std::current_exception());
  } catch (const std::exception &ex) {
      std::cerr << typeid(ex).name() << std::endl;
      std::cerr << "  what(): " << ex.what() << std::endl;
  } catch (...) {
      std::cerr << typeid(std::current_exception()).name() << std::endl;
      std::cerr << " ...something, not an exception, dunno what." << std::endl;
  }
#ifdef _WIN32
  std::cerr << "errno: " << errno << std::endl;
#else
  std::cerr << "errno: " << errno << ": " << std::strerror(errno) << std::endl;
#endif // _WIN32
  std::abort();
}

int main(int argc, char ** argv) {
  std::set_terminate (globalExceptionHandler);
  
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
      std::cout << "(? for help) >";
      std::cin >> user;
      
      switch (user[0]) {
        case '?' :
          std::cout << "? : help\n";
          std::cout << "l : list active users\n";
          std::cout << "q : quit server\n";
          std::cout << "m : toggle mousePos transfer\n";
          std::cout << "s : statisics\n";
          break;
        case 'l' : {
          std::vector<ClientInfoServerSide> ci = s.getClientInfo();
          for (const ClientInfoServerSide& client : ci) {
            std::cout << "ID:" << client.id << " Name:" << client.name << " Color:" << client.color << " Pos:" << client.pos << " FastUpdates: " << client.fastCursorUpdates << "\n";
          }
          break;
        }
        case 'm' : {
          s.setSkipMousePosTransfer(!s.getSkipMousePosTransfer());
          std::cout << "setSkipMousePosTransfer is now: "<< s.getSkipMousePosTransfer() << "\n";
          break;
        }
        case 's' : {
          std::cout << "Server Uptime: "<< s.uptime() << " sec.\n";
          std::cout << "Time since last backup: "<< s.getSecondsSinceLastBackup() << " sec.\n";
          std::cout << "Recorded Events: "<< s.getRecordedEvents() << "\n";
          break;
        }
      }
      std::cout << std::endl;
      
    } while (user != "q");

    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
