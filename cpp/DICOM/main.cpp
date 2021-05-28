#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "DICOMDirParser.h"

static void writeDat(const std::string& filename, const DCMVec3ui& size, const DCMVec3& aspect, const uint32_t allocated ) {
  std::ofstream f(filename+".dat");
  f << "ObjectFileName: " << filename << ".raw" << std::endl;
  f << "TaggedFileName: ---"  << std::endl;
  f << "Resolution:     " << size.x << " " << size.y << " " << size.z << std::endl;  
  f << "SliceThickness: " << aspect.x << " " << aspect.y << " " << aspect.z << std::endl;
  f << "Format:         " << (allocated == 8 ? "UCHAR" : "USHORT") << std::endl;
  f << "ObjectType:     TEXTURE_VOLUME_OBJECT" << std::endl;
  f << "ObjectModel:    RGBA" << std::endl;
  f << "GridType:       EQUIDISTANT" << std::endl;
  f << "Endianess:      LITTLE" << std::endl;
  f.close();
}

static void writeRaw(const std::string& filename, const std::vector<uint8_t>& data) {
  std::ofstream f(filename+".raw", std::ios::binary );
  f.write( (char*)data.data(), uint32_t(data.size()) );
  f.close();
}

int main(int argc, char ** argv) {
  std::string directory{argv[1]};
  DICOMDirParser parser{directory};
  
  std::cout << "Found " << parser.getVolumeCount() << " volume(s)." << std::endl;
  
  for (size_t i = 0;i<parser.getVolumeCount();++i) {
    std::cout << "  " << i+1 << std::endl;
    std::cout << "  Size:       " << parser.getVolmeSize(i).x << " x " << parser.getVolmeSize(i).y << " x " << parser.getVolmeSize(i).z << std::endl;
    std::cout << "  Aspect:     " <<  parser.getVolmeAspect(i).x << " x " << parser.getVolmeAspect(i).y << " x " << parser.getVolmeAspect(i).z << std::endl;
    std::cout << "  Components: " <<  parser.getComponentCount(i) << " allocating "  << parser.getAllocated(i) << " bits" << std::endl;
    
    std::stringstream ss;
    ss << "volume" << (i+1);
    const std::string filename = ss.str();
    writeDat(filename, parser.getVolmeSize(i), parser.getVolmeAspect(i), parser.getAllocated(i));
    writeRaw(filename, parser.getRawData(i));
  }
  
  return EXIT_SUCCESS;
}
