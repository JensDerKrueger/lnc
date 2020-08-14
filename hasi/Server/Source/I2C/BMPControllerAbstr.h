#pragma once

#include "I2CBase.h"

namespace I2C {

  class BMPControllerAbstr  {
  public:
    BMPControllerAbstr(int deviceHandle) : m_deviceHandle(deviceHandle) {}
    virtual ~BMPControllerAbstr() {}

    virtual std::string getDesc() const = 0;
    virtual void init()= 0;
    virtual float getPressure() = 0;
    virtual float getTemp() = 0;

  protected:
    int m_deviceHandle;
    int8_t version;

    int16_t readShort(uint8_t addr) {
      I2CBase::write(m_deviceHandle, addr);
      uint8_t msb = I2CBase::read(m_deviceHandle);
      I2CBase::write(m_deviceHandle, addr+1);
      uint8_t lsb = I2CBase::read(m_deviceHandle);
      return (msb<<8) | lsb;
    }

    
  };

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

