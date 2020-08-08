#include "I2CBus.h"

#include <fstream>      // std::ifstream
#include <string>       // std::string
#include <sstream>      // std::stringstream
#include <algorithm>    // std::sort
#include <stdexcept>    // std::invalid_argument

#ifndef NI2C
  #include <unistd.h>     // open, close
  #include <fcntl.h>      // O_RDWR, O_SYNC
  #include <sys/mman.h>   // mmap
#endif

#include <Script/ParserTools.h>  // removeSpaces, startsWith

#include <Tools/DebugOutHandler.h> // for IVDA_MESSAGE

#include <other-devices/SysInfo.h>

using namespace I2C;
using namespace IVDA;


BusData::~BusData() {
  for (auto it = m_vpAllBlocks.begin(); it != m_vpAllBlocks.end(); ++it) {
    delete it->second;
  }
}

void BusData::init() {
  for (auto it : m_vI2CBusManager) {
    it->init();
  }
  
  for (auto it = m_vpAllBlocks.begin(); it != m_vpAllBlocks.end(); ++it) {
    it->second->init();
  }
}

bool BusData::usesBus(uint8_t id) const {
  for (auto it : m_vI2CBusManager) {
    if (it->getBus() == id) {
      return true;
    }
  }
  
  for (auto it = m_vpAllBlocks.begin(); it != m_vpAllBlocks.end(); ++it) {
    if (it->second->getBus() == id) {
      return true;
    }
  }
  return false;
}


I2CBus::I2CBus() :
m_BusData(nullptr)
{
}

I2CBus::~I2CBus() {
#ifndef NI2C
  IVDA_MESSAGE("Shutting down I2C Bus");
  m_BusData = nullptr;
  IVDA_MESSAGE("I2C Bus is down");
#endif
}

void I2CBus::init() {
#ifndef NI2C
  if (m_BusData) {
    if (m_BusData->usesBus(0) && (HAS::SysInfo::getBoardID() >= HAS::EB_RASPBERRY_UNKNOWN && HAS::SysInfo::getBoardID() <= HAS::EB_RASPBERRY_3AP)) {
      IVDA_MESSAGE("Enabling I2C-0 on Raspberry PI");
      initSecondaryRPiI2CBus();
    }
    m_BusData->init();
  }
  IVDA_MESSAGE("I2C Bus is up");
#else
  IVDA_MESSAGE("I2C Support disabled in this binary");
#endif
}

void I2CBus::ParseDevices(HAS::HASConfigPtr config) {
#ifndef NI2C
  const std::string& filename = config->getDeviceFile();
  
  KeyValueFileParser parser(filename, false, "=");

  if (parser.FileReadable()) {
    std::shared_ptr<BusData> busData(new BusData);

    // parse bus managers first (i.e. twi multiplexer)
    addBusManager<TCA9548ABusManager>(parser, config, busData, "TCA9548ABusManager");
    
    // parse I2C blocks
    addBlocks(parser, config, busData, busData->m_vpInputBlocks, "InputBlock");
    addBlocks(parser, config, busData, busData->m_vpThermoHKLM75s, "Thermo");
    addBlocks(parser, config, busData, busData->m_vpArduPiDACs, "ARDUPI_DAC");
    addBlocks(parser, config, busData, busData->m_vpHKAnalogPCF8591s, "PCF8591_DAC");
    addBlocks(parser, config, busData, busData->m_vpLumi2561s, "Lumi");
    addBlocks(parser, config, busData, busData->m_vpProxi4000s, "Proxi");
    addBlocks(parser, config, busData, busData->m_vpBoschBMPs, "Pressure");
    addBlocks(parser, config, busData, busData->m_vpHIH6130s,"Humidity");
    addBlocks(parser, config, busData, busData->m_vpTMP006s,"TMP006");
    addBlocks(parser, config, busData, busData->m_vpMB1242s, "Distance");
    addBlocks(parser, config, busData, busData->m_vpAI418Ss, "AI418S_ADC");

    addBlocks(parser, config, busData, busData->m_vpBV4627s, "BV4627");
    addBlocks(parser, config, busData, busData->m_vpRelayBlocks, "RelayBlock");
    addBlocks(parser, config, busData, busData->m_vpBlinkMs, "BlinkM");
    addBlocks(parser, config, busData, busData->m_vpMcp4725DACs, "4725_DAC");
    addBlocks(parser, config, busData, busData->m_vpOLEDs, "OLED");

    // iff all ok, copy new bus data into class
    m_BusData = busData;
  } else {
    throw HAS::EDeviceParser(std::string("Unable to parse input file ") + filename);
  }
#endif
}

std::string I2CBus::toString() const {
  std::stringstream ss;
  if (m_BusData) {
    for (auto it = m_BusData->m_vpAllBlocks.cbegin();
         it != m_BusData->m_vpAllBlocks.cend(); ++it) {
      ss << it->second->toString() << std::endl;
    }
    
    ss << "I2C Bus Manager:\n";
    if (!m_BusData->m_vI2CBusManager.empty()) {
      for (auto it : m_BusData->m_vI2CBusManager) {
        ss << it->toString() << std::endl;
      }
    }
    
  }
  return ss.str();
}

I2CMember* I2CBus::getDevice(const std::string& strID) const {
  if (m_BusData) {
    auto iter = m_BusData->m_vpAllBlocks.find(strID);
    if (iter != m_BusData->m_vpAllBlocks.end())
      return m_BusData->m_vpAllBlocks.find(strID)->second;
    else
      throw HAS::EDeviceNotFound(strID);
  } else {
    throw HAS::EDeviceNotFound(strID);
  }
}

template <class T> void I2CBus::addBusManager(KeyValueFileParser& parser,
                                            HAS::HASConfigPtr config,
                                            std::shared_ptr<BusData>& busData,
                                            const std::string& strID) {
  std::vector<const KeyValPair*> b = parser.GetDataVec(strID);
  for (auto inputBlock = b.begin();inputBlock!=b.end();++inputBlock) {
    const std::vector<std::string>& entries = (*inputBlock)->vstrValue;
    
    if (entries.size() != 4) {
      std::stringstream ss;
      ss << "Invalid I2C bus manager entry ID=" << strID << " '" << (*inputBlock)->strValue
      << "' invalid value count. Expected 4 but "
      << "found " << entries.size() << ".";
      throw HAS::EDeviceParser(ss.str());
    }
    
    
    std::shared_ptr<I2CBusManager> bm = std::make_shared<T>(IVDA::SysTools::FromString<int>(entries[0]),
                                                            IVDA::SysTools::FromString<int>(entries[1]),
                                                            entries[2], entries[3], config);
    busData->m_vI2CBusManager.push_back(bm);
  }
}


template <class T> size_t I2CBus::addBlocks(KeyValueFileParser& parser,
                                            HAS::HASConfigPtr config,
                                            std::shared_ptr<BusData>& busData,
                                            std::vector<T*>& vec,
                                            const std::string& strID) {
  std::vector<const KeyValPair*> b = parser.GetDataVec(strID);
  size_t iNewDev = 0;
  for (auto inputBlock = b.begin();inputBlock!=b.end();++inputBlock) {
    const std::vector<std::string>& entries = (*inputBlock)->vstrValue;

    std::shared_ptr<I2CBusManager> pBusManager = nullptr;
    if (entries.size() == 4) {
      
      // nothing to do in this case, busManager is already a nullptr
      
    } else {
      if (entries.size() == 5) {
        
        // find bus manager
        for (std::shared_ptr<I2CBusManager> bm : busData->m_vI2CBusManager) {
          if (bm && bm->getID() == entries[4]) {
            pBusManager = bm;
            break;
          }
        }
        
        if (!pBusManager) {
          std::stringstream ss;
          ss << "Invalid I2C block entry ID=" << strID << " '" << (*inputBlock)->strValue
          << "' specified bus manager not found.";
          throw HAS::EDeviceParser(ss.str());
        }
        
      } else {
        std::stringstream ss;
        ss << "Invalid I2C block entry ID=" << strID << " '" << (*inputBlock)->strValue
        << "' invalid value count. Expected 4 or 5 (with bus manager) but "
        << "found " << entries.size() << ".";
        throw HAS::EDeviceParser(ss.str());
      }
    }
  
    
    T* inBlock = new T(IVDA::SysTools::FromString<int>(entries[0]),
                       IVDA::SysTools::FromString<int>(entries[1]),
                       entries[2], entries[3], config, pBusManager);
    vec.push_back(inBlock);
    iNewDev++;
    busData->m_vpAllBlocks.insert(make_pair(inBlock->getID(),inBlock));
  }
  return iNewDev;
}

void I2CBus::initSecondaryRPiI2CBus() const {
#ifndef NI2C
  const off_t BCM2708_PERI_BASE = 0x20000000;
  const off_t GPIO_BASE = BCM2708_PERI_BASE + 0x00200000;
  const size_t BLOCK_SIZE = 4096;
  int fileHandle = 0;

  if ((fileHandle = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    throw HAS::EAccessDenied("Failed to open /dev/mem, try checking permissions.");
  }

  void* map = mmap(
    0,
    BLOCK_SIZE,
    PROT_READ|PROT_WRITE,
    MAP_SHARED,
    fileHandle,
    GPIO_BASE
  );

  if (map == MAP_FAILED) {
    throw HAS::EAccessDenied("Failed to mem-map /dev/mem.");
  }

  volatile unsigned int *addr = (volatile unsigned int *)map;

  int reg0 = addr[0];
  int m0 = 0b00000000000000000000111111111111;
  int s0 = 0b00000000000000000000100100000000;
  int b0 = reg0 & m0;
  if (b0 != s0) {
     //  need to change register 0
     addr[0] = (reg0 & ~m0) | s0;
   } else {
     // register 0 is ok
   }

   int reg2 = addr[2];
   int m2 = 0b00111111000000000000000000000000;
   int s2 = 0b00100100000000000000000000000000;
   int b2 = reg2 & m2;
   if (b2 != s2) {
     // need to change register 2
     addr[2] = (reg2 & ~m2) | s2;
   } else {
     // register 2 is ok
   }

   munmap(map, BLOCK_SIZE);
   close(fileHandle);
#endif
}

/*
 The MIT License
 
 Copyright (c) 2013 Jens Krueger
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

