#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <Image.h>
#include <Vec2.h>
#include <Vec4.h>

struct PlayerPos {
  Vec2t<uint16_t> pos;
  std::string name;
};


class Realm {
public:
  Realm(const std::string& filename);
  Realm(uint32_t id, const std::string& name, const std::vector<Image>& layerImages, const std::string& filename);
  
  void save() const;
  
  void paint(const Vec2t<uint16_t>& pos, const Vec4t<uint8_t>& color, uint16_t brushSize, uint16_t target);
  void clear(const Vec4t<uint8_t>& color, uint16_t target);
  void deleteCursor(uint32_t playerID);
  void setCursor(const Vec2t<uint16_t>& pos, uint32_t playerID, const std::string& name);
  size_t getLayerCount() const;
  Vec2t<uint16_t> getLayerDims() const;
    
  uint32_t getID() const {return id;}
  std::string getName() const {return name;}
  const std::vector<Image>& getLayerImages() const {return layerImages;}
  
  std::vector<uint8_t> serialize() const;

private:
  mutable std::mutex imageLock;
  
  uint32_t id;
  std::string name;
  std::vector<Image> layerImages;
  std::string filename;
  std::map<uint32_t,PlayerPos> playerPos;

  void load();
};
