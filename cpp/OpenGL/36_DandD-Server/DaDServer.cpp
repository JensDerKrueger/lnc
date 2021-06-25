#include "DaDServer.h"

DaDServer::DaDServer(uint16_t port) :
Server(port)
{
  const std::scoped_lock<std::mutex> lock(realmMutex);

  std::vector<std::string> files;
  for (auto& p: std::filesystem::directory_iterator(".")) {
    try {
      if (p.path().extension() != ".realm") continue;
      realms.push_back(std::make_shared<Realm>(p.path().string()));
      existingRealms[realms.back()->getID()] = realms.size()-1;
      std::cout << "Loaded file " << p.path().string() << " containing realm " << realms.back()->getName()
                << " with id " << realms.back()->getID() << " and " << realms.back()->getLayerCount() << " layer." << std::endl;
    } catch (...) {
    }
  }

  if (existingRealms.find(0) == existingRealms.end()) {
    std::vector<Image> layerImages{Image{800,600,4},Image{800,600,4},Image{800,600,4}};
    realms.push_back(std::make_shared<Realm>(0, "Lobby", layerImages, "lobby.realm"));
    existingRealms[realms.back()->getID()] = realms.size()-1;
  }
}
  
DaDServer::~DaDServer() {
  saveRealms();
}
 
void DaDServer::handleClientMessage(uint32_t id, const std::string& message) {
  std::cerr << "Error: Client (id:" << id << ") send a string message" << std::endl;
}

void DaDServer::handleClientConnection(uint32_t id, const std::string& address, uint16_t port) {
  const std::scoped_lock<std::mutex> lock(realmMutex);

  std::cout << "Player " << id << " joinded from " << address << std::endl;
  printStats();
  sendMessage(realms[existingRealms[userRealms[id]]]->serialize(),id);
}

void DaDServer::handleClientDisconnection(uint32_t id) {
  std::cout << "Player " << id << " left the game from realm " << userRealms[id] << std::endl;
  removeCursor(id, userRealms[id]);
  userRealms.erase(id);
  printStats();
}

void DaDServer::handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) {
  std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
}

void DaDServer::handleError(const std::string& message) {
  std::cerr << "Error: " << message << std::endl;
}

void DaDServer::printStats() {
  std::cout << "A total of " << getValidIDs().size() << " players are currently connected." << std::endl;
}
  
void DaDServer::removeCursor(uint32_t id, uint32_t realmID) {
  realms[existingRealms[userRealms[id]]]->deleteCursor(id);  
  
  BinaryEncoder enc;
  enc.add(uint8_t(3));
  enc.add(std::numeric_limits<uint32_t>::max());
  enc.add(id);
  enc.add(0);
  enc.add(0);
  enc.add("");
  sendMessage(enc.getEncodedMessage());
}

void DaDServer::handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) {
  const std::scoped_lock<std::mutex> lock(realmMutex);

  try {
    BinaryDecoder dec(message);
    uint8_t messageID = dec.nextUint8();
    uint32_t realmID  = dec.nextUint32();
    if (!activeRealm(realmID, id)) return;
    switch (messageID) {
      case 1 :
        handlePaint(dec, realmID, id);
        sendMessage(message);
        break;
      case 2 :
        handleClear(dec, realmID, id);
        sendMessage(message);
        break;
      case 3 : {
        const std::vector<uint8_t> newMessage = handleCursorPos(dec, realmID, id);
        sendMessage(newMessage);
        break;
      }
      case 4 : {
        const std::vector<uint8_t> newMessage = handleSwitchRealm(realmID, id);
        sendMessage(newMessage);
        break;
      }
      default:
        throw DaDServerException("Invalid message ID.");
    }
  } catch (const DaDServerException& e) {
    std::cout << "Client " << id << " is sending invalid data '"
              << e.what() << "'"<< std::endl;
  }
}

void DaDServer::handleClear(BinaryDecoder& dec, uint32_t realmID, uint32_t id) {
  std::shared_ptr<Realm> realm = realms[existingRealms[userRealms[id]]];

  try {
    Vec4t<uint8_t> color;
    color.r = dec.nextUint8();
    color.g = dec.nextUint8();
    color.b = dec.nextUint8();
    color.a = dec.nextUint8();
    uint8_t target = dec.nextUint8();
    if (target >= realm->getLayerCount())
      throw DaDServerException("Invalid target");

    realm->clear(color,target);
  } catch (const MessageException& e) {
    throw DaDServerException("Message too short");
  }
}

void DaDServer::handlePaint(BinaryDecoder& dec, uint32_t realmID, uint32_t id) {
  std::shared_ptr<Realm> realm = realms[existingRealms[userRealms[id]]];

  try {
    Vec2t<uint16_t> pos;
    pos.x = dec.nextUint16();
    if (pos.x > realm->getLayerDims().x-1) throw DaDServerException("Invalid posX");

    pos.y = dec.nextUint16();
    if (pos.y > realm->getLayerDims().y-1) throw DaDServerException("Invalid posY");

    Vec4t<uint8_t> color;
    color.r = dec.nextUint8();
    color.g = dec.nextUint8();
    color.b = dec.nextUint8();
    color.a = dec.nextUint8();
    
    uint16_t brushSize = dec.nextUint16();
    if (brushSize > 200) throw DaDServerException("Invalid size");

    uint8_t target = dec.nextUint8();
    if (target >= realm->getLayerCount()) throw DaDServerException("Invalid target");
    
    realm->paint(pos, color, brushSize, target);
    
  } catch (const MessageException& e) {
    throw DaDServerException("Message too short");
  }
}

const std::vector<uint8_t> DaDServer::handleSwitchRealm(uint32_t realmID, uint32_t id) {
  BinaryEncoder enc;
  enc.add(uint8_t(4));
  enc.add(realmID);
  enc.add(id);
  return enc.getEncodedMessage();
}

const std::vector<uint8_t> DaDServer::handleCursorPos(BinaryDecoder& dec, uint32_t realmID, uint32_t id) {
  std::shared_ptr<Realm> realm = realms[existingRealms[userRealms[id]]];

  try {
    dec.nextUint32(); // dummy id from client

    Vec2t<uint16_t> pos;
    pos.x = dec.nextUint16();
    if (pos.x > realm->getLayerDims().x-1) throw DaDServerException("Invalid posX");

    pos.y = dec.nextUint16();
    if (pos.y > realm->getLayerDims().y-1) throw DaDServerException("Invalid posY");

    std::string name = dec.nextString();
    if (name.empty()) throw DaDServerException("Invalid name");
    
    for (const std::shared_ptr<Realm>& r : realms) {
      if (realm != r)
        r->deleteCursor(id);
    }
    realm->setCursor(pos, id, name);
    
    BinaryEncoder enc;
    enc.add(uint8_t(3));
    enc.add(realmID);
    enc.add(id);
    enc.add(pos.x);
    enc.add(pos.y);
    enc.add(name);
    return enc.getEncodedMessage();
    
  } catch (const MessageException& e) {
    throw DaDServerException("Message too short");
  }
}

bool DaDServer::activeRealm(uint32_t realmID, uint32_t id) {
  if (existingRealms.find(realmID) == existingRealms.end()) {
    std::cerr << "Player " << id << " selected the invalid realm " << realmID << std::endl;
    sendMessage(realms[existingRealms[userRealms[id]]]->serialize(),id);
    return false;
  }
  
  if (userRealms[id] != realmID) {
    std::cout << "Player " << id << " switching to realm " << realmID << std::endl;
    userRealms[id] = realmID;
    sendMessage(realms[existingRealms[realmID]]->serialize(),id);
    std::cout << "Player " << id << " switched to realm " << realmID << std::endl;
  }
  
  return true;
}

void DaDServer::saveRealms() const {
  const std::scoped_lock<std::mutex> lock(realmMutex);

  for (const std::shared_ptr<Realm>& r : realms) {
    r->save();
  }
}
