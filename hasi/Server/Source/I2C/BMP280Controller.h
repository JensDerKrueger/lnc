#pragma once

#include "BMPControllerAbstr.h"

namespace I2C {

  /*=========================================================================
   REGISTERS
   -----------------------------------------------------------------------*/
  enum
  {
    BMP280_REGISTER_DIG_T1              = 0x88,
    BMP280_REGISTER_DIG_T2              = 0x8A,
    BMP280_REGISTER_DIG_T3              = 0x8C,

    BMP280_REGISTER_DIG_P1              = 0x8E,
    BMP280_REGISTER_DIG_P2              = 0x90,
    BMP280_REGISTER_DIG_P3              = 0x92,
    BMP280_REGISTER_DIG_P4              = 0x94,
    BMP280_REGISTER_DIG_P5              = 0x96,
    BMP280_REGISTER_DIG_P6              = 0x98,
    BMP280_REGISTER_DIG_P7              = 0x9A,
    BMP280_REGISTER_DIG_P8              = 0x9C,
    BMP280_REGISTER_DIG_P9              = 0x9E,

    BMP280_REGISTER_CHIPID             = 0xD0,
    BMP280_REGISTER_VERSION            = 0xD1,
    BMP280_REGISTER_SOFTRESET          = 0xE0,

    BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

    BMP280_REGISTER_CONTROL            = 0xF4,
    BMP280_REGISTER_CONFIG             = 0xF5,
    BMP280_REGISTER_PRESSUREDATA       = 0xF7,
    BMP280_REGISTER_TEMPDATA           = 0xFA,
  };

  /*=========================================================================*/

  /*=========================================================================
   CALIBRATION DATA
   -----------------------------------------------------------------------*/
  typedef struct
  {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
  } bmp280_calib_data;
  /*=========================================================================*/

  class BMP280Controller : public BMPControllerAbstr {
  public:
    BMP280Controller(int deviceHandle);

    virtual std::string getDesc() const;
    virtual void init();
    virtual float getPressure();
    virtual float getTemp();

  protected:
    void getCalibrationData(void);

    void      write8(uint8_t reg, uint8_t value);
    uint8_t   read8(uint8_t reg);
    uint16_t  read16(uint8_t reg);

    int16_t   readS16(uint32_t reg);
    uint16_t  read16_LE(uint32_t reg); // little endian
    int16_t   readS16_LE(uint32_t reg); // little endian

    uint8_t   _i2caddr;
    int32_t   _sensorID;
    int32_t t_fine;

    int8_t _cs, _mosi, _miso, _sck;

    bmp280_calib_data _bmp280_calib;
  };

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

