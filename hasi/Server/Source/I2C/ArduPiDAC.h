#pragma once

#ifndef ARDUPIDAC_H
#define ARDUPIDAC_H

#include "I2CMember.h"
#include "IAnalogIn.h"
#include <array>

namespace I2C {

class ArduPiDAC : public I2CMember, public IAnalogIn {
public:
  ArduPiDAC(uint8_t busID=1, uint8_t i2cAddress=0x08,
            const std::string& devID="BI_DAC",
            const std::string& hrname="Build-In DAC",
            const HAS::HASConfigPtr config = nullptr,
            std::shared_ptr<I2CBusManager> busManager = nullptr);

  virtual uint8_t getAnalogInChannelCount() const {return 8;}
  virtual uint32_t pollAnalogIn();
  virtual float getAnalog(uint8_t iChannel);

private:
  std::array<float,8> m_val;

  static uint8_t addresses[8];
  virtual std::string getDesc() const;

  float get(uint8_t input);
};

}

#endif // ARDUPIDAC_H

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

