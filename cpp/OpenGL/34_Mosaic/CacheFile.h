#pragma once

#include <string>
#include <exception>
#include <fstream>
#include <vector>
#include <map>

#include <Vec3.h>
#include <Vec2.h>
#include <Image.h>
#include <MD5.h>

std::vector<Vec3t<double>> computeFeatureTensorForImage(const Image& image,
                                                        const Vec2ui& globalStart,
                                                        const Vec2ui& compressionFactor,
                                                        const Vec2ui& blockSize);

class CacheFileException : public std::exception {
  public:
  CacheFileException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

struct CacheFileEntry {
  CacheFileEntry(std::ifstream& file, uint32_t tensorLength);
  
  CacheFileEntry(const Image& image,
                 std::ofstream& file,
                 const Vec2ui& smallImageResolution,
                 const Vec2ui& largeImageBlockSize,
                 uint64_t offset);
  
  void save(std::ofstream& file) const;
      
  std::vector<Vec3t<double>> featureTensor;
  uint64_t offset;
  uint64_t size;
};

class CacheFile {
public:
  CacheFile(const std::string& filename);
  
  Vec2ui getSmallImageResolution() const {
    return smallImageResolution;
  }
  Vec2ui getLargeImageBlockSize() const {
    return largeImageBlockSize;
  }
  
  size_t getImageCount() const;
  std::vector<Vec3t<double>> getFeatureTensor(size_t i) const;
  Image getImage(size_t i);
  
private:
  std::ifstream file;
  Vec2ui smallImageResolution;
  Vec2ui largeImageBlockSize;
  std::vector<CacheFileEntry> cacheFileEntries;
  
  size_t extracted();
  
  void load(const std::string& filename);
  size_t loadHeader();
};

class CacheFileGenerator {
public:
  CacheFileGenerator(const Vec2ui& smallImageResolution,
                     const Vec2ui& largeImageBlockSize,
                     const size_t maxCacheFileEntries,
                     const std::string& filename);
  ~CacheFileGenerator();
  
  void addImage(const std::string& filename);
  size_t getImageCount() const {
    return cacheFileEntries.size();
  }
  
private:
  std::ofstream file;
  std::map<MD5Hash, bool> hashes;
  
  const Vec2ui& smallImageResolution;
  const Vec2ui& largeImageBlockSize;
  const size_t maxCacheFileEntries;
  uint64_t offset;
  std::vector<CacheFileEntry> cacheFileEntries;
  
  size_t getHeaderSize() const;
  size_t getVectorSize() const;
};
