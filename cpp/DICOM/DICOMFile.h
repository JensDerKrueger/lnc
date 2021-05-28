#pragma once

#include <string>
#include <fstream>
#include <exception>
#include <vector>

#include "DICOMTable.h"

// if the following define is set, the DICOM parser outputs detailed parsing
// information. Be careful with this option: it creates a massive amount of
// output.
//#define DEBUG_DICOM

#ifdef DEBUG_DICOM
  #include <iostream>
  #define DICOM_DBG(s) std::cout << s << std::endl;
  #define DICOM_DBG_NL(s) std::cout << s;
#else
  #define DICOM_DBG(s)
  #define DICOM_DBG_NL(s)
#endif

template <typename T>
struct DCMVec3t {
  T x{T(1)};
  T y{T(1)};
  T z{T(1)};
  bool operator == (const DCMVec3t& other) const {
    return x == other.x &&
           y == other.y &&
           z == other.z;
  }
};

typedef DCMVec3t<float> DCMVec3;
typedef DCMVec3t<uint32_t> DCMVec3ui;

class DICOMFileException : public std::exception {
  public:
    DICOMFileException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};


struct ElementInfo {
  uint16_t groupID{0};
  uint16_t elementID{0};
  uint32_t elementLength{0};
  DICOM_eType elementType{DICOM_eType::TYPE_UN};
};

struct MetaHeader {
  std::string filename{""};
  bool implicitFileType{false};
  bool isJPEGEncoded{false};
  bool isBigEndian{false};
  bool needsEndianConversion{false};
  uint32_t offsetToEnd{0};
  bool complete{false};
};


class DICOMFile {
public:
  DICOMFile(const std::string& filename);

  std::vector<uint8_t> getData() const {
    return getData(m_iRawDataSize);
  }
  std::vector<uint8_t> getData(uint32_t count) const;
  uint32_t getRawDataSize() const {
    return m_iRawDataSize;
  }
  
  DCMVec3 getPatientPosition() const {
    return m_fvPatientPosition;
  }
  
  float getSliceLocation() const {
    return m_fSliceLocation;
  }
  
  void correctZAspect(float z) {
    m_fvfAspect.z = z;
  }
  
  DCMVec3 getAspect() const {
    return m_fvfAspect;
  }

  DCMVec3ui getSize() const {
    return m_ivSize;
  }

  uint32_t getComponentCount() const {
    return m_iComponentCount;
  }

  uint32_t getAllocated() const {
    return m_iAllocated;
  }

  bool match(const DICOMFile& other) const;
    
  bool stackLessCompare(const DICOMFile& other) const;
  bool depthLessCompare(const DICOMFile& other) const;
    
  
private:
  MetaHeader metaHeader;
  
  uint32_t      m_iSeries;
  DCMVec3ui     m_ivSize;
  DCMVec3       m_fvfAspect;
  uint32_t      m_iAllocated;
  uint32_t      m_iStored;
  std::string   m_strAcquDate;
  std::string   m_strAcquTime;
  std::string   m_strModality;
  std::string   m_strDesc;
  DCMVec3       m_fvPatientPosition;
  DCMVec3       m_fvPatientOrientation1;
  DCMVec3       m_fvPatientOrientation2;
  bool          m_bSigned;
  float         m_fWindowCenter;
  float         m_fWindowWidth;
  float         m_fBias;
  float         m_fScale;
  uint32_t      m_iImageIndex;
  uint32_t      m_iComponentCount;
  float         m_fSliceSpacing;
  float         m_fSliceLocation;
  
  uint32_t      m_iOffset;
  uint32_t      m_iRawDataSize;
  

  ElementInfo readElemInfo(std::ifstream& fileDICOM) const;  
  void readMetaHeaderElem(std::ifstream& fileDICOM, const ElementInfo& info);
  void readHeaderElem(std::ifstream& fileDICOM, const ElementInfo& info);

  std::string readSizedElement(std::ifstream& fileDICOM, const uint32_t elemLength) const;
  void skipUnusedElement(std::ifstream& fileDICOM, const uint32_t elemLength) const;

  void parseUndefLengthSequence(std::ifstream& fileDICOM, uint16_t iSeqGroupID, uint16_t iSeqElementID,
                                const bool bImplicit, uint32_t iDepth=1);

  uint32_t getUInt(std::ifstream& fileDICOM, const DICOM_eType eElementType, const uint32_t iElemLength);

  void validatePixelData(std::ifstream& fileDICOM, ElementInfo info);
};
