#include "HKAnalogPCF8591.h"
#include <stdexcept>

using namespace I2C;

HKAnalogPCF8591::HKAnalogPCF8591(uint8_t busID, uint8_t i2cAddress,
                                 const std::string& devID,
                                 const std::string& hrname,
                                 const HAS::HASConfigPtr config,
                                 std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager)
{
  m_bAnalogOutNeedsUpdate = true;
  for (size_t i = 0;i<getAnalogInChannelCount();++i)
    m_inVal[i] = 0;
  m_outVal = 0;
}

std::string HKAnalogPCF8591::getDesc() const {
  return "Horter & Kalb – Analogboard with PCF8591";
}

uint32_t HKAnalogPCF8591::pollAnalogIn() {
  activate();
  I2CBase::write(m_DeviceHandle, 64+4);  // 4=register autoinc mode
  for (size_t i = 0;i<getAnalogInChannelCount();++i)
    m_inVal[i] = I2CBase::read(m_DeviceHandle)/25.5f;
  return 0;
}

float HKAnalogPCF8591::getAnalog(uint8_t iChannel) {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogOutChannelCount()-1))
    throw std::out_of_range("HKAnalogPCF8591::getAnalog: index out of range");

  return m_inVal[iChannel];
}

void HKAnalogPCF8591::setAnalog(uint8_t iChannel, float value) {
  m_outVal = value;
  m_bAnalogOutNeedsUpdate = true;
}

void HKAnalogPCF8591::applyAnalogOut() {
  set(m_outVal);
  m_bAnalogOutNeedsUpdate = false;
}

void HKAnalogPCF8591::set(float fVal) {
  uint8_t val = uint8_t(clamp(fVal,0.0f,1.0f)*255);
  set(val);
}

void HKAnalogPCF8591::set(uint8_t val) {
  activate();
  I2CBase::writeReg8(m_DeviceHandle, 64, val);
}

float HKAnalogPCF8591::get(uint8_t iChannel) {
  activate();
  I2CBase::write(m_DeviceHandle, 64+iChannel);
  return float(I2CBase::read(m_DeviceHandle));
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
