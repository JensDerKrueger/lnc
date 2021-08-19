#include "MosaicMaker.h"

#include <filesystem>
#include <algorithm>
#include <limits>

#include <bmp.h>
#include <ColorConversion.h>

MosaicMaker::MosaicMaker(const std::string& smallDir,
                         const std::string& largeImageFilename,
                         const uint32_t smallImageWidth,
                         const Vec2ui& largeImageBlockSize,
                         const Vec2ui& minMaxMinImageDist,
                         const Vec3t<double>& yuvScale,
                         const double tintScale) :
smallDir{smallDir},
largeImageFilename{largeImageFilename},
smallImageResolution{smallImageWidth, (smallImageWidth*largeImageBlockSize.y)/largeImageBlockSize.x},
largeImageBlockSize{largeImageBlockSize},
minMaxMinImageDist{minMaxMinImageDist},
yuvScale(yuvScale),
tintScale(tintScale)
{
}

MosaicMaker::~MosaicMaker() {
  if (computeThread.joinable()) computeThread.join();
}


void MosaicMaker::updateSmallImageCache() {
  
  const std::string cacheFilename{smallDir+"/cache.data"};
  try {
    cacheFile = std::make_shared<CacheFile>(cacheFilename);
    if (cacheFile->getSmallImageResolution() == smallImageResolution &&
        cacheFile->getLargeImageBlockSize() == largeImageBlockSize) return;
    cacheFile = nullptr;    
  } catch (...) {
  }
  
  setProgressStage("Updating Small Image Cache");

  std::vector<std::string> files;
  for (auto& p: std::filesystem::directory_iterator(smallDir)) {
    try {
      if (p.path().extension() != ".bmp") continue;
      files.push_back(p.path().string());
    } catch (...) {
    }
  }

  try {
    CacheFileGenerator gen(smallImageResolution, largeImageBlockSize, files.size(), cacheFilename);

    startProgress(uint32_t(files.size()));
    uint32_t element{1};
    
    for (const std::string& filename: files) {
      setProgress(element++);
      try {
        gen.addImage(filename);
      } catch (...) {
      }
    }
    
    if (gen.getImageCount() == 1)
      throw MosaicMakerException("No small images found");
    
  } catch (...) {
    throw MosaicMakerException("Unable to create image database");
  }

  try {
    cacheFile = std::make_shared<CacheFile>(cacheFilename);
  } catch (...) {
    throw MosaicMakerException("Unable to load image database");
  }


}

void MosaicMaker::loadLargeImage() {
  setProgressStage("Loading Large Image");
  try {
    largeImage = BMP::load(largeImageFilename);
  } catch (...) {
    throw MosaicMakerException("Unable to load large image");
  }
}

std::vector<Vec3t<double>> MosaicMaker::computeFeatureTensor(const uint32_t xBlock,
                                                             const uint32_t yBlock) const {
  const Vec2ui start{xBlock*largeImageBlockSize.x, yBlock*largeImageBlockSize.y};
  return computeFeatureTensorForImage(largeImage, start, {1,1}, largeImageBlockSize);
}


size_t MosaicMaker::findBestSmallImage(const std::vector<Vec3t<double>>& largeImageFeatureTensor,
                                       const std::vector<size_t>& recentBricks) const {
  size_t minElementIndex = 0;
  double minDist = std::numeric_limits<double>::max();
  for (size_t i=0;i<cacheFile->getImageCount();++i) {
    if (std::find(recentBricks.begin(), recentBricks.end(), i) != recentBricks.end())
      continue;

    double currentDist{0.0};
    const std::vector<Vec3t<double>> featureTensor = cacheFile->getFeatureTensor(i);
    for (size_t j=0;j<largeImageFeatureTensor.size();++j) {
      currentDist += ((featureTensor[j]-largeImageFeatureTensor[j])*yuvScale).length();
    }
    
    if (currentDist < minDist) {
      minDist = currentDist;
      minElementIndex = i;
    }

  }
  return minElementIndex;
}

void MosaicMaker::placeSmallImageIntoResult(const uint32_t xBlock, const uint32_t yBlock,
                                            const size_t imageIndex,
                                            const std::vector<Vec3t<double>>& largeImageFeatureTensor) {
  const uint32_t xStart = xBlock*smallImageResolution.x;
  const uint32_t yStart = yBlock*smallImageResolution.y;
   
  const Image smallImage = cacheFile->getImage(imageIndex);
  const std::vector<Vec3t<double>> featureTensor = cacheFile->getFeatureTensor(imageIndex);
  
  Vec3t<double> errorVec{0,0,0};
  for (size_t j=0;j<largeImageFeatureTensor.size();++j) {
    errorVec = errorVec + ((featureTensor[j]-largeImageFeatureTensor[j])*yuvScale);
  }
  errorVec = ColorConversion::yuvToRgb(errorVec / largeImageFeatureTensor.size()) * tintScale;
 
  for (uint32_t y = yStart;y<yStart+smallImageResolution.y;++y) {
    for (uint32_t x = xStart;x<xStart+smallImageResolution.x;++x) {
      resultImage.setValue(x,y,0, uint8_t(std::clamp<double>(smallImage.getValue(x-xStart,y-yStart,0) -
                                                             errorVec.r*255, 0, 255)));
      resultImage.setValue(x,y,1, uint8_t(std::clamp<double>(smallImage.getValue(x-xStart,y-yStart,1) -
                                                             errorVec.g*255, 0, 255)));
      resultImage.setValue(x,y,2, uint8_t(std::clamp<double>(smallImage.getValue(x-xStart,y-yStart,2) -
                                                             errorVec.b*255, 0, 255)));
      resultImage.setValue(x,y,3,255);
    }
  }
  
  usedImages.push_back(imageIndex);
}

const std::vector<size_t> MosaicMaker::gatherRecentBricks(uint32_t x, uint32_t y,
                                                                  uint32_t dist) {
  
  const uint32_t xBricks = largeImage.width/largeImageBlockSize.x;
  std::vector<size_t> result;
  const int64_t currentIndex = int64_t(x + y * xBricks);
  
  for (int64_t dy = -int64_t(dist);dy<=0;++dy) {
    for (int64_t dx = -int64_t(dist);dx<int64_t(dist);++dx) {
      const int64_t testIndex = (int64_t(x)+dx) + (int64_t(y)+dy) * int64_t(xBricks);
      if (int64_t(x)+dx < 0 || (int64_t(y)+dy) * int64_t(xBricks) < 0 || testIndex >= currentIndex)
        continue;
      result.push_back(usedImages[uint32_t(testIndex)]);
    }
  }
  
  return result;
}

void MosaicMaker::generateResultImage() {
  setProgressStage("Generating result image");

  const uint32_t largeWidth  = (largeImage.width / largeImageBlockSize.x) *
                                smallImageResolution.x;
  const uint32_t largeHeight = (largeImage.height / largeImageBlockSize.y) *
                                smallImageResolution.y;
  resultImage = Image{largeWidth, largeHeight, 4};
  
  const uint32_t xBricks = largeImage.width/largeImageBlockSize.x;
  const uint32_t yBricks = largeImage.height/largeImageBlockSize.y;

  setProgressStage("Filling result image");
  startProgress(yBricks);
  
  for (uint32_t y = 0;y<yBricks;++y) {
    setProgress(y+1);
    for (uint32_t x = 0;x<xBricks;++x) {
      const std::vector<Vec3t<double>> featureTensor = computeFeatureTensor(x,y);
      const uint32_t minImageDist = minMaxMinImageDist[0] == minMaxMinImageDist[1] ? minMaxMinImageDist[0] : Rand::rand<uint32_t>(minMaxMinImageDist[0],minMaxMinImageDist[1]);
      const std::vector<size_t> recentBricks = gatherRecentBricks(x,y,minImageDist);
      const size_t index = findBestSmallImage(featureTensor, recentBricks);
      placeSmallImageIntoResult(x,y, index, featureTensor);
    }
  }
}

void MosaicMaker::generate() {
  loadLargeImage();
  updateSmallImageCache();
  generateResultImage();

  progressComplete();
}

void MosaicMaker::generateAsync() {
  computeThread = std::thread(&MosaicMaker::generate, this);
}

void MosaicMaker::setProgress(uint32_t element) {
  const std::scoped_lock<std::mutex> lock(progressMutex);
  progress.currentElement = element;
}

void MosaicMaker::startProgress(uint32_t targetCount) {
  const std::scoped_lock<std::mutex> lock(progressMutex);
  progress.currentElement = 0;
  progress.targetCount = targetCount;
}

void MosaicMaker::setProgressStage(const std::string& name) {
  const std::scoped_lock<std::mutex> lock(progressMutex);
  progress.stageName = name;
  progress.currentElement = 0;
  progress.targetCount = 0;
}

void MosaicMaker::progressComplete() {
  const std::scoped_lock<std::mutex> lock(progressMutex);
  progress.complete = true;
}

Progress MosaicMaker::getProgress() {
  const std::scoped_lock<std::mutex> lock(progressMutex);
  return progress;
}

const Image& MosaicMaker::getResultImage() const {
  return resultImage;
}

Image MosaicMaker::getResultImage(const uint32_t maxWidth) const {
  if (maxWidth < resultImage.width)
    return resultImage.resample(maxWidth);
  else
    return resultImage;
}
