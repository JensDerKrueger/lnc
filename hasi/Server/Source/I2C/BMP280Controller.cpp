#include <sstream>
#include "BMP280Controller.h"

using namespace I2C;

BMP280Controller::BMP280Controller(int deviceHandle) :
BMPControllerAbstr(deviceHandle)
{
  version = 0;
}

std::string BMP280Controller::getDesc() const {
  return "Bosch BMP280 Barometric Pressure Sensor";
}

void BMP280Controller::init() {
  getCalibrationData();
  I2CBase::writeReg8(m_deviceHandle, BMP280_REGISTER_CONTROL, 0x3F);
}

float BMP280Controller::getPressure() {
  int64_t var1, var2, p;

  int32_t adc_P = read16(BMP280_REGISTER_PRESSUREDATA);
  adc_P <<= 8;
  adc_P |= read8(BMP280_REGISTER_PRESSUREDATA+2);
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
  var2 = var2 + ((var1*(int64_t)_bmp280_calib.dig_P5)<<17);
  var2 = var2 + (((int64_t)_bmp280_calib.dig_P4)<<35);
  var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3)>>8) +
  ((var1 * (int64_t)_bmp280_calib.dig_P2)<<12);
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)_bmp280_calib.dig_P1)>>33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((int64_t)_bmp280_calib.dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7)<<4);
  return (float)p/256;
}


float BMP280Controller::getTemp() {
  int32_t var1, var2;

  int32_t adc_T = read16(BMP280_REGISTER_TEMPDATA);
  adc_T <<= 8;
  adc_T |= read8(BMP280_REGISTER_TEMPDATA+2);
  adc_T >>= 4;

  var1  = ((((adc_T>>3) - ((int32_t)_bmp280_calib.dig_T1 <<1))) *
           ((int32_t)_bmp280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1)) *
             ((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) *
           ((int32_t)_bmp280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  int32_t T  = (t_fine * 5 + 128) >> 8;
  return float(T)/100.0f;
}

uint16_t BMP280Controller::read16_LE(uint32_t reg) {
#ifndef NI2C
  ssize_t ret = write(m_deviceHandle, &reg, 1);
  if (ret != 1) return 0;

  uint8_t buffer[2];
  ret = read(m_deviceHandle, buffer, 2);
  if (ret != 2) return 0;

  int h = buffer[1] << 8;
  int l = buffer[0];

  return (uint16_t)(h + l);
#else
	return 0;
#endif
}

int16_t BMP280Controller::readS16(uint32_t reg)
{
  return (int16_t)read16(reg);

}

int16_t BMP280Controller::readS16_LE(uint32_t reg)
{
  return (int16_t)read16_LE(reg);

}
void BMP280Controller::getCalibrationData(void)
{
  _bmp280_calib.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1);
  _bmp280_calib.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2);
  _bmp280_calib.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3);

  _bmp280_calib.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1);
  _bmp280_calib.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2);
  _bmp280_calib.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3);
  _bmp280_calib.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4);
  _bmp280_calib.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5);
  _bmp280_calib.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6);
  _bmp280_calib.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7);
  _bmp280_calib.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8);
  _bmp280_calib.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9);
}

void BMP280Controller::write8(uint8_t reg, uint8_t value){
  I2CBase::writeReg8(m_deviceHandle, reg, value);
}

uint8_t BMP280Controller::read8(uint8_t reg){
 return I2CBase::readReg8(m_deviceHandle, reg);
}

uint16_t BMP280Controller::read16(uint8_t reg) {
#ifndef NI2C
  ssize_t ret = write(m_deviceHandle, &reg, 1);
  if (ret != 1) return 0;

  uint8_t buffer[2];
  ret = read(m_deviceHandle, buffer, 2);
  if (ret != 2) return 0;

  return (buffer[0] << 8) | buffer[1];
#else
  return 0;
#endif
}

/*
 The MIT License

 Copyright (c) 2013 Jens Krueger
 based on the BMP280 Ardunio Library by Adafruit

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
