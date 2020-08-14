#include "ProxiVCNL4000.h"

using namespace I2C;

// commands and constants

enum {
  VCNL4000_COMMAND = 0x80,
  VCNL4000_PRODUCTID = 0x81,
  VCNL4000_IRLED = 0x83,
  VCNL4000_AMBIENTPARAMETER = 0x84,
  VCNL4000_AMBIENTDATA = 0x85,
  VCNL4000_PROXIMITYDATA = 0x87,
  VCNL4000_SIGNALFREQ = 0x89,
  VCNL4000_PROXINITYADJUST = 0x8A
} /* VCNL4000Register */;

enum {
  VCNL4000_3M125 = 0,
  VCNL4000_1M5625 = 1,
  VCNL4000_781K25 = 2,
  VCNL4000_390K625 = 3
} /* VCNL4000SamplingFequency */;

enum {
  VCNL4000_MEASUREAMBIENT = 0x10,
  VCNL4000_MEASUREPROXIMITY = 0x08,
  VCNL4000_AMBIENTREADY = 0x40,
  VCNL4000_PROXIMITYREADY = 0x20
} /* VCNL4000Parameter */;
 
ProxiVCNL4000::ProxiVCNL4000(uint8_t busID, uint8_t i2cAddress,
                             const std::string& devID,
                             const std::string& hrname,
                             const HAS::HASConfigPtr config,
                             std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
}

std::string ProxiVCNL4000::getDesc() const {
  return "Adafruit VCNL4000 Proximity & Light sensor";
}

void ProxiVCNL4000::init() {
  I2CMember::init();
  
  activate();

  int rev = I2CBase::readReg8(m_DeviceHandle, VCNL4000_PRODUCTID);
  if ((rev & 0xF0) != 0x10) {
    throw EI2CDeviceInit("Unable to init Proximity VCNL4000 sensor,"
                         " wrong i2c address?");
  }

  // IR current -> set to 20 * 10mA = 200mA
  I2CBase::writeReg8(m_DeviceHandle, VCNL4000_IRLED, 20);
  // ambient light measures - single conversion mode, 128 averages
  I2CBase::writeReg8(m_DeviceHandle, VCNL4000_AMBIENTPARAMETER, 0x0F);
  // set proximity adjustment to 0x81 (recommended by manufacturer)
  I2CBase::writeReg8(m_DeviceHandle, VCNL4000_PROXINITYADJUST, 0x81);
}


uint16_t ProxiVCNL4000::get(int iTimeout) {
  activate();
  
  I2CBase::writeReg8(m_DeviceHandle, VCNL4000_COMMAND, VCNL4000_MEASUREPROXIMITY);
  for (int i =0;i<iTimeout;++i) {
    uint8_t result = I2CBase::readReg8(m_DeviceHandle, VCNL4000_COMMAND);
    if (result & VCNL4000_PROXIMITYREADY) {
      return I2CBase::readReg16(m_DeviceHandle, VCNL4000_PROXIMITYDATA);
    }
    delay(1);
  }
  // timeout
  return 0;
}

uint16_t ProxiVCNL4000::getAmbient(int iTimeout) {
  activate();
  
  I2CBase::writeReg8(m_DeviceHandle, VCNL4000_COMMAND, VCNL4000_MEASUREAMBIENT);
  for (int i =0;i<iTimeout;++i) {
    uint8_t result = I2CBase::readReg8(m_DeviceHandle, VCNL4000_COMMAND);
    if (result & VCNL4000_AMBIENTREADY) {
      return I2CBase::readReg16(m_DeviceHandle, VCNL4000_AMBIENTDATA);
    }
    delay(1);
  }
  // timeout
  return 0;
}

float ProxiVCNL4000::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 : return float(get());
    case 1 : return float(getAmbient());
    default : throw std::out_of_range("ProxiVCNL4000::getAnalog: index out of range");
  }
}

std::string ProxiVCNL4000::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Distance";
    case 1 : return "Brightness";
    default : throw std::out_of_range("ProxiVCNL4000::getAnalogChannelDesc: index out of range");
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
