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
  SmallImageInfo(const std::string& filename, const Vec3t<int>& hsv);
  SmallImageInfo(const std::string& filename);
  
  std::string filename;
  Vec3t<int> hsv;

private:
  void computeAverageColor();
};

class MosaicMaker {
public:
  MosaicMaker(const std::string& smallDir,
              const std::string& largeImageFilename,
              const Vec2ui& smallImageResolution,
              const Vec2ui& largeImageResolution);
  
  void generate();
  Image getResultImage() const;
  
private:
  const std::string smallDir;
  const std::string largeImageFilename;
  const Vec2ui smallImageResolution;
  const Vec2ui largeImageResolution;
 
  Image resultImage;
  Image largeImage;
  std::vector<SmallImageInfo> smallImages;
  
  void updateSmallImageCache();
  void loadLargeImage();
  void generateResultImage();
  
};
