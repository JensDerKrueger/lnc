#include <fstream>
#include <sstream>

#include <bmp.h>

#include "Realm.h"

Realm::Realm(const std::string& filename) {
  load(filename);
}

Realm::Realm(uint32_t id, const std::string& name, const std::vector<Image>& layerImages) :
  id(id),
  name(name),
  layerImages(layerImages)
{
}
  
void Realm::save(const std::string& filename) const {
  std::ofstream file(filename);
  file << id << std::endl;
  file << name << std::endl;
  file << layerImages.size() << std::endl;
  file.close();

  for (size_t i = 0;i<layerImages.size();++i) {
    std::stringstream ss;
    ss << filename << "_" << i << ".bmp";
    BMP::save(ss.str(),layerImages[i]);
  }
}

void Realm::load(const std::string& filename) {
  std::ifstream file(filename);
  file >> id;
  file >> name;
  uint32_t layerNameCount;
  file >> layerNameCount;
  file.close();

  layerImages.clear();
  for (size_t i = 0;i<layerNameCount;++i) {
    std::stringstream ss;
    ss << filename << "_" << i << ".bmp";
    layerImages.push_back(BMP::load(ss.str()));
  }
}
