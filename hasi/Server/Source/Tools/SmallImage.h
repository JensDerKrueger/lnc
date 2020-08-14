#ifndef IVDA_SMALLIMAGE
#define IVDA_SMALLIMAGE

#include "StdDefines.h"
#include "Vectors.h"
#include <string>

//#define SMIM_JPG_SUPPORT
#define SMIM_IMG_MAGIC_SUPPORT

namespace IVDA
{
  typedef VECTOR3<uint8_t> Color;

  class SmallImage {
    public:
       SmallImage();
       SmallImage(unsigned int width, unsigned int height, unsigned int iComponentCount);
       SmallImage(Vec2ui const& size, unsigned int iComponentCount);
       SmallImage(const std::string& filename);
       SmallImage(const SmallImage& other);

       SmallImage& operator=(const SmallImage& other);
       virtual ~SmallImage(void);

       static bool PeekBMPHeader(const std::string& filename, Vec2ui& size, unsigned int& iComponentCount);
#ifdef SMIM_IMG_MAGIC_SUPPORT
       static std::string Convert(const std::string& filename, const std::string& newExt);
#endif
       bool LoadFromBMPFile(const std::string& filename);
#ifdef SMIM_JPG_SUPPORT
       bool LoadFromJPGMem(const std::vector<uint8_t>& vData);
       bool LoadFromJPGFile(const std::string& filename);
       bool SaveToJPGMem(std::vector<uint8_t>& vData) const;
       bool SaveToJPGFile(const std::string& filename) const;
#endif
	  bool SaveToRAWFile(const std::string& filename) const;
	  bool SaveToBMPFile(const std::string& filename) const;
  
    // set non-premultiplied pixel values (if premultiplication is enabled the colors get converted automatically)
	  void SetPixel(unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	  void SetPixel(unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b);
	  void SetPixel(unsigned int x, unsigned int y, uint8_t grey);
	  void SetPixel(unsigned int x, unsigned int y, const Color& c);
  
	  void GetPixel(unsigned int x, unsigned int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;
	  void GetPixel(unsigned int x, unsigned int y, uint8_t& r, uint8_t& g, uint8_t& b) const;
	  void GetPixel(unsigned int x, unsigned int y, uint8_t& grey) const;
	  void GetPixel(unsigned int x, unsigned int y, Color& c) const;
	  Color GetPixel(unsigned int x, unsigned int y) const;
  
	  int ComponentCount() const {return m_iComponentCount;}
	  int Height() const {return m_size.y;}
	  int Width() const {return m_size.x;}
	  int Area() const {return m_size.area();}
	  const Vec2ui& GetSize() const {return m_size;}
  
    bool IsAlphaPremultiplied() const { return m_bPremultipliedAlpha; }
    void PremultiplyAlpha(bool bTrueFalse); // apply alpha premultiplication or revert to non-premultiplied colors if possible
	  void ForceComponentCount(unsigned int newCompCount, uint8_t padValue=255);
	  void Resample(unsigned int newWidth, unsigned int newHeight, bool bKeepAspect=false);
	  SmallImage* GeneratePreviewImage(unsigned int newWidth, unsigned int newHeight, bool bKeepAspect=false);
  
	  const uint8_t* GetDataPtr() const { return m_pData;}
	  uint8_t* GetDataPtrRW() { return m_pData;}
  
	
	private:
    bool           m_bPremultipliedAlpha;
	  Vec2ui    m_size;
	  unsigned int   m_iComponentCount;
	  uint8_t       *m_pData;
	
	  static bool PeekBMPHeader(const std::string& filename, Vec2ui& size, unsigned int& iComponentCount, bool& bUpsideDown, int& iOffsetToData);
	
	  void InitData();
  
	  size_t OneDIndex(unsigned int x, unsigned int y) const { return size_t(m_iComponentCount*(x+y*m_size.x)); }
	  void Resample(uint8_t* pTarget, unsigned int newWidth, unsigned int newHeight);
	  void AdjustToAspect(unsigned int& newWidth, unsigned int& newHeight);
	};
}

#endif // SMALLIMAGE_H
