#include "AI418S.h"
#include <stdexcept>
#include <sstream>

#include "I2CBusManager.h"

using namespace I2C;

static uint32_t MEASURE_DELAY[4] = {5,17,70,270};


AI418S::AI418S(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID, const std::string& hrname,
               const HAS::HASConfigPtr config,
               std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
m_prec(PREC_12BIT),
m_gain(GAIN_1X),
m_bContConv(false),
m_fMultiplier(100.0),
m_fDecimalFact(1.0)
{
}

float AI418S::getAnalog(uint8_t iChannel) {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogInChannelCount()-1)) {
    std::stringstream ss;
    ss << "AI418S::getAnalog: channel index " << int(iChannel)
    << " out of range [0-" << int(getAnalogInChannelCount()-1) << "]";
    throw std::out_of_range( ss.str() );
  }

  sendConfigData(iChannel);
  return float(readValue());
}

void AI418S::sendConfigData(uint8_t iChannel) {  
  uint8_t command = 0x80 |  // trigger in "one-shot"-mode, ignored in continous mode
                    (uint8_t(m_gain) & 3) |
                    ((uint8_t(m_prec) & 3) << 2) |
                    ((uint8_t(m_bContConv) & 1) << 4) |
                    ((iChannel & 3) << 5);
  activate();
  I2CBase::write(m_DeviceHandle, command);
  delay(MEASURE_DELAY[int(m_prec)]);
}

double AI418S::getMaxData() const {
  switch (m_prec) {
    case PREC_12BIT:
      return 2047.0;
    case PREC_14BIT:
      return 8191.0;
    case PREC_16BIT:
      return 32767.0;
    default:
      return 131071.0;
  }
}

double AI418S::getGainValue() const {
  switch (m_gain) {
    case GAIN_1X:
      return 1.0;
    case GAIN_2X:
      return 2.0;
    case GAIN_4X:
      return 4.0;
    default:
      return 8.0;
  }
}


double AI418S::readValue() {
#ifndef NI2C
  activate();
  
  uint32_t data;
  ssize_t result;

  if (m_prec == PREC_18BIT) {
    unsigned char buffer [4] ;
    result = read(m_DeviceHandle, buffer, 4) ;
    if (result == 4)
       data = buffer[0]*65536+buffer[1]*256+buffer[2];
    else
       data = 0;
  } else {
    unsigned char buffer [3] ;
    result = read(m_DeviceHandle, buffer, 3) ;
    if (result == 3)
      data = buffer[0]*256+buffer[1];
    else
       data = 0;
  }

  double sensorValue = (double(data)/getMaxData())*(double(2.048)/getGainValue()) * (double(180)/double(33));
  double rescaledValue = floor(m_fMultiplier*sensorValue*m_fDecimalFact+0.5)/m_fDecimalFact;

  return rescaledValue;
#else
  return 0.0f;
#endif
}

std::string AI418S::getDesc() const {
  return "ERE I2C Bus Voltage and Current Analog Input Board I2C-AI418S";
}

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
