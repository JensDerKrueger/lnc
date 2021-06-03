#pragma once

#include <string>
#include <exception>
#include <thread>
#include <mutex>

#include <Vec3.h>
#include <Vec2.h>
#include <Image.h>
#include <MD5.h>

class MosaicMakerException : public std::exception {
  public:
    MosaicMakerException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

class SmallImageInfo {
public:
  SmallImageInfo(const std::string& filename,
                 const Vec2ui& smallImageResolution,
                 const Vec2ui& largeImageBlockSize);
  
  std::string filename;
  std::vector<Vec3t<double>> featureTensor;
  
  bool operator == ( const SmallImageInfo& other ) const {
    return hash == other.hash;
  }

private:
  void computeFeatureTensor(const Vec2ui& largeImageBlockSize,
                            const Vec2ui& smallImageResolution);
  MD5Hash hash;
};

struct Progress {
  std::string stageName{""};
  uint32_t currentElement{0};
  uint32_t targetCount{0};
  bool complete{false};
};

class MosaicMaker {
public:
  MosaicMaker(const std::string& smallDir,
              const std::string& largeImageFilename,
              const uint32_t smallImageWidth,
              const Vec2ui& largeImageBlockSize,
              const Vec2ui& minMaxMinImageDist = {4,7},
              const Vec3t<double>& yuvScale = {1.5,1.0,1.0},
              const double tintScale = 0.5);
  ~MosaicMaker();
  
  void generate();
  void generateAsync();
  Progress getProgress();
  const Image& getResultImage() const;
  Image getResultImage(const uint32_t maxWidth) const;
  
private:
  const std::string smallDir;
  const std::string largeImageFilename;
  const Vec2ui smallImageResolution;
  const Vec2ui largeImageBlockSize;
  const Vec2ui minMaxMinImageDist;
  const Vec3t<double> yuvScale;
  const double tintScale;
  Progress progress;
  
  std::thread computeThread;
 
  Image resultImage;
  Image largeImage;
  std::vector<SmallImageInfo> smallImageInfos;
  std::vector<SmallImageInfo> usedImages;
  
  void updateSmallImageCache();
  void loadLargeImage();
  void generateResultImage();
  
  std::vector<Vec3t<double>> computeFeatureTensor(const uint32_t xBlock, const uint32_t yBlock) const;
  const SmallImageInfo& findBestSmallImage(const std::vector<Vec3t<double>>& largeImageFeatureTensor,
                                           const std::vector<SmallImageInfo>& recentBricks) const;
  void placeSmallImageIntoResult(const uint32_t xBlock, const uint32_t yBlock,
                                 const SmallImageInfo& imageInfo,
                                 const std::vector<Vec3t<double>>& largeImageFeatureTensor);
    
  const std::vector<SmallImageInfo> gatherRecentBricks(uint32_t x, uint32_t y, uint32_t dist);
};
