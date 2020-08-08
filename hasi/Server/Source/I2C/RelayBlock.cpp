#include "RelayBlock.h"
#include <stdexcept>
#include <sstream>

#include <Tools/DebugOutHandler.h> // for IVDA_WARNING

using namespace I2C;


RelayBlock::RelayBlock(uint8_t busID, uint8_t i2cAddress,
                       const std::string& devID, const std::string& hrname,
                       const HAS::HASConfigPtr config,
                       std::shared_ptr<I2CBusManager> busManager,
                       bool bNoExternalChange) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager),
m_bNoExternalChange(bNoExternalChange)
{
  m_bDigitalOutNeedsUpdate = true;
}

void RelayBlock::init() {
  I2CMember::init();
  for (size_t i = 0 ; i < 8; ++i) m_inputStates[i] = UNCHANGED;
  m_BlockData = pollRawData();
}

void RelayBlock::setDigital(uint8_t iChannel, BitVal value) {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalOutChannelCount()-1))
    throw std::out_of_range("RelayBlock::setDigital: index out of range");
  m_BlockData = bitWrite(m_BlockData, iChannel, value);
  m_bDigitalOutNeedsUpdate = true;
}


void RelayBlock::applyDigitalOut() {
  activate();
  
  I2CBase::write(m_DeviceHandle, ~m_BlockData);
  m_bDigitalOutNeedsUpdate = false;

  if (m_bNoExternalChange) {
    int iTimeout = 10;
    uint8_t currentPoll=0;
    do {
      currentPoll = pollRawData();
      if (currentPoll != m_BlockData) {
        if (m_config && m_config->getReportInvalidChanges()) {
          IVDA_WARNING("Relay block transfer failed to block "
                      << m_devID<< ", resending.");
        }
        activate();
        I2CBase::write(m_DeviceHandle, ~m_BlockData);
      }
      iTimeout--;
    } while (currentPoll != m_BlockData && iTimeout > 0);

    if (iTimeout == 0) {
      IVDA_WARNING("Relay block transfer failed ten times in a row block "
                 << m_devID<< ", giving up for now.");
    }
  
  }
}

std::string RelayBlock::getDesc() const {
  std::stringstream ss;
  ss << "ERE High Current Relay, 12v, PCF8574(A) ( ";
  for (uint8_t bit = 0;bit<getDigitalInChannelCount();bit++) {
    ss << int(getDigital(bit)) << " ";
  }
  ss << ")";
  return ss.str();
}

void RelayBlock::pollDigitalIn() {
  uint8_t currentPoll = pollRawData();

  if (m_bNoExternalChange) {
    if (currentPoll != m_BlockData) {
      if (m_config && m_config->getReportInvalidChanges()) {
        IVDA_WARNING("Relay block data (" << int(currentPoll)
                     << ") differs from internal data ("
                     << int(m_BlockData) << ") for " << m_devID
                     << ", fixing relay block.");
      }
      applyDigitalOut();
    }
    return;
  }

  for (uint8_t bit = 0;bit<8;bit++) {
    uint8_t lastBit = (1<<bit) & m_BlockData;
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
  m_BlockData = currentPoll;
}

BitVal RelayBlock::getDigital(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalInChannelCount()-1))
    throw std::out_of_range("RelayBlock::getDigital: index out of range");
  return BitVal(bitRead(m_BlockData, iChannel));
}

BitState RelayBlock::getDigitalState(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getDigitalInChannelCount()-1))
    throw std::out_of_range("RelayBlock::getDigitalState: index out of range");
  return m_inputStates[iChannel];
}

uint8_t RelayBlock::pollRawData() {
  activate();
  return ~uint8_t(I2CBase::read(m_DeviceHandle));
}

/*
 The MIT License
 
 Copyright (c) 2013-2015 Jens Krueger
 
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
