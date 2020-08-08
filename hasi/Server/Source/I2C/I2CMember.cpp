#include "I2CMember.h"
#include "I2CBusManager.h"

using namespace I2C;

I2CMember::I2CMember(uint8_t busID, uint8_t i2cAddress,
                     const std::string& devID, const std::string& hrname,
                     const HAS::HASConfigPtr config,
                     std::shared_ptr<I2CBusManager> busManager) :
HAS::HASMember(devID, hrname, config),
m_i2cBusID(busID),
m_i2cAddress(i2cAddress),
m_DeviceHandle(-1),
m_busManager(busManager)
{
}

I2CMember::~I2CMember() {
#ifndef NI2C
  close(m_DeviceHandle);  // TODO: find out if I really need todo this
#endif
}

void I2CMember::init() {
  if (m_busManager)
    m_DeviceHandle = I2CBase::setupBus(m_i2cAddress, m_busManager->m_i2cBusID);
  else
    m_DeviceHandle = I2CBase::setupBus(m_i2cAddress, m_i2cBusID);
    
  
  if (m_DeviceHandle == -1)
    throw EI2CDeviceInit("Unable to create device handle");
}

uint8_t I2CMember::getBus() const {
  if (m_busManager)
    return m_busManager->getBus();
  else
    return m_i2cBusID;
}


std::string I2CMember::toString() const {
  std::stringstream ss;
  if (m_busManager) {
    ss << getDesc() << " \"" << m_hrname << "\" connected to bus "
    << int(m_i2cBusID) << " of bus manager " << m_busManager->m_hrname
    << " with I2C address " << int(m_i2cAddress)
    << " (" << int_to_hex(m_i2cAddress) << ") ID=" << m_devID
    << " (" << (getIsActive() ? "active" : "inactive") << ")";
  } else {
    ss << getDesc() << " \"" << m_hrname << "\" connected to bus "
    << int(m_i2cBusID) << " with I2C address " << int(m_i2cAddress)
    << " (" << int_to_hex(m_i2cAddress) << ") ID=" << m_devID
    << " (" << (getIsActive() ? "active" : "inactive") << ")";
  }
  return ss.str();
}

std::string I2CMember::getDesc() const {
  return "Unknown I2C device";
}

void I2CMember::activate() {
  if (m_busManager)
    m_busManager->setActiveBus(m_i2cBusID);
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
