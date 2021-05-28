#include <sstream>
#include <filesystem>
#include <climits>
#include "DICOMFile.h"


template <typename T>
T swap_endian(T u) {
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;
    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
}

inline bool machineIsBigEndian() {
  union {
    int i;
    char c[sizeof(int)];
  } tmp;
  tmp.i=0x1020;
  return tmp.c[0]!=0x20;
}


static std::vector<std::string> tokenize(const std::string& strInput, char delim) {
  std::vector<std::string> elements;
  size_t iStart = 0;
  size_t i = 0;
  for (;i<strInput.size();i++) {
    if (strInput[i] == delim) {
      if (i-iStart > 0)
        elements.push_back(strInput.substr(iStart, i-iStart));
      iStart = i+1;
    }
  }
  if (i-iStart > 0)
    elements.push_back(strInput.substr(iStart, i-iStart));
  return elements;
}


DICOMFile::DICOMFile(const std::string& filename) {
  metaHeader.filename = filename;
  
  DICOM_DBG("Processing file " << filename);
  
  // Check if file has minimal required size
  try {
    if (std::filesystem::file_size(filename) < 128+4) {
      throw DICOMFileException("Not a DICOM file (too short)");
    }
  } catch(std::filesystem::filesystem_error&) {
    throw DICOMFileException("Error reading file stats");
  }
  DICOM_DBG("File exists and has sufficent length");
  
  // Skip the first 128 bytes (that's how DICOM works)
  std::ifstream fileDICOM(filename, std::ios::in | std::ios::binary);
  fileDICOM.seekg(128);

  // Read the Meta-Header
  ElementInfo elemInfo;
  char buffer[4];
  fileDICOM.read(buffer,4);
  metaHeader.needsEndianConversion = machineIsBigEndian();
  if (buffer[0] != 'D' || buffer[1] != 'I' || buffer[2] != 'C' || buffer[3] != 'M') {
    DICOM_DBG("DICOM magic NOT found, this may still be a DICOM file, guessing parameters.");
    fileDICOM.seekg(0);
    metaHeader.implicitFileType = true;
    elemInfo = readElemInfo(fileDICOM);
    if (elemInfo.groupID != 0x08) {
      throw DICOMFileException("Not a DICOM file");
    }
  } else {
    DICOM_DBG("DICOM magic found, reading parameters.");
    elemInfo = readElemInfo(fileDICOM);
    while (!metaHeader.complete && elemInfo.groupID == 0x2 && !fileDICOM.eof()) {
      readMetaHeaderElem(fileDICOM, elemInfo);
      elemInfo = readElemInfo(fileDICOM);
    }
  }
  
  // Read Actual-Header
  while (elemInfo.groupID != 0x7fe0 && elemInfo.elementType != DICOM_eType::TYPE_UN) {
    readHeaderElem(fileDICOM, elemInfo);
    elemInfo = readElemInfo(fileDICOM);
  }
  
  m_iOffset = uint32_t(fileDICOM.tellg());
  validatePixelData(fileDICOM, elemInfo);

  
  fileDICOM.close();
}

void DICOMFile::validatePixelData(std::ifstream& fileDICOM, ElementInfo info) {
  if (info.elementType != DICOM_eType::TYPE_UN) {
    if (!metaHeader.implicitFileType) {
      
      // for an explicit file we can actually check if we found the pixel
      // data block (and not some color table)
      const uint32_t iPixelDataSize = m_ivSize.x*m_ivSize.y*m_ivSize.z*m_iAllocated / 8;
      uint32_t iDataSizeInFile = info.elementLength;
      if (iDataSizeInFile == 0) fileDICOM.read((char*)&iDataSizeInFile,4);

      if (metaHeader.isJPEGEncoded) {
        unsigned char iJPEGID[2];
        while (!fileDICOM.eof()) {
          fileDICOM.read((char*)iJPEGID,2);
          if (iJPEGID[0] == 0xFF && iJPEGID[1] == 0xE0 ) break;
        }
        // Try to get the offset, which can fail.  If it does, report an error
        // and fake an offset -- we're screwed at that point anyway.
        size_t offset = static_cast<size_t>(fileDICOM.tellg());
        if(static_cast<int>(fileDICOM.tellg()) == -1) {
          throw DICOMFileException("JPEG offset unknown; DICOM parsing failed.");
        }
        offset -= 4;
        
        DICOM_DBG("JPEG data is at offset " << m_iOffset);
      } else {
        if (iPixelDataSize != iDataSizeInFile) {
          info.elementType = DICOM_eType::TYPE_UN;
        } else {
          m_iOffset = uint32_t(fileDICOM.tellg());
        }
      }
    } else {
      m_iOffset = uint32_t(fileDICOM.tellg());
    }
  }
  
  // don't put this into an else as we may change elementType in the if above
  if (info.elementType == DICOM_eType::TYPE_UN) {
    // ok we encoutered some strange DICOM file (most likely that additional
    // SIEMENS header) and found an unknown tag,
    // so lets just march througth the rest of the file and search the magic
    // 0x7fe0, then use the last one found
    DICOM_DBG("Manual search for GroupId 0x7fe0\n");
    std::streampos iPosition   = fileDICOM.tellg();
    fileDICOM.seekg(0,std::ios::end);
    size_t iFileLength = size_t(fileDICOM.tellg());
    fileDICOM.seekg(iPosition,std::ios::beg);

    DICOM_DBG("  Volume/Slice Size" << m_ivSize.x * m_ivSize.y * m_ivSize.z * m_iComponentCount);
    uint32_t iPixelDataSize = m_iComponentCount *
                              m_ivSize.x * m_ivSize.y * m_ivSize.z *
                              m_iAllocated / 8;
    bool bOK = false;
    do {
      info.groupID = 0;
      iPosition = fileDICOM.tellg();

      while (!fileDICOM.eof() && info.groupID != 0x7fe0 &&
             uint32_t(iPosition)+iPixelDataSize < iFileLength) {
        iPosition+=1;
        fileDICOM.read((char*)&info.groupID,2);
      }

      // check if this 0x7fe0 is really a group ID
      if (info.groupID == 0x7fe0) {
        fileDICOM.seekg(-2, std::ios_base::cur);
        readHeaderElem(fileDICOM, info);
        bOK = (info.elementType == DICOM_eType::TYPE_OW ||
               info.elementType == DICOM_eType::TYPE_OB ||
               info.elementType == DICOM_eType::TYPE_OF);

        if (bOK) {
          DICOM_DBG("Manual search for GroupID seemed to work.");
          if (!metaHeader.implicitFileType) {
            uint32_t iVolumeDataSize = m_ivSize.x * m_ivSize.y * m_ivSize.z * m_iAllocated / 8;
            uint32_t iDataSizeInFile;
            fileDICOM.read((char*)&iDataSizeInFile,4);
            if (iVolumeDataSize != iDataSizeInFile) bOK = false;
          }

          m_iOffset = uint32_t(fileDICOM.tellg());
        } else {
          DICOM_DBG("Manual search for this iteration failed,"
                    "skipping element of type " << int(info.elementType));
          fileDICOM.seekg(info.elementLength, std::ios_base::cur);
        }
      }
    } while (info.groupID == 0x7fe0);

    if (!bOK) {
      if (iFileLength < iPixelDataSize) {
        throw DICOMFileException("Invalid DICOM file. File is too small for data");
      }
      // ok everthing failed than let's just use the data we have so far,
      // and let's hope that the file ends with the data
      DICOM_DBG("WARNING: Manual search failed assuming pixel data is stored at the end of the file.");
      m_iOffset = uint32_t(iFileLength - size_t(iPixelDataSize));
    }
  }
    
  m_iRawDataSize = info.elementLength;
  DICOM_DBG("Reporting " << m_iRawDataSize << " bytes of pixel data at position " << m_iOffset << " in file");

  if (!metaHeader.isJPEGEncoded) {
    uint32_t expectedSize = m_iComponentCount* m_ivSize.x * m_ivSize.y * m_ivSize.z*m_iAllocated/8;
    if (expectedSize == m_iRawDataSize) {
      DICOM_DBG("Reported size of data matches volume/slice size.");
    }
  }
  
}


ElementInfo DICOMFile::readElemInfo(std::ifstream& fileDICOM) const {
  
  bool implicitElementType = metaHeader.implicitFileType;
  bool elementNeedsEndianConversion = metaHeader.needsEndianConversion;
  
  ElementInfo info;
  std::string typeString = "  ";

  fileDICOM.read((char*)&info.groupID,2);
  fileDICOM.read((char*)&info.elementID,2);

  if (info.groupID == 0x2) {  // ignore global file parameters for meta block
    implicitElementType = false;
    elementNeedsEndianConversion = machineIsBigEndian();
  }
  
  if (elementNeedsEndianConversion) {
    info.groupID = swap_endian<uint16_t>(info.groupID);
    info.elementID = swap_endian<uint16_t>(info.elementID);
  }

  if (implicitElementType) {
    info.elementType = DICOM_eType::TYPE_Implicit;
    fileDICOM.read((char*)&info.elementLength,4);
    if (elementNeedsEndianConversion) info.elementLength = swap_endian<uint32_t>(info.elementLength);
    
    if (info.elementLength == 0xFFFFFFFF) {
      DICOM_DBG("Reader read implict field groupID=" << info.groupID << ", elementID=" << info.elementID << ", elemLength=undefined");
    } else {
      DICOM_DBG("Reader read implict field groupID=" << info.groupID << ", elementID=" << info.elementID << ", elemLength=" << info.elementLength);
    }
  } else {
    fileDICOM.read(&typeString[0],2);
    uint16_t tmp;
    fileDICOM.read((char*)&tmp,2);
    if (elementNeedsEndianConversion) tmp = swap_endian<uint16_t>(tmp);
    info.elementLength = tmp;
    info.elementType = DICOM_eType::TYPE_UN;
    uint32_t i=0;
    for (;i<27;i++) {
      if (typeString == DICOM_TypeStrings[i]) {
        info.elementType = DICOM_eType(i);
        break;
      }
    }
    if (i==27) {
      DICOM_DBG("WARNING: Reader could not interpret type ->" << typeString[0] << typeString[1] <<
                "<- (groupID=" << info.groupID << ", elementID=" << info.elementID << ", elemLength=" << info.elementLength << ")");
    } else {
      DICOM_DBG("Reader read explicit field " << typeString[0] << typeString[1] <<
                " (groupID=" << info.groupID << ", elementID=" << info.elementID << ", elemLength=" << info.elementLength << ")");
    }
  }

  if ((info.elementType == DICOM_eType::TYPE_OF || info.elementType == DICOM_eType::TYPE_OW ||
       info.elementType == DICOM_eType::TYPE_OB || info.elementType == DICOM_eType::TYPE_UT) && info.elementLength == 0) {
    fileDICOM.read((char*)&info.elementLength,4);
    if (metaHeader.needsEndianConversion) info.elementLength = swap_endian<uint32_t>(info.elementLength);
    DICOM_DBG("Reader found zero length of type " << typeString[0] << typeString[1] <<
              " and read the length again which is now " << info.elementLength << " (groupID=" << info.groupID << ", elementID=" << info.elementID << ")");
  }
  
  return info;
}

void DICOMFile::readMetaHeaderElem(std::ifstream& fileDICOM, const ElementInfo& info) {
  std::string value;
  
  switch (info.elementID) {
    case 0x0 : {  // File Meta Elements Group Len
          if (info.elementLength != 4) {
            std::stringstream ss;
            ss << "Invalid meta group elemLength " << info.elementLength;
            throw DICOMFileException(ss.str());
          }
          uint32_t metaHeaderLength;
          fileDICOM.read((char*)&metaHeaderLength,4);
          metaHeader.offsetToEnd = metaHeaderLength + uint32_t(fileDICOM.tellg());
          DICOM_DBG("DICOM metaHeader end found at " << metaHeader.offsetToEnd);
         } break;
    case 0x1 : {  // Version
          value = readSizedElement(fileDICOM, std::max<uint32_t>(info.elementLength,1));
          DICOM_DBG("DICOM file Version ->" << value << "<- (this string is often empty)");
         } break;
    case 0x10 : {  // Parse Type to find out endianess
          value = readSizedElement(fileDICOM, info.elementLength);

          if (value == "1.2.840.10008.1.2") {
            metaHeader.implicitFileType = true;
            metaHeader.needsEndianConversion = machineIsBigEndian();
            metaHeader.isBigEndian = false;
            DICOM_DBG("DICOM file is implicit value representatio Little Endian");
          } else if (value == "1.2.840.10008.1.2.1") {
            metaHeader.implicitFileType = false;
            metaHeader.needsEndianConversion = machineIsBigEndian();
            metaHeader.isBigEndian = false;
            DICOM_DBG("DICOM file is explicit value representation Little Endian");
          } else if (value == "1.2.840.10008.1.2.2") {
            metaHeader.implicitFileType = false;
            metaHeader.needsEndianConversion = !machineIsBigEndian();
            metaHeader.isBigEndian = true;
            DICOM_DBG("DICOM file is explicit value representatio Big Endian");
          } else if (value == "1.2.840.10008.1.2.4.50" ||   // JPEG Baseline            ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.51" ||   // JPEG Extended            ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.55" ||   // JPEG Progressive         ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.57" ||   // JPEG Lossless            ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.58" ||   // JPEG Lossless            ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.70" ||   // JPEG Lossless            ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.80" ||   // JPEG-LS Lossless         ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.81" ||   // JPEG-LS Near-lossless    ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.90" ||   // JPEG 2000 Lossless       ( untested due to lack of example DICOMs)
                     value == "1.2.840.10008.1.2.4.91" ) {  // JPEG 2000                ( untested due to lack of example DICOMs)
            metaHeader.isJPEGEncoded = true;
            metaHeader.implicitFileType = false;
            metaHeader.needsEndianConversion = machineIsBigEndian();
            metaHeader.isBigEndian = false;
            DICOM_DBG("DICOM file is JPEG explicit value representatio Big Endian");
          } else {
            std::stringstream ss;
            ss << "Unknown/Unsupported DICOM type " << value;
            throw DICOMFileException(ss.str());
          }
          metaHeader.complete = true;
          fileDICOM.seekg(metaHeader.offsetToEnd, std::ios_base::beg);
         } break;
    default : {
      skipUnusedElement(fileDICOM, info.elementLength);
    } break;
  }
}

std::string DICOMFile::readSizedElement(std::ifstream& fileDICOM, const uint32_t elemLength) const {
  char* strBuffer = new char[elemLength+1];
  std::fill_n(strBuffer,elemLength+1,0);
  if (elemLength) {
    fileDICOM.read(strBuffer,elemLength);
  }
  std::string result(strBuffer);
  delete [] strBuffer;
  return result;
}

void DICOMFile::skipUnusedElement(std::ifstream& fileDICOM, const uint32_t elemLength) const {
  readSizedElement(fileDICOM, elemLength);
}

void DICOMFile::readHeaderElem(std::ifstream& fileDICOM, const ElementInfo& info) {
  
  std::string value{"skiped unused element"};
  
  if (info.elementType == DICOM_eType::TYPE_SQ) { // read explicit sequence
    uint32_t iElemLength{0};
    fileDICOM.read((char*)&iElemLength,4);
    if (iElemLength == 0xFFFFFFFF) {
      parseUndefLengthSequence(fileDICOM, info.groupID, info.elementID, false);
      value = "SEQUENCE";
    } else {
      // HACK: here we simply skip over the entire sequence
      skipUnusedElement(fileDICOM, iElemLength);
      value = "SKIPPED EXPLICIT SEQUENCE";
    }
  } else if (info.elementType == DICOM_eType::TYPE_Implicit && info.elementLength == 0xFFFFFFFF) { // read implicit sequence
    parseUndefLengthSequence(fileDICOM, info.groupID, info.elementID, true);
    value = "SEQUENCE";
  } else {
    switch (info.groupID) {
      case 0x8 :
        switch (info.elementID) {
            case 0x22 : { // Acquisition Date
                  m_strAcquDate.resize(info.elementLength);
                  fileDICOM.read(&m_strAcquDate[0],info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_strAcquDate << " (Acquisition Date: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x32 : { // Acquisition Time
                  m_strAcquTime.resize(info.elementLength);
                  fileDICOM.read(&m_strAcquTime[0],info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_strAcquTime << " (Acquisition Time: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x60 : { // Modality
                  m_strModality.resize(info.elementLength);
                  fileDICOM.read(&m_strModality[0],info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_strModality << " (Modality: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x1030 : { // Study Description
                  m_strDesc.resize(info.elementLength);
                  fileDICOM.read(&m_strDesc[0],info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_strDesc << " (Study Description: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            default : {
              #ifdef DEBUG_DICOM
                value = readSizedElement(fileDICOM, info.elementLength);
              #else
                skipUnusedElement(fileDICOM, info.elementLength);
              #endif
            } break;
           } break;
        
      case 0x18 : switch (info.elementID) {
            case 0x50 : { // Slice Thickness
                  value = readSizedElement(fileDICOM, info.elementLength);
                  m_fvfAspect.z = float(atof(value.c_str()));
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_fvfAspect.z << " (Slice Thinkness: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x88 : { // Spacing
                  value = readSizedElement(fileDICOM, info.elementLength);
                  m_fSliceSpacing = float(atof(value.c_str()));
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_fSliceSpacing << " (Slice Spacing: recognized)";
                    value = ss.str();
                  }
                  #endif
                  } break;
             default : {
              #ifdef DEBUG_DICOM
                value = readSizedElement(fileDICOM, info.elementLength);
              #else
                skipUnusedElement(fileDICOM, info.elementLength);
              #endif
            } break;
          }  break;
      case 0x20 : switch (info.elementID) {
            case 0x11 : { // Series Number
                  m_iSeries = getUInt(fileDICOM, info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_iSeries << " (Series Number: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x13 : { // Image Number
                  m_iImageIndex = getUInt(fileDICOM, info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_iImageIndex << " (Image Number: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  } break;
            case 0x32 : // patient position
              {
                value = readSizedElement(fileDICOM, info.elementLength);
              
                std::vector<std::string> values = tokenize(value, '\\');
                if (values.size() != 3) break;
                
                m_fvPatientPosition.x = std::stof(values[0]);
                m_fvPatientPosition.y = std::stof(values[1]);
                m_fvPatientPosition.z = std::stof(values[2]);

                #ifdef DEBUG_DICOM
                {
                  std::stringstream ss;
                  ss << m_fvPatientPosition.x << ", " <<
                        m_fvPatientPosition.y << ", " <<
                        m_fvPatientPosition.z <<
                        " (x,y,z Patient Position: recognized and stored)";
                  value = ss.str();
                }
                #endif
              }  break;
          case 0x37 : // patient orientation
            {
              value = readSizedElement(fileDICOM, info.elementLength);
              std::vector<std::string> values = tokenize(value, '\\');
              if (values.size() != 6) break;
              
              m_fvPatientOrientation1.x = std::stof(values[0]);
              m_fvPatientOrientation1.y = std::stof(values[1]);
              m_fvPatientOrientation1.z = std::stof(values[2]);
              m_fvPatientOrientation2.x = std::stof(values[3]);
              m_fvPatientOrientation2.y = std::stof(values[4]);
              m_fvPatientOrientation2.z = std::stof(values[5]);
              #ifdef DEBUG_DICOM
              {
                std::stringstream ss;
                ss << m_fvPatientOrientation1.x << ", " <<
                      m_fvPatientOrientation1.y << ", " <<
                      m_fvPatientOrientation1.z << ", " <<
                      m_fvPatientOrientation2.x << ", " <<
                      m_fvPatientOrientation2.y << ", " <<
                      m_fvPatientOrientation2.z <<
                      " (Patient orientation: recognized and stored)";
                value = ss.str();
              }
              #endif
            }  break;
            case 0x1041: // Slice Location
              value = readSizedElement(fileDICOM, info.elementLength);
              m_fSliceLocation = float(atof(value.c_str()));
              #ifdef DEBUG_DICOM
                {
                  std::stringstream ss;
                  ss << m_fSliceLocation << " (Slice Location: recognized and stored)";
                  value = ss.str();
                }
              #endif
              break;
            default : {
              #ifdef DEBUG_DICOM
                value = readSizedElement(fileDICOM, info.elementLength);
              #else
                skipUnusedElement(fileDICOM, info.elementLength);
              #endif
            } break;
           } break;
      case 0x28 : switch (info.elementID) {
            case  0x2 : // component count
                  m_iComponentCount = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_iComponentCount << " (samples per pixel: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            case  0x8 : // Slices
                  m_ivSize.z = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_ivSize.z << " (Slices: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            case 0x10 : // Rows
                  m_ivSize.y = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_ivSize.y << " (Rows: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                    break;
            case 0x11 : // Columns
                  m_ivSize.x = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_ivSize.x << " (Columns: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                    break;
            case 0x30 : // x,y spacing
              {
                value = readSizedElement(fileDICOM, info.elementLength);

                std::vector<std::string> values = tokenize(value, '\\');
                if (values.size() != 2) break;
                m_fvfAspect.x = std::stof(values[0]);
                m_fvfAspect.y = std::stof(values[1]);
                #ifdef DEBUG_DICOM
                {
                  std::stringstream ss;
                  ss << m_fvfAspect.x << ", " << m_fvfAspect.y << " (x,y spacing: recognized and stored)";
                  value = ss.str();
                }
                #endif
              }  break;
            case 0x100 : // Allocated
                  m_iAllocated = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_iAllocated << " (Allocated bits: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                    break;
            case 0x101 : // Stored
                  m_iStored = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength);
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_iStored << " (Stored bits: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            case 0x0103 : // sign
                  m_bSigned = getUInt(fileDICOM, (info.elementType == DICOM_eType::TYPE_Implicit) ? DICOM_eType::TYPE_US : info.elementType, info.elementLength) == 1;
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_bSigned << " (Sign bit: recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            case 0x1050: // Window Center
              value = readSizedElement(fileDICOM, info.elementLength);
              m_fWindowCenter = float(atof(value.c_str()));
              #ifdef DEBUG_DICOM
                {
                  std::stringstream ss;
                  ss << m_fWindowCenter << " (Window Center: recognized and stored)";
                  value = ss.str();
                }
              #endif
              break;
            case 0x1051: // Window Width
              value = readSizedElement(fileDICOM, info.elementLength);
              m_fWindowWidth =-float(atof(value.c_str()));
              #ifdef DEBUG_DICOM
                {
                  std::stringstream ss;
                  ss << m_fWindowWidth << " (Window Width: recognized and stored)";
                  value = ss.str();
                }
              #endif
              break;
            case 0x1052 : // Rescale Intercept (Bias)
                  value = readSizedElement(fileDICOM, info.elementLength);
                  m_fBias = float(atof(value.c_str()));
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_fBias << " (Rescale Intercept (Bias): recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            case 0x1053 : // Rescale Slope (Scale)
                  value = readSizedElement(fileDICOM, info.elementLength);
                  m_fScale = float(atof(value.c_str()));
                  #ifdef DEBUG_DICOM
                  {
                    std::stringstream ss;
                    ss << m_fScale << " (Rescale Slope (Scale): recognized and stored)";
                    value = ss.str();
                  }
                  #endif
                  break;
            default : {
              #ifdef DEBUG_DICOM
                value = readSizedElement(fileDICOM, info.elementLength);
              #else
                skipUnusedElement(fileDICOM, info.elementLength);
              #endif
            } break;
           } break;
      default : {
        #ifdef DEBUG_DICOM
          value = readSizedElement(fileDICOM, info.elementLength);
        #else
          skipUnusedElement(fileDICOM, info.elementLength);
        #endif
      } break;

    }
  }
  
  #ifdef DEBUG_DICOM
  if (value != "SEQUENCE")
    DICOM_DBG("iGroupID="<< info.groupID <<" iElementID="<< info.elementID <<" elementType="
              << DICOM_TypeStrings[int(info.elementType)] << " value=" << value);
  #endif
}


void DICOMFile::parseUndefLengthSequence(std::ifstream& fileDICOM, uint16_t iSeqGroupID, uint16_t iSeqElementID,
                                         const bool bImplicit, uint32_t iDepth) {


  #ifdef DEBUG_DICOM
    for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
    DICOM_DBG("iGroupID="<< iSeqGroupID <<" iElementID="<< iSeqElementID <<" elementType=SEQUENCE (undef length)");
  #endif

  uint32_t iItemCount{0};
  uint32_t iData;
  std::string value;

  do {
    fileDICOM.read((char*)&iData,4);

    if (iData == 0xE000FFFE) {
      iItemCount++;
      fileDICOM.read((char*)&iData,4);
      #ifdef DEBUG_DICOM
        for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
        DICOM_DBG("START ITEM");
      #endif
    } else if (iData == 0xE00DFFFE) {
      iItemCount--;
      fileDICOM.read((char*)&iData,4);
      #ifdef DEBUG_DICOM
        for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
        DICOM_DBG("START ITEM");
      #endif
    } else if (iData != 0xE0DDFFFE) {
      fileDICOM.seekg(-4, std::ios_base::cur);
    }


    if (iItemCount > 0) {
      ElementInfo subInfo = readElemInfo(fileDICOM);
      iData = subInfo.elementLength;
      
      if (subInfo.elementType == DICOM_eType::TYPE_SQ) {
        fileDICOM.read((char*)&iData,4);
        if (iData == 0xFFFFFFFF) {
          parseUndefLengthSequence(fileDICOM, subInfo.groupID, subInfo.elementID, bImplicit);
        } else {
          // HACK: here we simply skip over the entire sequence
          value.resize(iData);
          fileDICOM.read(&value[0],iData);
          value = "SKIPPED EXPLICIT SEQUENCE";
        }
      } else {
        if (subInfo.elementLength == 0xFFFFFFFF) {
          parseUndefLengthSequence(fileDICOM, subInfo.groupID, subInfo.elementID, bImplicit, iDepth+1);
        } else {
          if(subInfo.elementLength > 0) {
            value.resize(subInfo.elementLength);
            fileDICOM.read(&value[0],subInfo.elementLength);
            #ifdef DEBUG_DICOM
              for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
              DICOM_DBG("iGroupID="<< subInfo.groupID <<" iElementID="<< subInfo.elementID <<" elementType="
                        << DICOM_TypeStrings[int(subInfo.elementType)] << " value=" << value);
            #endif
          } else {
            #ifdef DEBUG_DICOM
              for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
              DICOM_DBG("iGroupID="<< subInfo.groupID <<" iElementID="<< subInfo.elementID <<" elementType="
                        << DICOM_TypeStrings[int(subInfo.elementType)] << " value=empty");
            #endif
          }
        }
      }
    }
  } while (iData != 0xE0DDFFFE && !fileDICOM.eof());
  fileDICOM.read((char*)&iData,4);

#ifdef DEBUG_DICOM
  for (uint32_t i = 0;i<iDepth;i++) DICOM_DBG_NL("  ");
  DICOM_DBG("START SEQUENCE");
#endif

}


uint32_t DICOMFile::getUInt(std::ifstream& fileDICOM, const DICOM_eType eElementType,
                            const uint32_t iElemLength) {
  std::string value;
  uint32_t result;
  switch (eElementType) {
    case DICOM_eType::TYPE_Implicit :
    case DICOM_eType::TYPE_IS  : {
              value = readSizedElement(fileDICOM, iElemLength);
              result = uint32_t(std::stoul(value.c_str()));
              break;
            }
    case DICOM_eType::TYPE_UL  : {
              fileDICOM.read((char*)&result,4);
              if (metaHeader.needsEndianConversion) result = swap_endian<uint32_t>(result);
              break;
            }
    case DICOM_eType::TYPE_US  : {
              uint16_t tmp;
              fileDICOM.read((char*)&tmp,2);
              if (metaHeader.needsEndianConversion) tmp = swap_endian<uint16_t>(tmp);
              result = tmp;
              break;
            }
    default : result = 0; break;
  }
  return result;
}


std::vector<uint8_t> DICOMFile::getData(uint32_t count) const {
  std::ifstream fileDICOM(metaHeader.filename, std::ios::in | std::ios::binary);
  fileDICOM.seekg(m_iOffset);
  std::vector<uint8_t> data(count);
  fileDICOM.read((char*)data.data(), count);
  fileDICOM.close();
  return data;
}

bool DICOMFile::match(const DICOMFile& other) const {
  return
    m_iSeries         == other.m_iSeries &&
    m_ivSize          == other.m_ivSize &&
    m_iAllocated      == other.m_iAllocated &&
    m_iStored         == other.m_iStored &&
    m_iComponentCount == other.m_iComponentCount &&
    m_bSigned         == other.m_bSigned &&
    m_fvfAspect       == other.m_fvfAspect &&
    metaHeader.isBigEndian    == other.metaHeader.isBigEndian &&
    metaHeader.isJPEGEncoded  == other.metaHeader.isJPEGEncoded &&
    m_strAcquDate     == other.m_strAcquDate &&
    m_strModality     == other.m_strModality &&
    m_strDesc         == other.m_strDesc;
}


bool DICOMFile::stackLessCompare(const DICOMFile& other) const {
  return m_iSeries < other.m_iSeries;
}

bool DICOMFile::depthLessCompare(const DICOMFile& other) const {
  return m_fvPatientPosition.z < other.m_fvPatientPosition.z;
}

