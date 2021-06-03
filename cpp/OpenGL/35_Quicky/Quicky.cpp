#include "Quicky.h"

#include <filesystem>
#include <algorithm>
#include <cstdio>

#include <bmp.h>

Quicky::Quicky(const std::string& imageDirectory,
               const Vec2ui& targetImageSize,
               const size_t skipImages) :
imageDirectory(imageDirectory),
targetImageSize(targetImageSize),
currentImageIndex(0)
{
  scanDir(false);  
  currentImageIndex = std::min(filenames.size()-1,skipImages);
  load();
}

void Quicky::scanDir(bool validateFiles) {
  for (auto& p: std::filesystem::directory_iterator(imageDirectory)) {
    try {
      if (p.path().extension() != ".bmp") continue;
      if (validateFiles) BMP::load(p.path().string());
      filenames.push_back(p.path().string());
    } catch (...) {
    }
    sort(filenames.begin(), filenames.end());
  }
}

void Quicky::reset() {
  load();
}

void Quicky::next() {
  if (currentImageIndex+1 < getImageCount()) {
    currentImageIndex++;
    load();
  }
}

void Quicky::previous() {
  if (currentImageIndex > 0) {
    currentImageIndex--;
  } else {
    currentImageIndex = getImageCount()-1;
  }
  load();
}

bool Quicky::getEdited() const {
  return edited;
}

void Quicky::save() {
  BMP::save(filenames[currentImageIndex], currentImage);
  edited = false;
}

void Quicky::load() {
  currentImage = BMP::load(filenames[currentImageIndex]);
  edited = false;
}

std::string Quicky::getFilename() const {
  return filenames[currentImageIndex];
}

const Image& Quicky::getCurrentImage() const {
  return currentImage;
}

size_t Quicky::getImageCount() const {
  return filenames.size();
}

size_t Quicky::getCurrentImageIndex() const {
  return currentImageIndex;
}

void Quicky::rotate90() {
  Image result{currentImage.height, currentImage.width, currentImage.componentCount};
  for (uint32_t y = 0;y<currentImage.height;++y) {
    for (uint32_t x = 0;x<currentImage.width;++x) {
      for (uint32_t c = 0;c<currentImage.componentCount;++c) {
        result.setValue(currentImage.height-y-1,x,c,currentImage.getValue(x,y,c));
      }
    }
  }
  currentImage = result;
  edited = true;
}

void Quicky::rotate180() {
  Image result{currentImage.width, currentImage.height, currentImage.componentCount};
  for (uint32_t y = 0;y<currentImage.height;++y) {
    for (uint32_t x = 0;x<currentImage.width;++x) {
      for (uint32_t c = 0;c<currentImage.componentCount;++c) {
        result.setValue(currentImage.width-x-1,currentImage.height-y-1,c,currentImage.getValue(x,y,c));
      }
    }
  }
  currentImage = result;
  edited = true;
}

void Quicky::rotate270() {
  Image result{currentImage.height, currentImage.width, currentImage.componentCount};
  for (uint32_t y = 0;y<currentImage.height;++y) {
    for (uint32_t x = 0;x<currentImage.width;++x) {
      for (uint32_t c = 0;c<currentImage.componentCount;++c) {
        result.setValue(y,currentImage.width-x-1,c,currentImage.getValue(x,y,c));
      }
    }
  }
  currentImage = result;
  edited = true;
}

Vec2ui Quicky::getTargetImageSize() const {
  return targetImageSize;
}

void Quicky::crop(const std::array<Vec2,2>& cropBox) {
  const Vec2 bottomLeft = cropBox[0]*Vec2(currentImage.width, currentImage.height);
  const Vec2 topRight = cropBox[1]*Vec2(currentImage.width, currentImage.height);
  
  currentImage = currentImage.crop(uint32_t(std::max(0.0f,bottomLeft.x)),
                                   uint32_t(std::max(0.0f,bottomLeft.y)),
                                   uint32_t(std::min<float>(currentImage.width-1,topRight.x)),
                                   uint32_t(std::min<float>(currentImage.height-1,topRight.y)));
  edited = true;
}

void Quicky::erase() {
  if (getImageCount() <= 1) return;
  remove(filenames[currentImageIndex].c_str());
  filenames.erase(filenames.begin()+long(currentImageIndex));
  if (currentImageIndex >= getImageCount()) currentImageIndex = getImageCount()-1;
  load();
}
