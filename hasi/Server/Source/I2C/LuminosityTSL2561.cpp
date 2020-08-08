#include "LuminosityTSL2561.h"

enum
{
  TSL2561_REGISTER_CONTROL          = 0x00,
  TSL2561_REGISTER_TIMING           = 0x01,
  TSL2561_REGISTER_THRESHHOLDL_LOW  = 0x02,
  TSL2561_REGISTER_THRESHHOLDL_HIGH = 0x03,
  TSL2561_REGISTER_THRESHHOLDH_LOW  = 0x04,
  TSL2561_REGISTER_THRESHHOLDH_HIGH = 0x05,
  TSL2561_REGISTER_INTERRUPT        = 0x06,
  TSL2561_REGISTER_CRC              = 0x08,
  TSL2561_REGISTER_ID               = 0x0A,
  TSL2561_REGISTER_CHAN0_LOW        = 0x0C,
  TSL2561_REGISTER_CHAN0_HIGH       = 0x0D,
  TSL2561_REGISTER_CHAN1_LOW        = 0x0E,
  TSL2561_REGISTER_CHAN1_HIGH       = 0x0F
};

#define TSL2561_LUX_LUXSCALE      (14)      // Scale by 2^14
#define TSL2561_LUX_RATIOSCALE    (9)       // Scale ratio by 2^9
#define TSL2561_LUX_CHSCALE       (10)      // Scale channel values by 2^10
#define TSL2561_LUX_CHSCALE_TINT0 (0x7517)  // 322/11 * 2^TSL2561_LUX_CHSCALE
#define TSL2561_LUX_CHSCALE_TINT1 (0x0FE7)  // 322/81 * 2^TSL2561_LUX_CHSCALE

#define TSL2561_COMMAND_BIT       (0x80)    // Must be 1
#define TSL2561_WORD_BIT          (0x20)    // 1 = read/write word (rather than uint8_t)

// T, FN and CL package values
#define TSL2561_LUX_K1T           (0x0040)  // 0.125 * 2^RATIO_SCALE
#define TSL2561_LUX_B1T           (0x01f2)  // 0.0304 * 2^LUX_SCALE
#define TSL2561_LUX_M1T           (0x01be)  // 0.0272 * 2^LUX_SCALE
#define TSL2561_LUX_K2T           (0x0080)  // 0.250 * 2^RATIO_SCALE
#define TSL2561_LUX_B2T           (0x0214)  // 0.0325 * 2^LUX_SCALE
#define TSL2561_LUX_M2T           (0x02d1)  // 0.0440 * 2^LUX_SCALE
#define TSL2561_LUX_K3T           (0x00c0)  // 0.375 * 2^RATIO_SCALE
#define TSL2561_LUX_B3T           (0x023f)  // 0.0351 * 2^LUX_SCALE
#define TSL2561_LUX_M3T           (0x037b)  // 0.0544 * 2^LUX_SCALE
#define TSL2561_LUX_K4T           (0x0100)  // 0.50 * 2^RATIO_SCALE
#define TSL2561_LUX_B4T           (0x0270)  // 0.0381 * 2^LUX_SCALE
#define TSL2561_LUX_M4T           (0x03fe)  // 0.0624 * 2^LUX_SCALE
#define TSL2561_LUX_K5T           (0x0138)  // 0.61 * 2^RATIO_SCALE
#define TSL2561_LUX_B5T           (0x016f)  // 0.0224 * 2^LUX_SCALE
#define TSL2561_LUX_M5T           (0x01fc)  // 0.0310 * 2^LUX_SCALE
#define TSL2561_LUX_K6T           (0x019a)  // 0.80 * 2^RATIO_SCALE
#define TSL2561_LUX_B6T           (0x00d2)  // 0.0128 * 2^LUX_SCALE
#define TSL2561_LUX_M6T           (0x00fb)  // 0.0153 * 2^LUX_SCALE
#define TSL2561_LUX_K7T           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B7T           (0x0018)  // 0.00146 * 2^LUX_SCALE
#define TSL2561_LUX_M7T           (0x0012)  // 0.00112 * 2^LUX_SCALE
#define TSL2561_LUX_K8T           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B8T           (0x0000)  // 0.000 * 2^LUX_SCALE
#define TSL2561_LUX_M8T           (0x0000)  // 0.000 * 2^LUX_SCALE

// CS package values
#define TSL2561_LUX_K1C           (0x0043)  // 0.130 * 2^RATIO_SCALE
#define TSL2561_LUX_B1C           (0x0204)  // 0.0315 * 2^LUX_SCALE
#define TSL2561_LUX_M1C           (0x01ad)  // 0.0262 * 2^LUX_SCALE
#define TSL2561_LUX_K2C           (0x0085)  // 0.260 * 2^RATIO_SCALE
#define TSL2561_LUX_B2C           (0x0228)  // 0.0337 * 2^LUX_SCALE
#define TSL2561_LUX_M2C           (0x02c1)  // 0.0430 * 2^LUX_SCALE
#define TSL2561_LUX_K3C           (0x00c8)  // 0.390 * 2^RATIO_SCALE
#define TSL2561_LUX_B3C           (0x0253)  // 0.0363 * 2^LUX_SCALE
#define TSL2561_LUX_M3C           (0x0363)  // 0.0529 * 2^LUX_SCALE
#define TSL2561_LUX_K4C           (0x010a)  // 0.520 * 2^RATIO_SCALE
#define TSL2561_LUX_B4C           (0x0282)  // 0.0392 * 2^LUX_SCALE
#define TSL2561_LUX_M4C           (0x03df)  // 0.0605 * 2^LUX_SCALE
#define TSL2561_LUX_K5C           (0x014d)  // 0.65 * 2^RATIO_SCALE
#define TSL2561_LUX_B5C           (0x0177)  // 0.0229 * 2^LUX_SCALE
#define TSL2561_LUX_M5C           (0x01dd)  // 0.0291 * 2^LUX_SCALE
#define TSL2561_LUX_K6C           (0x019a)  // 0.80 * 2^RATIO_SCALE
#define TSL2561_LUX_B6C           (0x0101)  // 0.0157 * 2^LUX_SCALE
#define TSL2561_LUX_M6C           (0x0127)  // 0.0180 * 2^LUX_SCALE
#define TSL2561_LUX_K7C           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B7C           (0x0037)  // 0.00338 * 2^LUX_SCALE
#define TSL2561_LUX_M7C           (0x002b)  // 0.00260 * 2^LUX_SCALE
#define TSL2561_LUX_K8C           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B8C           (0x0000)  // 0.000 * 2^LUX_SCALE
#define TSL2561_LUX_M8C           (0x0000)  // 0.000 * 2^LUX_SCALE

#define TSL2561_CONTROL_POWERON   (0x03)
#define TSL2561_CONTROL_POWEROFF  (0x00)


using namespace I2C;

LuminosityTSL2561::LuminosityTSL2561(uint8_t busID, uint8_t i2cAddress,
                                     const std::string& devID,
                                     const std::string& hrname,
                                     const HAS::HASConfigPtr config,
                                     std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
m_integration(INTEGRATIONTIME_13MS),
m_gain(GAIN_16X)
{
}

std::string LuminosityTSL2561::getDesc() const {
  return "Adafruit TSL2561 digital luminosity sensor";
}


void LuminosityTSL2561::init() {
  I2CMember::init();
  activate();
  I2CBase::write(m_DeviceHandle, TSL2561_REGISTER_ID);
  int x = I2CBase::read(m_DeviceHandle);

  if (! (x & 0x0A) ) {
    throw EI2CDeviceInit("Unable to init Luminosity TSL2561 sensor,"
                         " wrong i2c address?");
  }

  // Set default integration time and gain
  setTiming(m_integration);
  setGain(m_gain);

  // Note: by default, the device is in power down mode on bootup
  disable();
}

void LuminosityTSL2561::enable()
{
  activate();
  // Enable the device by setting the control bit to 0x03
  I2CBase::writeReg8(m_DeviceHandle,
                       TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
                       TSL2561_CONTROL_POWERON);
}

void LuminosityTSL2561::disable()
{
  activate();
  // Disable the device by setting the control bit to 0x03
  I2CBase::writeReg8(m_DeviceHandle,
                       TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
                       TSL2561_CONTROL_POWEROFF);
}


void LuminosityTSL2561::setGain(LuminosityTSL2561::Gain_t gain) {
  enable();
  m_gain = gain;
  I2CBase::writeReg8(m_DeviceHandle,
                       TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,
                       m_integration | m_gain);
  disable();
}

void LuminosityTSL2561::setTiming(LuminosityTSL2561::IntTime_t integration)
{
  enable();
  m_integration = integration;
  I2CBase::writeReg8(m_DeviceHandle,
                       TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,
                       m_integration | m_gain);
  disable();
}

uint32_t LuminosityTSL2561::calculateLux(uint16_t ch0, uint16_t ch1) const
{
  unsigned long chScale;
  unsigned long channel1;
  unsigned long channel0;
  
  switch (m_integration)
  {
    case INTEGRATIONTIME_13MS:
      chScale = TSL2561_LUX_CHSCALE_TINT0;
      break;
    case INTEGRATIONTIME_101MS:
      chScale = TSL2561_LUX_CHSCALE_TINT1;
      break;
    default: // No scaling ... integration time = 402ms
      chScale = (1 << TSL2561_LUX_CHSCALE);
      break;
  }
  
  // Scale for gain (1x or 16x)
  if (!m_gain) chScale = chScale << 4;
  
  // scale the channel values
  channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
  channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;
  
  // find the ratio of the channel values (Channel1/Channel0)
  unsigned long ratio1 = 0;
  if (channel0 != 0) ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
  
  // round the ratio value
  unsigned long ratio = (ratio1 + 1) >> 1;
  
  unsigned int b=TSL2561_LUX_B1T,
               m=TSL2561_LUX_M1T;
  
  if (ratio <= TSL2561_LUX_K1T)
  {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
  else if (ratio <= TSL2561_LUX_K2T)
  {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
  else if (ratio <= TSL2561_LUX_K3T)
  {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
  else if (ratio <= TSL2561_LUX_K4T)
  {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
  else if (ratio <= TSL2561_LUX_K5T)
  {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
  else if (ratio <= TSL2561_LUX_K6T)
  {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
  else if (ratio <= TSL2561_LUX_K7T)
  {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
  else if (ratio > TSL2561_LUX_K8T)
  {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}
  
  unsigned long temp;
  temp = ((channel0 * b) - (channel1 * m));
  
  // do not allow negative lux value
  // if (temp < 0) temp = 0;
  
  // round lsb (2^(LUX_SCALE-1))
  temp += (1 << (TSL2561_LUX_LUXSCALE-1));
  
  // strip off fractional portion
  uint32_t lux = temp >> TSL2561_LUX_LUXSCALE;
  
  // Signal I2C had no errors
  return lux;
}

uint32_t LuminosityTSL2561::getFullLuminosity ()
{
  // Enable the device by setting the control bit to 0x03
  enable();
  
  // Wait x ms for ADC to complete
  switch (m_integration)
  {
    case INTEGRATIONTIME_13MS:
      delay(14);
      break;
    case INTEGRATIONTIME_101MS:
      delay(102);
      break;
    default:
      delay(400);
      break;
  }
  
  uint32_t x;
  x = I2CBase::readReg16(m_DeviceHandle, TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
  x <<= 16;
  x |= I2CBase::readReg16(m_DeviceHandle, TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);
  
  disable();
  
  return x;
}
uint16_t LuminosityTSL2561::getLuminosity (Channel_t channel) {
  
  uint32_t x = getFullLuminosity();
  
  switch (channel) {
    case FULLSPECTRUM:
      return (x & 0xFFFF);
    case INFRARED:
      return (x >> 16);
    default: // == case VISIBLE:
      return ( (x & 0xFFFF) - (x >> 16));
  }
  
}


uint16_t LuminosityTSL2561::getLux()
{
  uint32_t lum = getFullLuminosity();

  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  return calculateLux(full, ir);
}


float LuminosityTSL2561::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 : return float(getLux());
    default : throw std::out_of_range("LuminosityTSL2561::getAnalog: index out of range");
  }
}

std::string LuminosityTSL2561::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Luminosity";
    default : throw std::out_of_range("LuminosityTSL2561::getAnalogChannelDesc: index out of range");
  }
}

std::string LuminosityTSL2561::getAnalogChannelUnit(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "Lux";
    default : throw std::out_of_range("LuminosityTSL2561::getAnalogChannelUnit: index out of range");
  }
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

 Derived from the tsl2561.cpp file by K. Townsend (microBuilder.eu)
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
