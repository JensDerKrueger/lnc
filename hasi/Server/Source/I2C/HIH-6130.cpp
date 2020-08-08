#include "HIH-6130.h"

using namespace I2C;

#include <sstream>

HIH6130::HIH6130(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID,
                 const std::string& hrname,
                 const HAS::HASConfigPtr config,
                 std::shared_ptr<I2CBusManager> busManager) :
  I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
  m_status(255)
{
}

std::string HIH6130::getDesc() const {
  switch (m_status) {
    case 0 : return "Sparkfun HIH6130 Humidity Sensor (normal operation)";
    case 1 : return "Sparkfun HIH6130 Humidity Sensor (stale data mode)";
    case 2 : return "Sparkfun HIH6130 Humidity Sensor (in command mode)";
    case 3 : return "Sparkfun HIH6130 Humidity Sensor (diagnostic mode)";
    default : return "Sparkfun HIH6130 Humidity Sensor";
  }
}

void HIH6130::init() {
  I2CMember::init();

  uint16_t h,t;
  getHandT(h,t); //update status

  if (m_status > 1) {
    throw EI2CDeviceInit(std::string("Unable to init ") +
                         getDesc() +
                         std::string(" wrong i2c address?"));
  }
}

void HIH6130::getHandT(uint16_t &h, uint16_t &t) {
  activate();
  
  I2CBase::write(m_DeviceHandle, 0);
  delay(50);

  uint8_t Hum_H = I2CBase::read(m_DeviceHandle);
  uint8_t Hum_L = I2CBase::read(m_DeviceHandle);
  uint8_t Temp_H = I2CBase::read(m_DeviceHandle);
  uint8_t Temp_L = I2CBase::read(m_DeviceHandle);

  m_status = (Hum_H >> 6) & 0x03;
  Hum_H = Hum_H & 0x3f;
  h = (((uint16_t)Hum_H) << 8) | Hum_L;
  t = (((uint16_t)Temp_H) << 8) | Temp_L;

  t /= 4;
}

uint32_t HIH6130::pollAnalogIn() {
  return 0;
}

float HIH6130::get() {
  uint16_t h, t;
  getHandT(h,t);
  return float(double(h) * .006103888176769);
}


float HIH6130::getTemp() {
  uint16_t h, t;
  getHandT(h,t);
  return float( double(t) * 0.010071415491668 - 40.0);
}

float HIH6130::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 : return get();
    case 1 : return getTemp();
    default : throw std::out_of_range("HIH6130::getAnalog: index out of range");
  }
}

std::string HIH6130::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Relative Humidity";
    case 1 : return "Temperature";
    default : throw std::out_of_range("HIH6130::getAnalogChannelDesc: index out of range");
  }
}

std::string HIH6130::getAnalogChannelUnit(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "%";
    case 1 : return "°C";
    default : throw std::out_of_range("HIH6130::getAnalogChannelUnit: index out of range");
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
