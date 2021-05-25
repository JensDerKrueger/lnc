#pragma once

#include <string>
#include <exception>

#include <Vec3.h>
#include <Vec2.h>
#include <Image.h>

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
  SmallImageInfo(const std::string& filename, const Vec3t<double> featureVec);
  SmallImageInfo(const std::string& filename);
  
  std::string filename;
  Vec3t<double> featureVec;

private:
  void computeFeatureVector();
};

class MosaicMaker {
public:
  MosaicMaker(const std::string& smallDir,
              const std::string& largeImageFilename,
              const Vec2ui& smallImageResolution,
              const Vec2ui& largeImageBlockSize);
  
  void generate();
  Image getResultImage() const;
  Image getResultImage(const uint32_t maxWidth) const;
  
private:
  const std::string smallDir;
  const std::string largeImageFilename;
  const Vec2ui smallImageResolution;
  const Vec2ui largeImageBlockSize;
 
  Image resultImage;
  Image largeImage;
  std::vector<SmallImageInfo> smallImageInfos;
  
  void updateSmallImageCache();
  void loadLargeImage();
  void generateResultImage();
  
  Vec3t<double> computeFeatureVec(const uint32_t xBlock, const uint32_t yBlock) const;
  const SmallImageInfo& findBestSmallImage(const Vec3t<double>& largeImageFeatureVec) const;
  void placeSmallImageIntoResult(const uint32_t xBlock, const uint32_t yBlock,
                                 const SmallImageInfo& imageInfo);
    
  
};
