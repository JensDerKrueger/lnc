#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory>

#ifndef NI2C
  #include <sys/ioctl.h>
  #ifdef USE_SYSTEM_I2C_HEADER
    #include <linux/i2c-dev.h>
  #else
    #include "I2C/i2c-dev.h"
  #endif
#endif

#include "I2CBase.h"

int I2CBase::read (int fd) {
#ifndef NI2C
  return i2c_smbus_read_byte (fd) ;
#else
  return 0;
#endif
}

int I2CBase::readReg8 (int fd, int reg) {
#ifndef NI2C
  return i2c_smbus_read_byte_data(fd, reg);
#else
  return 0;
#endif
}

int I2CBase::readReg16 (int fd, int reg) {
#ifndef NI2C
  return i2c_smbus_read_word_data(fd, reg);
#else
  return 0;
#endif
}

int I2CBase::readBlock (int fd, int reg, unsigned char *data) {
#ifndef NI2C
  return i2c_smbus_read_block_data(fd, reg, data);
#else
  return 0;
#endif
}

int I2CBase::write (int fd, int data) {
#ifndef NI2C
  return i2c_smbus_write_byte(fd, data);
#else
  return 0;
#endif
}

int I2CBase::writeReg8 (int fd, int reg, int data) {
#ifndef NI2C
  return i2c_smbus_write_byte_data(fd, reg, data);
#else
  return 0;
#endif
}

int I2CBase::writeReg16 (int fd, int reg, int data) {
#ifndef NI2C
  return i2c_smbus_write_word_data(fd, reg, data);
#else
  return 0;
#endif
}

int I2CBase::writeBlock (int fd, int reg, unsigned int length, unsigned char *data){
#ifndef NI2C
  return i2c_smbus_write_block_data(fd, reg, length, data);
#else
  return 0;
#endif
}

int I2CBase::setupBus (int devId, uint8_t busId) {
#ifndef NI2C
  int fd ;
  char device[64];
  sprintf(device, "/dev/i2c-%d", busId);
  
  if ((fd = open (device, O_RDWR)) < 0) return -1;
  if (ioctl (fd, I2C_SLAVE, devId) < 0) return -1;
  
  return fd ;
#else
  return 0;
#endif
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
