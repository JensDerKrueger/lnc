#include <cmath>  // sqrt

#include "TMP006.h"

#define TMP006_B0 -0.0000294
#define TMP006_B1 -0.00000057
#define TMP006_B2 0.00000000463
#define TMP006_C2 13.4
#define TMP006_TREF 298.15
#define TMP006_A2 -0.00001678
#define TMP006_A1 0.00175
#define TMP006_S0 6.4  // * 10^-14

#define TMP006_CONFIG       0x02

#define TMP006_CFG_RESET    (uint16_t)0x8000
#define TMP006_CFG_MODEON   (uint16_t)0x7000
#define TMP006_CFG_1SAMPLE  (uint16_t)0x0000
#define TMP006_CFG_2SAMPLE  (uint16_t)0x0200
#define TMP006_CFG_4SAMPLE  (uint16_t)0x0400
#define TMP006_CFG_8SAMPLE  (uint16_t)0x0600
#define TMP006_CFG_16SAMPLE (uint16_t)0x0800
#define TMP006_CFG_DRDYEN   (uint16_t)0x0100
#define TMP006_CFG_DRDY     (uint16_t)0x0080

#define TMP006_MANID 0xFE
#define TMP006_DEVID 0xFF

#define TMP006_VOBJ  0x0
#define TMP006_TAMB  0x01


using namespace I2C;

#include <sstream>

TMP006::TMP006(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID,
               const std::string& hrname,
               const HAS::HASConfigPtr config,
               std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager)
{
}

std::string TMP006::getDesc() const {
  return "Adafruit Contact-less Infrared Thermopile Sensor";
}

void TMP006::init() {
  I2CMember::init();

  activate();

  uint16_t initialControl = TMP006_CFG_MODEON | TMP006_CFG_DRDYEN | TMP006_CFG_2SAMPLE;
  write16BE(TMP006_CONFIG, initialControl);

  uint16_t mid, did;
  mid = read16(TMP006_MANID);
  did = read16(TMP006_DEVID);

  if (mid != 0x5449 || did != 0x67) {
    throw EI2CDeviceInit(std::string("Unable to init ") +
                         getDesc() +
                         std::string(" wrong i2c address?"));
  }
}

uint32_t TMP006::pollAnalogIn() {
  return 0;
}

float TMP006::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 : return (float)readDieTempC();
    case 1 : return (float)readObjTempC();
    default : throw std::out_of_range("TMP006::getAnalog: index out of range");
  }
}

std::string TMP006::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Die Temperature";
    case 1 : return "Object Temperature";
    default : throw std::out_of_range("TMP006::getAnalogChannelDesc: index out of range");
  }
}

std::string TMP006::getAnalogChannelUnit(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "°C";
    case 1 : return "°C";
    default : throw std::out_of_range("TMP006::getAnalogChannelUnit: index out of range");
  }
}


double TMP006::readDieTempC() {
  activate();
  double Tdie = readRawDieTemperature();
  Tdie *= 0.03125; // convert to celsius
  return Tdie;
}

double TMP006::readObjTempC() {
  activate();

  double Tdie = readRawDieTemperature();
  double Vobj = readRawVoltage();
  Vobj *= 156.25;  // 156.25 nV per LSB
  Vobj /= 1000; // nV -> uV
  Vobj /= 1000; // uV -> mV
  Vobj /= 1000; // mV -> V
  Tdie *= 0.03125; // convert to celsius
  Tdie += 273.15; // convert to kelvin

  double tdie_tref = Tdie - TMP006_TREF;
  double S = (1 + TMP006_A1*tdie_tref +
                 TMP006_A2*tdie_tref*tdie_tref);
  S *= TMP006_S0;
  S /= 10000000;
  S /= 10000000;

  double Vos = TMP006_B0 + TMP006_B1*tdie_tref +
               TMP006_B2*tdie_tref*tdie_tref;
  double fVobj = (Vobj - Vos) + TMP006_C2*(Vobj-Vos)*(Vobj-Vos);
  double Tobj = sqrt(sqrt(Tdie * Tdie * Tdie * Tdie + fVobj/S));
  Tobj -= 273.15; // Kelvin -> *C
  return Tobj;
}

int16_t TMP006::readRawDieTemperature() {
  int16_t raw = read16(TMP006_TAMB);
  raw >>= 2;
  return raw;
}

int16_t TMP006::readRawVoltage() {
  return read16(TMP006_VOBJ);
}

uint16_t TMP006::read16(uint8_t reg) {
#ifndef NI2C
  ssize_t ret = write(m_DeviceHandle, &reg, 1);
  if (ret != 1) return 0;

  uint8_t buffer[2];
  ret = read(m_DeviceHandle, buffer, 2);
  if (ret != 2) return 0;


  return (buffer[0] << 8) | buffer[1];
#else
  return 0;
#endif
}

void TMP006::write16BE(uint8_t reg, uint16_t value){
  value = ((value & 0xFF) << 8) | (value >> 8);  // little to big endian conversion
  I2CBase::writeReg16(m_DeviceHandle, reg, value);
}


/*
 The MIT License
 
 Copyright (c) 2016 Jens Krueger
 
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
