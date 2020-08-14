#include "ArduPiDAC.h"
#include <stdexcept>
#include <sstream>

using namespace I2C;

uint8_t ArduPiDAC::addresses[8] = {0xDC, 0x9C, 0xCC, 0x8C, 0xAC, 0xEC, 0xBC, 0xFC};

ArduPiDAC::ArduPiDAC(uint8_t busID, uint8_t i2cAddress,
                     const std::string& devID, const std::string& hrname,
                     const HAS::HASConfigPtr config,
                     std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
  for (size_t i = 0;i<getAnalogInChannelCount();++i)
    m_val[i] = 0;
}

uint32_t ArduPiDAC::pollAnalogIn() {
  activate();
  for (uint8_t i = 0;i<getAnalogInChannelCount();++i)
    m_val[i] = get(i);
  return 0;
}

float ArduPiDAC::getAnalog(uint8_t iChannel) {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogInChannelCount()-1)) {
    std::stringstream ss;
    ss << "ArduPiDAC::getAnalog: channel index " << int(iChannel)
    << " out of range [0-" << int(getAnalogInChannelCount()-1) << "]";
    throw std::out_of_range( ss.str() );
  }

  return m_val[iChannel];
}

float ArduPiDAC::get(uint8_t iChannel) {
  uint8_t address = addresses[iChannel];
  I2CBase::write(m_DeviceHandle, address);

  uint8_t msb = I2CBase::read(m_DeviceHandle);
  uint8_t lsb = I2CBase::read(m_DeviceHandle);
  int val = int(msb)*16 + int(lsb>>4);

  return val * 5.0f / 4095.0f;
}

std::string ArduPiDAC::getDesc() const {
  return "Raspberry Pi to Arduino Connection Bridge build-in 12-bit DAC";
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
