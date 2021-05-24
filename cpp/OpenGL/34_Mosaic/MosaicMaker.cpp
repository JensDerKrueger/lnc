#include "MosaicMaker.h"

#include <bmp.h>

MosaicMaker::MosaicMaker(const std::string& smallDir,
                         const std::string& largeImageFilename,
                         const Vec2ui& smallImageResolution,
                         const Vec2ui& largeImageResolution) :
smallDir{smallDir},
largeImageFilename{largeImageFilename},
smallImageResolution{smallImageResolution},
largeImageResolution{largeImageResolution}
{
}


void MosaicMaker::updateSmallImageCache() {
  // TODO: iterate over files in directory
  if (smallImages.empty())
    throw MosaicMakerException("Unable to load small images");
}

void MosaicMaker::loadLargeImage() {
  try {
    largeImage = BMP::load(largeImageFilename);
  } catch (...) {
    throw MosaicMakerException("Unable to load large image");
  }
}

void MosaicMaker::generateResultImage() {
  // TODO: iterate over image and find best candidates
  resultImage = Image::genTestImage(300,300);
}

void MosaicMaker::generate() {
  updateSmallImageCache();
  loadLargeImage();
  generateResultImage();
}

Image MosaicMaker::getResultImage() const {
  return resultImage;
}

Image MosaicMaker::getResultImage(const uint32_t maxWidth) const {
  if (maxWidth < resultImage.width)
    return resultImage.resample(maxWidth);
  else
    return resultImage;
}

SmallImageInfo::SmallImageInfo(const std::string& filename,
                               const Vec3t<double> featureVec) :
filename{filename},
featureVec{featureVec}
{
}

SmallImageInfo::SmallImageInfo(const std::string& filename) :
  filename{filename}
{
  computeFeatureVector();
}

void SmallImageInfo::computeFeatureVector() {
  Image image = BMP::load(filename);
  if (image.componentCount < 3) throw BMP::BMPException("Too few image componets.");
  featureVec = Vec3t<double>{0,0,0};
  for (uint32_t y = 0;y<image.height;++y) {
    for (uint32_t x = 0;x<image.width;++x) {
      const Vec3 rgb{image.getValue(x,y,0)/255.0f,
                     image.getValue(x,y,1)/255.0f,
                     image.getValue(x,y,2)/255.0f};
      featureVec = featureVec + Vec3t<double>(Vec3::rgbToHsv(rgb));
    }
  }
  featureVec = featureVec / double(image.height * image.width);
}
