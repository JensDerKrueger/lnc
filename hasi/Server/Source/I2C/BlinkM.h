#pragma once

#ifndef BLINKM_H
#define BLINKM_H

#include "I2CMember.h"
#include <cstdarg>

namespace I2C {

class BlinkM : public I2CMember {
public:
  BlinkM(uint8_t busID, uint8_t i2cAddress,
         const std::string& devID,
         const std::string& hrname,
         const HAS::HASConfigPtr config,
         std::shared_ptr<I2CBusManager> busManager);
  virtual void init();
  void ChangeTo(uint8_t r, uint8_t g, uint8_t b);
  void ChangeTo(float r, float g, float b);
  void FadeTo(uint8_t r, uint8_t g, uint8_t b);
  void FadeTo(float r, float g, float b);
  void GetColor(uint8_t& r, uint8_t& g, uint8_t& b);
  void ChangeI2CAddress(uint8_t newAddress);
  
protected:
  void WriteCommand(unsigned char cmd, size_t count, ...);
  virtual std::string getDesc() const;
};

}

#endif // BLINKM_H

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

