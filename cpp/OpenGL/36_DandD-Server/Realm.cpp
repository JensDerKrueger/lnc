#include <fstream>
#include <sstream>

#include <bmp.h>
#include <NetCommon.h>

#include "Realm.h"

Realm::Realm(const std::string& filename) :
filename(filename)
{
  load();
}

Realm::Realm(uint32_t id, const std::string& name, const std::vector<Image>& layerImages, const std::string& filename) :
  id(id),
  name(name),
  layerImages(layerImages),
  filename(filename)
{
}
  
void Realm::save() const {
  const std::scoped_lock<std::mutex> lock(imageLock);

  std::ofstream file(filename);
  file << id << std::endl;
  file << name << std::endl;
  file << layerImages.size() << std::endl;
  file.close();

  for (size_t i = 0;i<layerImages.size();++i) {
    std::stringstream ss;
    ss << filename << "_" << i << ".bmp";
    BMP::save(ss.str(),layerImages[i].flipHorizontal());
  }
}

void Realm::load() {
  const std::scoped_lock<std::mutex> lock(imageLock);

  std::string line;
  std::ifstream file(filename);
  std::getline(file,line);
  id = uint32_t(std::stoi(line));
  std::getline(file,name);
  std::getline(file,line);
  size_t layerNameCount = size_t(std::stoi(line));
  file.close();

  layerImages.clear();
  for (size_t i = 0;i<layerNameCount;++i) {
    std::stringstream ss;
    ss << filename << "_" << i << ".bmp";
    Image image = BMP::load(ss.str());
    image = image.flipHorizontal();
    if (3 == image.componentCount) image.generateAlpha();
    layerImages.push_back(image);

    if (image.width != layerImages[0].width ||
        image.height != layerImages[0].height ||
        4 != image.componentCount) {
      throw BMP::BMPException("Invalid Image dimensions");
    }
  }
}

std::vector<uint8_t> Realm::serialize() const {
  const std::scoped_lock<std::mutex> lock(imageLock);
  std::vector<uint8_t> data;
  if (!layerImages.empty()) {
    BinaryEncoder enc;
    enc.add(uint8_t(0));
    enc.add(uint16_t(layerImages[0].width));
    enc.add(uint16_t(layerImages[0].height));
    enc.add(uint8_t(layerImages.size()));
    enc.add(name);
    enc.add(id);
    enc.add(uint32_t(playerPos.size()));
    for (const auto& p : playerPos) {
      enc.add(p.first);
      enc.add(p.second.pos.x);
      enc.add(p.second.pos.y);
      enc.add(p.second.name);
    }
    data = enc.getEncodedMessage();
    for (const Image& layer : layerImages) {
      data.insert( data.end(), layer.data.begin(), layer.data.end() );
    }
  }
  return data;
}

void Realm::paint(const Vec2t<uint16_t>& pos, const Vec4t<uint8_t>& color, uint16_t brushSize, uint16_t target) {
  const std::scoped_lock<std::mutex> lock(imageLock);
  Image& image = layerImages[target];
  for (uint32_t y = 0;y<brushSize;++y) {
    for (uint32_t x = 0;x<brushSize;++x) {
      const uint32_t posX = pos.x+x;
      const uint32_t posY = pos.y+y;
      if (posX < image.width && posY < image.height) {
        const size_t serialPos = size_t(posX + posY * image.width)*4;
        image.data[serialPos+0] = color.r;
        image.data[serialPos+1] = color.g;
        image.data[serialPos+2] = color.b;
        image.data[serialPos+3] = color.a;
      }
    }
  }
}

void Realm::clear(const Vec4t<uint8_t>& color, uint16_t target) {
  const std::scoped_lock<std::mutex> lock(imageLock);
  Image& image = layerImages[target];
  for (uint32_t i = 0;i<image.data.size();i+=4) {
    image.data[i+0] = color.r;
    image.data[i+1] = color.g;
    image.data[i+2] = color.b;
    image.data[i+3] = color.a;
  }
}

void Realm::deleteCursor(uint32_t playerID) {
  playerPos.erase(playerID);
}

void Realm::setCursor(const Vec2t<uint16_t>& pos, uint32_t playerID, const std::string& name) {
  playerPos[playerID] = PlayerPos{pos,name};
}

size_t Realm::getLayerCount() const {
  return layerImages.size();  
}
  
Vec2t<uint16_t> Realm::getLayerDims() const {
  if (!layerImages.empty()) {
    return {uint16_t(layerImages[0].width), uint16_t(layerImages[0].height)};
  } else {
    return {0,0};
  }
}

