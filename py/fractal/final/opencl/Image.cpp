/*************************************
*        Quick und Dirty             *
*       OpenCL Fractal Test          *
*                                    *
*        (c) 2016 Jens Krüger        *
*  This code is in the public domain *
**************************************/

#include <fstream>
#include "Image.h"

void Image::setData(unsigned int x, unsigned int y, char value) {
  target[y*w+x] = value;
}

char Image::getData(unsigned int x, unsigned int y) const {
  return target[y*w+x];
}

std::string Image::getExt(const std::string& filename) const {
  size_t i = filename.find_last_of('.');
  
  if (i != std::string::npos) {
    return filename.substr(i+1);
  } else {
    return "";
  }
}

std::string Image::toString() const {
  std::string s = "";
  for (int y = 0;y<h;++y) {
    for (int x = 0;x<w;++x) {
      char c = getData(x,y);
      s += (c < 50) ? "." : "X";
    }
    s += "\n";
  }
  return s;
}

bool Image::save(const std::string& filename) {
  const uint8_t iComponentCount = 3;
  
  std::ofstream outStream(filename.c_str(), std::ofstream::binary);
  if (!outStream.is_open()) return false;
  
  // write BMP-Header
  outStream.write((char*)"BM", 2); // all BMP-Files start with "BM"
  uint32_t header[3];
  int rowPad= 4-((w*8*iComponentCount)%32)/8;
  if (rowPad == 4) rowPad = 0;
  header[0] = 54+w*h*iComponentCount+rowPad*h;	// filesize = 54 (header) + sizeX * sizeY * numChannels
  header[1] = 0;						      // reserved = 0 (4 Bytes)
	header[2] = 54;						      // File offset to Raster Data
	outStream.write((char*)header, 4*3);
	// write BMP-Info-Header
  uint32_t infoHeader[10];
	infoHeader[0] = 40;	          // size of info header
	infoHeader[1] = w;            // Bitmap Width
	infoHeader[2] = -h;           // Bitmap Height (negative to flip image)
	infoHeader[3] = 1+65536*8*iComponentCount;
  // first 2 bytes=Number of Planes (=1)
  // next  2 bytes=BPP
	infoHeader[4] = 0;				  	// compression (0 = none)
	infoHeader[5] = 0;					  // compressed file size (0 if no compression)
	infoHeader[6] = 11810;				// horizontal resolution: Pixels/meter (11810 = 300 dpi)
	infoHeader[7] = 11810;				// vertical resolution: Pixels/meter (11810 = 300 dpi)
	infoHeader[8] = 0;					  // Number of actually used colors
  infoHeader[9] = 0;					  // Number of important colors  0 = all
	outStream.write((char*)infoHeader, 4*10);
  
  // data in BMP is stored BGR, so convert scalar BGR
  std::vector<char> pData(iComponentCount*w*h);
  
  uint32_t i = 0;
  for (int y = 0;y<h;++y) {
    for (int x = 0;x<w;++x) {
      
      char r = getData(x,y);
      char g = (char)(r*3);
      char b = (char)(r*25);
      char a = char(255);
      
      pData[i++]   = b;
      pData[i++] = g;
      pData[i++] = r;
      if (iComponentCount==4) pData[i++] = a;
    }
  }
  
  // write data (pad if necessary)
  if (rowPad==0) {
    outStream.write(&(pData[0]), iComponentCount*w*h);
  }
  else {
    uint8_t zeroes[9]={0,0,0,0,0,0,0,0,0};
    for (int i=0; i<h; i++) {
      outStream.write(&(pData[iComponentCount*i*w]), iComponentCount*w);
      outStream.write((char*)zeroes, rowPad);
    }
  }
  
  outStream.close();
  return true;
}
