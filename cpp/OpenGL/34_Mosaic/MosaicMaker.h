#pragma once

#include <string>
#include <exception>
#include <thread>
#include <mutex>
#include <fstream>

#include <Vec3.h>
#include <Vec2.h>
#include <Image.h>
#include <MD5.h>

#include "CacheFile.h"

class MosaicMakerException : public std::exception {
  public:
    MosaicMakerException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
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
  std::shared_ptr<CacheFile> cacheFile{nullptr};
  
  std::thread computeThread;
  std::mutex progressMutex;
 
  Image resultImage;
  Image largeImage;
  std::vector<size_t> usedImages;
  
  void updateSmallImageCache();
  void loadLargeImage();
  void generateResultImage();
  void setProgressStage(const std::string& name);
  void startProgress(uint32_t targetCount);
  void setProgress(uint32_t element);
  void progressComplete();
  
  std::vector<Vec3t<double>> computeFeatureTensor(const uint32_t xBlock, const uint32_t yBlock) const;
  size_t findBestSmallImage(const std::vector<Vec3t<double>>& largeImageFeatureTensor,
                            const std::vector<size_t>& recentBricks) const;
  void placeSmallImageIntoResult(const uint32_t xBlock, const uint32_t yBlock,
                                 const size_t imageIndex,
                                 const std::vector<Vec3t<double>>& largeImageFeatureTensor);
    
  const std::vector<size_t> gatherRecentBricks(uint32_t x, uint32_t y, uint32_t dist);
};
