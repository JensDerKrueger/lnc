#include <memory>  // make_unique

#include "BoschBMP.h"
#include "BMP085_180Controller.h"
#include "BMP280Controller.h"

using namespace I2C;

#include <sstream>

BoschBMP::BoschBMP(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID,
               const std::string& hrname,
                   const HAS::HASConfigPtr config,
                   std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
m_pController(nullptr){
}

std::string BoschBMP::getDesc() const {
  if (m_pController)
    return m_pController->getDesc();
  else
    return "Uninitiallized Bosch Sensor";
}

void BoschBMP::init() {
  I2CMember::init();

  activate();
  I2CBase::write(m_DeviceHandle, 0xD0);
  int id = I2CBase::read(m_DeviceHandle);

  switch (id) {
    case 0x55 : m_pController = std::unique_ptr<BMP085_180Controller>(new BMP085_180Controller(m_DeviceHandle)); break;
    case 0x58 : m_pController = std::unique_ptr<BMP280Controller>(new BMP280Controller(m_DeviceHandle)); break;
    default : throw EI2CDeviceInit("Unable to init Bosch Sensor, wrong i2c address?");
  }

  m_pController->init();
}

uint32_t BoschBMP::pollAnalogIn() {
  return 0; // reads are all blocking so no wait between poll and read required
}

float BoschBMP::getAnalog(uint8_t iChannel) {
  if (!m_pController) return -1;

  activate();
  switch (iChannel) {
    case 0 : return m_pController->getPressure();
    case 1 : return m_pController->getTemp();
    default : throw std::out_of_range("BoschBMP::getAnalog: index out of range");
  }
}

std::string BoschBMP::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Pressure";
    case 1 : return "Temperature";
    default : throw std::out_of_range("BoschBMP::getAnalogChannelDesc: index out of range");
  }
}

std::string BoschBMP::getAnalogChannelUnit(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "hPa";
    case 1 : return "°C";
    default : throw std::out_of_range("BoschBMP::getAnalogChannelUnit: index out of range");
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
