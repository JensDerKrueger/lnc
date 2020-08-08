#include "MB1242.h"

using namespace I2C;

#include <sstream>

MB1242::MB1242(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID,
               const std::string& hrname,
               const HAS::HASConfigPtr config,
               std::shared_ptr<I2CBusManager> busManager,
               bool blocking) :
  I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
  m_lastValue(0.0f),
  m_blocking(blocking)
{
}

std::string MB1242::getDesc() const {
  return "MaxSonar-EX High Performance Ultrasonic Range Finder MB1242";
}

void MB1242::init() {
  I2CMember::init();
  delay(100);
  if (!m_blocking) {
    startRangeReading();
    delay(100);
  }
}


void MB1242::setNewAddress(uint8_t newAddress) {
  newAddress = newAddress<<1; // convert 7 bit address to 8 bit address
  activate();
  I2CBase::writeReg16(m_DeviceHandle,0xAA,0xA5+256*newAddress);
}

void MB1242::startRangeReading() {
  activate();
  I2CBase::write(m_DeviceHandle,0x51);
}

float MB1242::getRange() {
  activate();
  uint16_t v = I2CBase::readReg16(m_DeviceHandle,0);
  uint16_t s = (v & 0x00FF)*256 + (v&0xFF00)/256;
  return s/100.0f;
}

uint32_t MB1242::pollAnalogIn() {
  if (m_blocking) {
    startRangeReading();
    return 100;
  } else {
    m_lastValue = getRange();
    startRangeReading();
    return 0;
  }
}

float MB1242::get() {
  return m_lastValue;
}

float MB1242::getAnalog(uint8_t ) {
  return get();
}

std::string MB1242::getAnalogChannelDesc(uint8_t iChannel) const {
  return "Distance to nozzle";
}

std::string MB1242::getAnalogChannelUnit(uint8_t iChannel) const {
  return "m";
}

/*
 The MIT License

 Copyright (c) 2014 Jens Krueger

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
