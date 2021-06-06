#include "CacheFile.h"

#include <filesystem>
#include <algorithm>
#include <limits>

#include <ColorConversion.h>
#include <bmp.h>

std::vector<Vec3t<double>> computeFeatureTensorForImage(const Image& image,
                                                        const Vec2ui& globalStart,
                                                        const Vec2ui& compressionFactor,
                                                        const Vec2ui& blockSize) {
  
  std::vector<Vec3t<double>> featureTensor(blockSize.x * blockSize.y);
  for (uint32_t yBlock = 0;yBlock<blockSize.y;++yBlock) {
    for (uint32_t xBlock = 0;xBlock<blockSize.x;++xBlock) {
      const Vec2ui start = globalStart + Vec2ui(xBlock, yBlock) * compressionFactor;
      Vec3t<double> featureVector{0,0,0};
      for (uint32_t y = start.y;y<start.y+compressionFactor.y;++y) {
        for (uint32_t x = start.x;x<start.x+compressionFactor.x;++x) {
          const Vec3t<double> rgb{
            image.getValue(x,y,0)/255.0,
            image.getValue(x,y,1)/255.0,
            image.getValue(x,y,2)/255.0
          };
          featureVector = featureVector + ColorConversion::rgbToYuv(rgb);
        }
      }
      featureVector = featureVector / double(compressionFactor.x * compressionFactor.y);
      featureTensor[xBlock+yBlock*blockSize.x] = featureVector;
    }
  }
  return featureTensor;
}

CacheFile::CacheFile(const std::string& filename)
{
  load(filename);
}

size_t CacheFile::getImageCount() const {
  return cacheFileEntries.size();
}

std::vector<Vec3t<double>> CacheFile::getFeatureTensor(size_t i) const {
  return cacheFileEntries[i].featureTensor;
}

Image CacheFile::getImage(size_t i) {
  const uint64_t offset = cacheFileEntries[i].offset;
  const uint64_t size = cacheFileEntries[i].size;
  file.seekg(std::streamoff(offset), std::ios_base::beg);
  
  Image image{smallImageResolution.x, smallImageResolution.y, uint32_t(size/(smallImageResolution.x*smallImageResolution.y)) };
  if(!file.read((char*)image.data.data(), std::streamsize(size)))
    throw CacheFileException("Unable to read image in cache file");
  return image;
}

size_t CacheFile::loadHeader() {
  if(!file.read((char*)&(smallImageResolution.x), sizeof(smallImageResolution.x)))
    throw CacheFileException("Unable to read header in cache file 1");
  if(!file.read((char*)&(smallImageResolution.y), sizeof(smallImageResolution.y)))
    throw CacheFileException("Unable to read header in cache file 2");
  if(!file.read((char*)&(largeImageBlockSize.x), sizeof(largeImageBlockSize.x)))
    throw CacheFileException("Unable to read header in cache file 3");
  if(!file.read((char*)&(largeImageBlockSize.y), sizeof(largeImageBlockSize.y)))
    throw CacheFileException("Unable to read header in cache file 4");
  uint64_t temp;
  if(!file.read((char*)&temp, sizeof(temp)))
    throw CacheFileException("Unable to read header in cache file 5");
  size_t fileCount;
  if (temp <= std::numeric_limits<size_t>::max())
    fileCount = size_t(temp);
  else
    throw CacheFileException("Unable to read header in cache file 6");
  return fileCount;
}

void CacheFile::load(const std::string& filename) {
  file.open(filename, std::fstream::binary);
  if (!file.is_open())
    throw CacheFileException("Unable to open cache file");
  
  size_t fileCount = loadHeader();

  for (size_t i = 0;i<fileCount;++i) {
    cacheFileEntries.push_back(CacheFileEntry{file, largeImageBlockSize.x*largeImageBlockSize.y});
  }
}

CacheFileGenerator::CacheFileGenerator(const Vec2ui& smallImageResolution,
                                       const Vec2ui& largeImageBlockSize,
                                       const size_t maxCacheFileEntries,
                                       const std::string& filename) :
smallImageResolution{smallImageResolution},
largeImageBlockSize{largeImageBlockSize},
maxCacheFileEntries{maxCacheFileEntries}
{
  file.open(filename, std::fstream::binary);
  if(!file.write((char*)&(smallImageResolution.x), sizeof(smallImageResolution.x)))
    throw CacheFileException("Unable to write cached image data 1");
  if(!file.write((char*)&(smallImageResolution.y), sizeof(smallImageResolution.y)))
    throw CacheFileException("Unable to write cached image data 2");
  if(!file.write((char*)&(largeImageBlockSize.x), sizeof(largeImageBlockSize.x)))
    throw CacheFileException("Unable to write cached image data 3");
  if(!file.write((char*)&(largeImageBlockSize.y), sizeof(largeImageBlockSize.y)))
    throw CacheFileException("Unable to write cached image data 4");
  uint64_t temp = maxCacheFileEntries;
  if(!file.write((char*)&temp, sizeof(temp)))
    throw CacheFileException("Unable to write cached image data 5");
  offset = getHeaderSize() + getVectorSize();
}

size_t CacheFileGenerator::getHeaderSize() const {
  return sizeof(smallImageResolution.x) +
         sizeof(smallImageResolution.y) +
         sizeof(largeImageBlockSize.x) +
         sizeof(largeImageBlockSize.y) +
         sizeof(uint64_t);
}

size_t CacheFileGenerator::getVectorSize() const {
  return maxCacheFileEntries * (sizeof(uint64_t) + sizeof(uint64_t) + 3*largeImageBlockSize.x*largeImageBlockSize.y*sizeof(double));;
}

CacheFileGenerator::~CacheFileGenerator() {
  // write proper cacheFileEntries.size()
  file.seekp(std::streamoff(getHeaderSize()-sizeof(uint64_t)), std::ios_base::beg);
  uint64_t temp = cacheFileEntries.size();
  file.write((char*)&temp, sizeof(temp));
  
  // write cacheFileEntries-vector
  for (const CacheFileEntry& entry: cacheFileEntries) {
    entry.save(file);
  }
  file.close();
}
  
void CacheFileGenerator::addImage(const std::string& filename) {
  const Image image = BMP::load(filename).cropToAspectAndResample(smallImageResolution.x, smallImageResolution.y);
  const MD5Hash hash = MD5::computeMD5(image.data);
  if (hashes.find(hash) != hashes.end()) return;
  
  const CacheFileEntry e{image, file, smallImageResolution, largeImageBlockSize, offset};
  
  hashes[hash] = true;;
  cacheFileEntries.push_back(e);
  offset += cacheFileEntries.back().size;
}

CacheFileEntry::CacheFileEntry(std::ifstream& file, uint32_t tensorLength) {
  if(!file.read((char*)&(offset), sizeof(offset)))
    throw CacheFileException("Unable to read offset to CacheFileEntry");
  if(!file.read((char*)&(size), sizeof(size)))
    throw CacheFileException("Unable to read size of CacheFileEntry");

  for (uint32_t i = 0;i<tensorLength;++i) {
    double x,y,z;
    if(!file.read((char*)&x, sizeof(x)))
      throw CacheFileException("Unable to read vector in cache file 1");
    if(!file.read((char*)&y, sizeof(y)))
      throw CacheFileException("Unable to read vector in cache file 2");
    if(!file.read((char*)&z, sizeof(z)))
      throw CacheFileException("Unable to read vector in cache file 3");
    featureTensor.push_back({x,y,z});
  }
}

CacheFileEntry::CacheFileEntry(const Image& image,
                               std::ofstream& file,
                               const Vec2ui& smallImageResolution,
                               const Vec2ui& largeImageBlockSize,
                               uint64_t offset) :
  offset{offset}
{
  if (image.componentCount < 3) throw BMP::BMPException("Too few image componets.");
  
  const Vec2ui compressionFactor = smallImageResolution/largeImageBlockSize;
  featureTensor = computeFeatureTensorForImage(image, {0,0}, compressionFactor, largeImageBlockSize);
  
  file.seekp(std::streamoff(offset));
  size = smallImageResolution.x*smallImageResolution.y*image.componentCount;
  file.write((char*)image.data.data(), std::streamsize(size));
}

void CacheFileEntry::save(std::ofstream& file) const {
  if(!file.write((char*)&(offset), sizeof(offset)))
    throw CacheFileException("Unable to write offset of CacheFileEntries");
  if(!file.write((char*)&(size), sizeof(size)))
    throw CacheFileException("Unable to write size of CacheFileEntries");

  for (const Vec3t<double>& e : featureTensor) {
    if(!file.write((char*)&e.x, sizeof(e.x)))
      throw CacheFileException("Unable to write vector into cache file 1");
    if(!file.write((char*)&e.y, sizeof(e.y)))
      throw CacheFileException("Unable to write vector into cache file 2");
    if(!file.write((char*)&e.z, sizeof(e.z)))
      throw CacheFileException("Unable to write vector into cache file 3");
  }
}

