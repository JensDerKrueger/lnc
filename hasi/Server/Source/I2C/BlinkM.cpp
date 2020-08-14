#include "BlinkM.h"

using namespace I2C;

BlinkM::BlinkM(uint8_t busID, uint8_t i2cAddress,
               const std::string& devID, const std::string& hrname,
               const HAS::HASConfigPtr config,
               std::shared_ptr<I2CBusManager> busManager) :
I2CMember(busID, i2cAddress, devID, hrname, config, busManager) {
}

void BlinkM::init() {
  I2CMember::init();
  WriteCommand('o', 0); //End the current Light script
  ChangeTo(0.0f, 0.0f, 0.0f);
}

void BlinkM::ChangeTo(uint8_t r, uint8_t g, uint8_t b) {
  WriteCommand('n', // change to color
               3,
               int(r), //Red component
               int(g), //Green component
               int(b)); //Blue component
}

void BlinkM::ChangeTo(float r, float g, float b) {
  ChangeTo(uint8_t(r*255), uint8_t(g*255), uint8_t(b*255));
}

void BlinkM::FadeTo(uint8_t r, uint8_t g, uint8_t b) {
  WriteCommand('c', // fade to color
               3,
               int(r), //Red component
               int(g), //Green component
               int(b)); //Blue component
}

void BlinkM::FadeTo(float r, float g, float b) {
  FadeTo(uint8_t(r*255), uint8_t(g*255), uint8_t(b*255));
}

void BlinkM::GetColor(uint8_t& r, uint8_t& g, uint8_t& b) {
  WriteCommand('g', 0); // get color
  r = I2CBase::read(m_DeviceHandle);
  g = I2CBase::read(m_DeviceHandle);
  b = I2CBase::read(m_DeviceHandle);
}

void BlinkM::ChangeI2CAddress(uint8_t newAddress) {
  WriteCommand('A', 4, int(newAddress), int(0xd0), int(0xd0), int(newAddress));
  m_i2cAddress = newAddress;
}

void BlinkM::WriteCommand(unsigned char cmd, size_t count, ...) {
  va_list vl;
  va_start(vl,count);
  
  activate();
  I2CBase::write(m_DeviceHandle, cmd);
  for (size_t i=0;i<count;i++)
  {
    uint8_t val=va_arg(vl,int);
    I2CBase::write(m_DeviceHandle, uint8_t(val));
  }

  va_end(vl);
}

std::string BlinkM::getDesc() const {
  return "BlinkM smart LED";
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
