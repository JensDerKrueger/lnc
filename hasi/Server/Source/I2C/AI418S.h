#pragma once

#ifndef AI418S_H
#define AI418S_H

#include "I2CMember.h"
#include "IAnalogIn.h"
#include <array>
#include <cmath>

namespace I2C {

class AI418S : public I2CMember, public IAnalogIn {
public:
  AI418S(uint8_t busID, uint8_t i2cAddress,
         const std::string& devID,
         const std::string& hrname,
         const HAS::HASConfigPtr config,
         std::shared_ptr<I2CBusManager> busManager);
  virtual uint8_t getAnalogInChannelCount() const {return 4;}
  virtual uint32_t pollAnalogIn() {return 0;}
  virtual float getAnalog(uint8_t iChannel);

  void setMultiplierAndPrecision(double fMultiplier, uint32_t decimals) {
    m_fMultiplier = fMultiplier;
    m_fDecimalFact = float(pow(10.0,double(decimals)));
  }

  enum EPrecision {
    PREC_12BIT = 0,
    PREC_14BIT = 1,
    PREC_16BIT = 2,
    PREC_18BIT = 3
  };
  void setPrecision(const EPrecision prec) {
    m_prec = prec;
  }
  enum EGain {
    GAIN_1X = 0,
    GAIN_2X = 1,
    GAIN_4X = 2,
    GAIN_8X = 3
  };
  void setPrecision(const EGain gain) {
    m_gain = gain;
  }
  void setContinousConversion(const bool bContConv) {
    m_bContConv = bContConv;
  }

private:
  EPrecision m_prec;
  EGain m_gain;
  bool m_bContConv;
  double m_fMultiplier;
  float m_fDecimalFact;

  void sendConfigData(uint8_t iChannel);
  double readValue();
  double getMaxData() const;
  double getGainValue() const;

  virtual std::string getDesc() const;
};

}

#endif // AI418S_H

/*
   The MIT License

   Copyright (c) 2014 Jens Krueger

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

