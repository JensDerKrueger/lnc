#include "Mcp4725DAC.h"

using namespace I2C;

Mcp4725DAC::Mcp4725DAC(uint8_t busID, uint8_t i2cAddress,
                       const std::string& devID, const std::string& hrname,
                       const HAS::HASConfigPtr config,
                       std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
}

void Mcp4725DAC::set(float fVal) {
  int val = int(std::max(std::min(fVal,1.0f),0.0f)*4095);
  set(val);
}

void Mcp4725DAC::set(int val) {
  uint8_t lsb = uint8_t(val & 0xFF);
  uint8_t msb = uint8_t((val >> 8) & 0xFF);
  
  activate();
  I2CBase::writeReg8(m_DeviceHandle, msb, lsb);
}

std::string Mcp4725DAC::getDesc() const {
  return "Adafruit MCP4725 Breakout Board – 12-Bit DAC";
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