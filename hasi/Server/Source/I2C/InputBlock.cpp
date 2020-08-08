#include "InputBlock.h"
#include <stdexcept>
#include <sstream>

using namespace I2C;

InputBlock::InputBlock(uint8_t busID, uint8_t i2cAddress,
                       const std::string& devID, const std::string& hrname,
                       const HAS::HASConfigPtr config,
                       std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
}

void InputBlock::init() {
  I2CMember::init();
  for (size_t i = 0 ; i < 8; ++i) m_inputStates[i] = UNCHANGED;
  m_lastPoll = pollRawData();
  m_debouncePoll = m_lastPoll;
}

void InputBlock::pollDigitalIn() {
  uint8_t currentPoll = pollRawData();

  // debounce/denoise: reject any change that does not last two polls
  if (currentPoll != m_debouncePoll) {
    m_debouncePoll = currentPoll;
    currentPoll = m_lastPoll;
  }

  for (uint8_t bit = 0;bit<8;bit++) {
    uint8_t lastBit = (1<<bit) & m_lastPoll;
    uint8_t currentBit = (1<<bit) & currentPoll;
    if (lastBit != currentBit) {
      if (lastBit) {
        m_inputStates[bit] = SET;
      } else {
        m_inputStates[bit] = UNSET;
      }
    } else {
      m_inputStates[bit] = UNCHANGED;
    }
  }
  m_lastPoll = currentPoll;
}

BitVal InputBlock::getDigital(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalInChannelCount()-1))
    throw std::out_of_range("InputBlock::getDigital: index out of range");
  return BitVal(bitRead(m_lastPoll, iChannel));
}

BitState InputBlock::getDigitalState(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalInChannelCount()-1))
    throw std::out_of_range("InputBlock::getDigitalState: index out of range");
  return m_inputStates[iChannel];
}

uint8_t InputBlock::pollRawData() {
  activate();
  return ~uint8_t(I2CBase::read(m_DeviceHandle));
}

std::string InputBlock::getDesc() const {
  std::stringstream ss;
  ss << "ERE Opto Input, 30V, PCF8574(A) ( ";
  for (uint8_t bit = 0;bit<getDigitalInChannelCount();bit++) {
    ss << int(getDigital(bit)) << " ";
  }
  ss << ")";
  return ss.str();
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
