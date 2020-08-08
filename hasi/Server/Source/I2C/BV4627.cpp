#include "BV4627.h"
#include <stdexcept>
#include <sstream>

#include <Tools/DebugOutHandler.h> // for IVDA_WARNING

using namespace I2C;


BV4627::BV4627(uint8_t busID, uint8_t i2cAddress,
                       const std::string& devID, const std::string& hrname,
                       const HAS::HASConfigPtr config,
                       std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager)
{
  m_bDigitalOutNeedsUpdate = true;
}

void BV4627::init() {
  I2CMember::init();
  reset();
  for (size_t i = 0 ; i < 8; ++i) m_inputStates[i] = UNCHANGED;
}

void BV4627::setDigital(uint8_t iChannel, BitVal value) {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalOutChannelCount()-1))
    throw std::out_of_range("BV4627::setDigital: index out of range");
  m_BlockData = bitWrite(m_BlockData, iChannel, value);
  m_bDigitalOutNeedsUpdate = true;
}

void BV4627::reset() {
  activate();
  I2CBase::write(m_DeviceHandle, 85);
}

void BV4627::allOn() {
  activate();
  I2CBase::write(m_DeviceHandle, 19);
}

void BV4627::allOff() {
  activate();
  I2CBase::write(m_DeviceHandle, 18);
}

void BV4627::changeI2CAddress(uint8_t newAddress) {
  activate();
  I2CBase::writeReg8(m_DeviceHandle, 82, newAddress*2);
}

void BV4627::applyDigitalOut() {
  activate();
  I2CBase::writeReg8(m_DeviceHandle, 20, m_BlockData);

  // or
  //   I2CBase::writeReg8(m_DeviceHandle, 20, ~m_BlockData);

  m_bDigitalOutNeedsUpdate = false;
}

bool BV4627::verifyDeviceID() {
  activate();
  return 4627 == I2CBase::readReg16 (m_DeviceHandle, 83);
}

std::string BV4627::getVersion() const {
  int val = I2CBase::readReg16 (m_DeviceHandle, 84);
  
  uint8_t lsb = val & 255;
  uint8_t msb = val >> 8;
  
  std::stringstream ss;
  ss << int(msb) << "." << int(lsb);
  return ss.str();
}

std::string BV4627::getDesc() const {
  std::stringstream ss;
  ss << "ByVac 8-Way Relay BV4627  " << getVersion();
  return ss.str();
}

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
