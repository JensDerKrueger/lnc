#pragma once

#include <string>
#include <vector>

#include <Image.h>

class Realm {
public:
  Realm(const std::string& filename);
  Realm(uint32_t id, const std::string& name, const std::vector<Image>& layerImages);
  
  void save(const std::string& filename) const;
  
  uint32_t getID() const {return id;}
  std::string getName() const {return name;}
  const std::vector<Image>& getLayerImages() const {return layerImages;}

private:
  uint32_t id;
  std::string name;
  std::vector<Image> layerImages;
  
  void load(const std::string& filename);
};
