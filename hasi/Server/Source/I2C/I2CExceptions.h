#pragma once

#ifndef I2CEXCEPTIONS_H
#define I2CEXCEPTIONS_H

#include "../HASExceptions.h"

namespace I2C {
  class EI2CBus : public HAS::EHASBus {
  public:
    explicit EI2CBus (const std::string& what_arg) :
    HAS::EHASBus(what_arg) {}
    explicit EI2CBus (const char* what_arg) :
    HAS::EHASBus(what_arg) {}
  };

  class EI2CDeviceInit : public HAS::EDeviceInit {
  public:
    explicit EI2CDeviceInit (const std::string& what_arg) :
    EDeviceInit(what_arg) {}
    explicit EI2CDeviceInit (const char* what_arg) :
    EDeviceInit(what_arg) {}
  };
}

#endif // I2CEXCEPTIONS_H

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
