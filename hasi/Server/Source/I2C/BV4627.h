#pragma once

#ifndef BV4627_H
#define BV4627_H

#include <array>          // std::array

#include "I2CMember.h"
#include "../IDigitalOut.h"

namespace I2C {
  
  class BV4627 : public I2CMember, public IDigitalOut {
  public:
    BV4627(uint8_t busID, uint8_t i2cAddress,
           const std::string& devID,
           const std::string& hrname,
           const HAS::HASConfigPtr config,
           std::shared_ptr<I2CBusManager> busManager);
    virtual void init();
    
    bool verifyDeviceID();
    void changeI2CAddress(uint8_t newAddress);
    void reset();
    void allOn();
    void allOff();
    std::string getVersion() const;

    // digital-out interface
    virtual uint8_t getDigitalOutChannelCount() const {return 8;}
    virtual void setDigital(uint8_t iChannel, BitVal value);
    virtual void applyDigitalOut();
    
  protected:
    std::array<BitState,8> m_inputStates;
    uint8_t m_BlockData;
    
    virtual std::string getDesc() const;
    
  };
  
}

#endif // BV4627_H

/*
 The MIT License
 
 Copyright (c) 2013-2016 Jens Krueger
 
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

