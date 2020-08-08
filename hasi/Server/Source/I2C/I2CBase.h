#pragma once

#ifndef I2CBASE_H
#define I2CBASE_H

#ifndef NI2C
  #include <unistd.h>
#endif

#include "I2CExceptions.h"

#include "HASBasics.h"

using namespace BitManip;

namespace I2CBase {
  int read(int fd) ;
  
  int readReg8(int fd, int reg);
  int readReg16(int fd, int reg);
  int readBlock(int fd, int reg, unsigned char *data);

  int write(int fd, int data);
  int writeReg8(int fd, int reg, int data);
  int writeReg16(int fd, int reg, int data);
  int writeBlock(int fd, int reg, unsigned int length, unsigned char *data);

  int setupBus(int devId, uint8_t busId);
}
#endif // I2CBASE_H

/*
 Copyright (c) 2014 Jens Krueger
 based heavily (mostly copy and paste) on Gordon Henderson's wiringPiI2C
 
 due to Gordon's code being LGPL this code is also LGPL
 
 this is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.
 
 wiringPi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this file.
 If not, see <http://www.gnu.org/licenses/>.
 */
