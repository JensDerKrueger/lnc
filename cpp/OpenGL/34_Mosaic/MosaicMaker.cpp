#include "MosaicMaker.h"

#include <filesystem>
#include <algorithm>
#include <limits>

#include <bmp.h>
#include <ColorConversion.h>

static std::vector<Vec3t<double>> computeFeatureTensorForImage(const Image& image,
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
  computeThread.join();
}

void MosaicMaker::updateSmallImageCache() {
  
  const std::string cacheFilename{smallDir+"/cache.data"};
  try {
    cacheFile = std::make_shared<CacheFile>(cacheFilename);
    return;
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

  cacheFile = std::make_shared<CacheFile>(smallImageResolution, largeImageBlockSize, files.size(), cacheFilename);

  
  startProgress(uint32_t(files.size()));
  uint32_t element{1};
  
  for (const std::string& filename: files) {
    setProgress(element++);
    try {
      SmallImageInfo info{filename, smallImageResolution, largeImageBlockSize};
    // TODO  cacheFile.add(info, image);
    } catch (...) {
    }
  }
  
  if (cacheFile->getImageCount() == 0)
    throw MosaicMakerException("Unable to load small images");

  cacheFile->save();

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
                                       const std::vector<SmallImageInfo>& recentBricks) const {
  size_t minElementIndex = 0;
  double minDist = std::numeric_limits<double>::max();
  for (size_t i=0;i<cacheFile->getImageCount();++i) {
    if (std::find(recentBricks.begin(), recentBricks.end(), cacheFile->getImageInfo(i)) != recentBricks.end())
      continue;

    double currentDist{0.0};
    for (size_t j=0;j<largeImageFeatureTensor.size();++j) {
      currentDist += ((cacheFile->getImageInfo(i).featureTensor[j]-
                      largeImageFeatureTensor[j])*yuvScale).length();
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
  const SmallImageInfo imageInfo = cacheFile->getImageInfo(imageIndex);
  
  Vec3t<double> errorVec{0,0,0};
  for (size_t j=0;j<largeImageFeatureTensor.size();++j) {
    errorVec = errorVec + ((imageInfo.featureTensor[j]-largeImageFeatureTensor[j])*yuvScale);
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
  
  usedImages.push_back(imageInfo);
}

const std::vector<SmallImageInfo> MosaicMaker::gatherRecentBricks(uint32_t x, uint32_t y,
                                                                  uint32_t dist) {
  
  const uint32_t xBricks = largeImage.width/largeImageBlockSize.x;
  std::vector<SmallImageInfo> result;
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
      const std::vector<SmallImageInfo> recentBricks = gatherRecentBricks(x,y,minImageDist);
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

SmallImageInfo::SmallImageInfo(const std::string& filename,
                               const Vec2ui& smallImageResolution,
                               const Vec2ui& largeImageBlockSize) :
  filename{filename}
{
  computeFeatureTensor(largeImageBlockSize, smallImageResolution);
}

void SmallImageInfo::computeFeatureTensor(const Vec2ui& largeImageBlockSize,
                                          const Vec2ui& smallImageResolution) {
  Image image = BMP::load(filename).cropToAspectAndResample(smallImageResolution.x, smallImageResolution.y);
  
  if (image.componentCount < 3) throw BMP::BMPException("Too few image componets.");
  hash = MD5::computeMD5(image.data);
    
  const Vec2ui compressionFactor = smallImageResolution/largeImageBlockSize;
  featureTensor = computeFeatureTensorForImage(image, {0,0}, compressionFactor, largeImageBlockSize);
}


CacheFile::CacheFile(const std::string& filename) :
filename(filename)
{
  load();
}

CacheFile::CacheFile(const Vec2ui& smallImageResolution,
                     const Vec2ui& largeImageBlockSize,
                     const size_t maxSmallFiles,
                     const std::string& filename) :
smallImageResolution(smallImageResolution),
largeImageBlockSize(largeImageBlockSize),
maxSmallFiles(maxSmallFiles),
filename(filename)
{
  create();
}

void CacheFile::addImage(const SmallImageInfo& info, const Image& image) {
  smallImageInfos.push_back(std::make_pair(info, offset));
  const size_t imageSize = smallImageResolution.x*smallImageResolution.y*3;
  file.write((char*)image.data.data(), std::streamsize(imageSize));
  offset += imageSize;
}

void CacheFile::save() {
  // TODO: write vector to file
  
  file.flush();
}

size_t CacheFile::getImageCount() const {
  return smallImageInfos.size();
}

SmallImageInfo CacheFile::getImageInfo(size_t i) const {
  return smallImageInfos[i].first;
}

Image CacheFile::getImage(size_t i) {
  const uint64_t offset = smallImageInfos[i].second;
  file.seekg(std::streamoff(offset), std::ios_base::beg);
  Image image{smallImageResolution.x, smallImageResolution.y, 3};
  if(!file.read((char*)image.data.data(), smallImageResolution.x*smallImageResolution.y*3))
    throw MosaicMakerException("Unable to read cached image data");
  return image;
}

void CacheFile::load() {
  std::fstream stream(filename, std::ios::in | std::ios::out | std::ios::binary);
  if (!stream.is_open())
    throw MosaicMakerException("Unable to open cache file");
  
  // load header
  if(!file.read((char*)&(smallImageResolution.x), sizeof(smallImageResolution.x)))
    throw MosaicMakerException("Unable to read cached image data");
  if(!file.read((char*)&(smallImageResolution.y), sizeof(smallImageResolution.y)))
    throw MosaicMakerException("Unable to read cached image data");
  if(!file.read((char*)&(largeImageBlockSize.x), sizeof(largeImageBlockSize.x)))
    throw MosaicMakerException("Unable to read cached image data");
  if(!file.read((char*)&(largeImageBlockSize.y), sizeof(largeImageBlockSize.y)))
    throw MosaicMakerException("Unable to read cached image data");
  uint64_t temp;
  if(!file.read((char*)&temp, sizeof(uint64_t)))
    throw MosaicMakerException("Unable to read cached image data");
  if (temp <= std::numeric_limits<size_t>::max())
    maxSmallFiles = temp;
  else
    throw MosaicMakerException("Unable to read cached image data");
  
  // load smallImageInfos
}

void CacheFile::create() {
  // TODO:
  // open file
  // store parameters in header
  // init offset
}
