#pragma once

#ifndef I2CMEMBER_H
#define I2CMEMBER_H


#include "HASMember.h"
#include "HASConfig.h"

#include "I2CBase.h"


#include <iomanip>   // std::setfill, std::setw, std::hex
#include <string>    // std::string
#include <sstream>   // std::stringstream

namespace I2C {
  
  class I2CBusManager;
  
  class I2CMember : public HAS::HASMember {
  public:
    I2CMember(uint8_t busID, uint8_t i2cAddress,
              const std::string& devID, const std::string& hrname,
              const HAS::HASConfigPtr config,
              std::shared_ptr<I2CBusManager> busManager);
    virtual ~I2CMember();
    virtual void init();
    std::string toString() const;
    
    static bool cmpPtr(const I2CMember* l, const I2CMember* r) {
      return *l < *r;
    }
    
    bool operator<(const I2CMember& other) const {
      return m_i2cBusID < other.m_i2cBusID ||
      (m_i2cBusID == other.m_i2cBusID && m_i2cAddress < other.m_i2cAddress);
    }
    
    uint8_t getBus() const;
    
  protected:
    uint8_t m_i2cBusID;
    uint8_t m_i2cAddress;
    int m_DeviceHandle;
    std::shared_ptr<I2CBusManager> m_busManager;
    
    
    template< typename T > static
    std::string int_to_hex( T i )
    {
      std::stringstream stream;
      
      // handle char / unsigned char seperately (i.e. cast them
      // to int to make sure they don't print as ASCII)
      if (sizeof(T) == 1)
        stream << "0x"
        << std::setfill ('0') << std::setw(sizeof(T)*2)
        << std::hex << (int)i;
      else
        stream << "0x"
        << std::setfill ('0') << std::setw(sizeof(T)*2)
        << std::hex << i;
      
      return stream.str();
    }
    
    void activate();
    
    virtual std::string getDesc() const;
  };
  
}

#endif // I2CMEMBER_H

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

