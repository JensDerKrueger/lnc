#include "BMP085_180Controller.h"

using namespace I2C;

#include <sstream>

BMP085_180Controller::BMP085_180Controller(int deviceHandle) :
BMPControllerAbstr(deviceHandle)
{
  pressure_conversiontime[0] = 5;
  pressure_conversiontime[1] = 8;
  pressure_conversiontime[2] = 14;
  pressure_conversiontime[3] = 26;
  oversampling_setting = 3;
  version = 0;
}

std::string BMP085_180Controller::getDesc() const {
  if (version == 0)
    return "Bosch BMP085 / BMP180 Barometric Pressure Sensor";
  else {
    std::stringstream ss;
    ss << "Bosch BMP085 / BMP180 Barometric Pressure Sensor (Ver. " << (int)version << ")";
    return ss.str();
  }
}

void BMP085_180Controller::init() {
  I2CBase::write(m_deviceHandle, 0xD1);
  version = I2CBase::read(m_deviceHandle);
  getCalibrationData();
}

float BMP085_180Controller::getPressure() {
  int32_t ut= readUT();
  int32_t up = readUP();
  int32_t x1, x2, x3, b3, b5, b6, p;
  uint32_t b4, b7;

  //calculate true temperature
  x1 = ((long)ut - ac6) * ac5 >> 15;
  x2 = ((long) mc << 11) / (x1 + md);
  b5 = x1 + x2;

  //calculate true pressure
  b6 = b5 - 4000;
  x1 = (b2 * (b6 * b6 >> 12)) >> 11;
  x2 = ac2 * b6 >> 11;
  x3 = x1 + x2;
  b3 = ((((int32_t) ac1 * 4 + x3)<< oversampling_setting) + 2) >> 2;
  x1 = ac3 * b6 >> 13;
  x2 = (b1 * (b6 * b6 >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
  b7 = ((uint32_t) up - b3) * (50000 >> oversampling_setting);
  p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  return (p + ((x1 + x2 + 3791) >> 4))/100.0f;
}


float BMP085_180Controller::getTemp() {
  int32_t ut = readUT();
  int32_t x1, x2, b5;

  //calculate true temperature
  x1 = (ut - ac6) * ac5 >> 15;
  x2 = (int32_t(mc) << 11) / (x1 + md);
  b5 = x1 + x2;
  return ((b5 + 8) >> 4) * 0.1f;
}


int32_t BMP085_180Controller::readUT() {
  I2CBase::writeReg8(m_deviceHandle, 0xf4, 0x2e);
  delay(5);
  return readShort(0xf6);
}

int32_t BMP085_180Controller::readUP() {
  I2CBase::writeReg8(m_deviceHandle, 0xf4,0x34+(oversampling_setting<<6));
  delay(pressure_conversiontime[oversampling_setting]);

  uint8_t msb, lsb, xlsb;
  I2CBase::write(m_deviceHandle, 0xf6);
  msb = I2CBase::read(m_deviceHandle);
  I2CBase::write(m_deviceHandle, 0xf7);
  lsb = I2CBase::read(m_deviceHandle);
  I2CBase::write(m_deviceHandle, 0xf8);
  xlsb = I2CBase::read(m_deviceHandle);

  return (((int32_t)msb<<16) | ((int32_t)lsb<<8) | ((int32_t)xlsb)) >>(8-oversampling_setting);
}

void BMP085_180Controller::getCalibrationData() {
  ac1 = readShort(0xAA);
  ac2 = readShort(0xAC);
  ac3 = readShort(0xAE);
  ac4 = readShort(0xB0);
  ac5 = readShort(0xB2);
  ac6 = readShort(0xB4);
  b1 = readShort(0xB6);
  b2 = readShort(0xB8);
  mb = readShort(0xBA);
  mc = readShort(0xBC);
  md = readShort(0xBE);
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
