#include "ThermoHKLM75.h"

using namespace I2C;

ThermoHKLM75::ThermoHKLM75(uint8_t busID, uint8_t i2cAddress,
                           const std::string& devID,
                           const std::string& hrname,
                           const HAS::HASConfigPtr config,
                           std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
}

float ThermoHKLM75::get() {
  activate();
  
  int rawData = I2CBase::readReg16(m_DeviceHandle,0x00);
  if (rawData & (1<<7)) {
    rawData ^= 0xFFFE;
    return - (float(rawData & 0xFF) + (rawData >> 15)/2.0f);
  } else {
    return float(rawData & 0xFF) + (rawData >> 15)/2.0f;
  }
}

std::string ThermoHKLM75::getDesc() const {
  return "Horter & Kalb LM75 digital thermometer";
}

float ThermoHKLM75::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 : return float(get());
    default : throw std::out_of_range("ThermoHKLM75::getAnalog: index out of range");
  }
}

std::string ThermoHKLM75::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Temperature";
    default : throw std::out_of_range("ThermoHKLM75::getAnalogChannelDesc: index out of range");
  }
}

std::string ThermoHKLM75::getAnalogChannelUnit(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "°C";
    default : throw std::out_of_range("ThermoHKLM75::getAnalogChannelUnit: index out of range");
  }
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
