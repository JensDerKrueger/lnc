#pragma once

#ifndef LUMINOSITYTSL2561_H
#define LUMINOSITYTSL2561_H

#include "I2CMember.h"
#include "IAnalogIn.h"

namespace I2C {

class LuminosityTSL2561 : public I2CMember, public IAnalogIn {
public:
  typedef enum
  {
    FULLSPECTRUM              = 0x00,
    INFRARED                  = 0x01,
    VISIBLE                   = 0x02
  }
  Channel_t;

  typedef enum
  {
    GAIN_0X                   = 0x00,    // No gain
    GAIN_16X                  = 0x10,    // 16x gain
  }
  Gain_t;

  typedef enum
  {
    INTEGRATIONTIME_13MS      = 0x00,    // 13.7ms
    INTEGRATIONTIME_101MS     = 0x01,    // 101ms
    INTEGRATIONTIME_402MS     = 0x02     // 402ms
  }
  IntTime_t;

  LuminosityTSL2561(uint8_t busID, uint8_t i2cAddress,
                    const std::string& devID,
                    const std::string& hrname,
                    const HAS::HASConfigPtr config,
                    std::shared_ptr<I2CBusManager> busManager);

  virtual void init();

  uint16_t getLuminosity(Channel_t channel=VISIBLE);
  uint16_t getLux();
  void setTiming(IntTime_t integration);
  void setGain(Gain_t gain);

  // IAnalogIn Interface
  virtual uint8_t getAnalogInChannelCount() const {return 1;}
  virtual uint32_t pollAnalogIn() {return 0;}
  virtual float getAnalog(uint8_t iChannel);
  virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
  virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;

protected:
  IntTime_t m_integration;
  Gain_t m_gain;

  virtual std::string getDesc() const;

  uint32_t getFullLuminosity ();
  uint32_t calculateLux(uint16_t ch0, uint16_t ch1) const;

  void enable();
  void disable();

};

}

#endif // LUMINOSITYTSL2561_H

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
 
   Derived from the tsl2561.h file by K. Townsend (microBuilder.eu) 
   that was released as
   
   Software License Agreement (BSD License)

   Copyright (c) 2010, microBuilder SARL
   All rights reserved.
 
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   3. Neither the name of the copyright holders nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.
 
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

