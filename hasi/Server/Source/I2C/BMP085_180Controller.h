#pragma once

#include <array>
#include "BMPControllerAbstr.h"

namespace I2C {

  class BMP085_180Controller : public BMPControllerAbstr {
  public:
    BMP085_180Controller(int deviceHandle);

    virtual std::string getDesc() const;
    virtual void init();
    virtual float getPressure();
    virtual float getTemp();


  protected:
    // oversampling for measurement
    uint8_t oversampling_setting;

    std::array<uint8_t, 4> pressure_conversiontime;

    // sensor registers from the BOSCH BoschBMP datasheet
    int16_t ac1, ac2, ac3;
    uint16_t ac4, ac5, ac6;
    int16_t b1, b2, mb, mc, md;
    int8_t version;

    // read uncompensated temperature value
    int32_t readUT();

    // read uncompensated pressure value
    int32_t readUP();

    void getCalibrationData();
  };

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

