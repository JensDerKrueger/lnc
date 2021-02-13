#include "MyClient.h"


#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer MyClient::fr{fontImage, fontPos};
#else
  FontRenderer MyClient::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

MyClient::MyClient(const std::string& address, short port, const std::string& name) :
  Client{address, port, "asdn932lwnmflj23", 5000},
  name{name},
  color{Vec3::hsvToRgb({360*Rand::rand01(),0.5f,1.0f}), 1.0f}
{
  
  
  
  for (uint32_t y = 0;y<image.height;++y) {
    for (uint32_t x = 0;x<image.width;++x) {
      const Vec3 rgb{0,0,0};
      image.setNormalizedValue(x,y,0,rgb.x());
      image.setNormalizedValue(x,y,1,rgb.y());
      image.setNormalizedValue(x,y,2,rgb.z());
      image.setValue(x,y,3,255);
    }
  }
}

void MyClient::moveMouse(uint32_t userID, const Vec2& pos) {
  for (size_t i = 0;i<mouseInfos.size();++i) {
    if (mouseInfos[i].id == userID) {
      mouseInfos[i].pos = pos;
      break;
    }
  }
}

void MyClient::addMouse(uint32_t userID, const std::string& name, const Vec4& color) {
  bool found{false};
  for (size_t i = 0;i<mouseInfos.size();++i) {
    if (mouseInfos[i].id == userID) {
      mouseInfos[i].name = name;
      mouseInfos[i].image = fr.render(name);
      mouseInfos[i].color = color;
      found = true;
      break;
    }
  }
  if (!found) {
    mouseInfos.push_back({{userID, name, color, {0,0}}, fr.render(name)});
  }
}

void MyClient::removeMouse(uint32_t userID) {
  for (size_t i = 0;i<mouseInfos.size();++i) {
    if (mouseInfos[i].id == userID) {
      mouseInfos.erase(mouseInfos.begin()+i);
      break;
    }
  }
}

void MyClient::initDataFromServer(const Image& serverImage,
                        const std::vector<MouseInfo>& mi) {
  image      = serverImage;
  
  mouseInfos.clear();
  for (const MouseInfo& m : mi) {
    mouseInfos.push_back({{m.id, m.name, m.color, m.pos}, fr.render(m.name) });
  }

  initComplete = true;
}
  
void MyClient::handleNewConnection() {
  NewUserPayload l(name, color);
  sendMessage(l.toString());
}

void MyClient::handleServerMessage(const std::string& message) {
  PayloadType pt = identifyString(message);
     
  miMutex.lock();
  switch (pt) {
    case PayloadType::MousePosPayload : {
      MousePosPayload l(message);
      moveMouse(l.userID, l.mousePos);
      break;
    }
    case PayloadType::NewUserPayload  : {
      NewUserPayload l(message);
      addMouse(l.userID, l.name, l.color);
      break;
    }
    case PayloadType::LostUserPayload : {
      LostUserPayload l(message);
      removeMouse(l.userID);
      break;
    }
    case PayloadType::InitPayload : {
      InitPayload l(message);
      initDataFromServer(l.image, l.mouseInfos);
      break;
    }
    case PayloadType::CanvasUpdatePayload : {
      CanvasUpdatePayload l(message);
      paint(l.color, l.pos);
      break;
    }
    default:
      std::cout << "unknown message " << int(pt) << " received" << std::endl;
      break;
  };
  miMutex.unlock();
}

void MyClient::setMousePos(const Vec2& normPos) {
  if (isConnecting() || !initComplete) {
    initComplete = false;
    return;
  }
  
  MousePosPayload m(normPos);
  sendMessage(m.toString());
}


void MyClient::paint(const Vec2i& pos) {
  if (isConnecting() || !initComplete) {
    initComplete = false;
    return;
  }

  paint(color, pos);
  CanvasUpdatePayload m{color, pos};
  sendMessage(m.toString());
}

const std::vector<ClientMouseInfo>& MyClient::getOtherMouseInfos() const {
  if (!rendererLock) {
    throw std::runtime_error("getImage called without lockData");
  }
  return mouseInfos;
}
  
const Image& MyClient::getImage() {
  if (!rendererLock) {
    throw std::runtime_error("getImage called without lockData");
  }

  if (isConnecting() || !initComplete) {
    initComplete = false;
  }
  return image;
}

void MyClient::lockData() {
  if (rendererLock) {
    throw std::runtime_error("lockData called repeatedly without unlockData");
  }
  miMutex.lock();
  rendererLock = true;
}

void MyClient::unlockData() {
  if (!rendererLock) {
    throw std::runtime_error("unlockData called without lockData");
  }
  miMutex.unlock();
  rendererLock = false;
}

Vec4 MyClient::getColor() const {
  return color;
}

bool MyClient::isValid() const {
  return initComplete;
}

void MyClient::setColor(const Vec4& color) {
  this->color = color;
}

void MyClient::paint(const Vec4& color, const Vec2i& pos) {
  if (pos.x() < 0 || uint32_t(pos.x()) >= image.width) return;
  if (pos.y() < 0 || uint32_t(pos.y()) >= image.height) return;
  
  image.setNormalizedValue(pos.x(),pos.y(),0,color.x());
  image.setNormalizedValue(pos.x(),pos.y(),1,color.y());
  image.setNormalizedValue(pos.x(),pos.y(),2,color.z());
  image.setNormalizedValue(pos.x(),pos.y(),3,color.w());
}
