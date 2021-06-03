#pragma once

#include <string>
#include <exception>

#include <Vec3.h>
#include <Vec2.h>
#include <Image.h>

class Quicky {
public:
  Quicky(const std::string& imageDirectory,
         const Vec2ui& targetImageSize,
         const size_t skipImages=0);
  
  const Image& getCurrentImage() const;
  
  void rotate90();
  void rotate180();
  void rotate270();
  void crop(const std::array<Vec2,2>& cropBox);
  void reset();
  void next() ;
  void previous();
  void save();
  void erase();
  
  bool getEdited() const;
  Vec2ui getTargetImageSize() const;
  size_t getImageCount() const;
  size_t getCurrentImageIndex() const;
  std::string getFilename() const;

private:
  const std::string imageDirectory;
  const Vec2ui targetImageSize;
  std::vector<std::string> filenames;
  Image currentImage;
  size_t currentImageIndex;
  bool edited;

  void load();
  void scanDir(bool validateFiles);
};
