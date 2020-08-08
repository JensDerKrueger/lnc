#pragma once

#ifndef HASBASICS_H
#define HASBASICS_H

#include <algorithm>  // std::min, std::max

void delay(int msec);

template<typename T> static T clamp(T val, T minVal, T maxVal) {
  return std::min(maxVal, std::max(minVal, val));
}

template<typename T> static T checkRange(T val, T minVal, T maxVal) {
  return (val == clamp(val, minVal, maxVal));
}


namespace BitManip {
  enum BitVal {
    OFF = 0,
    ON = 1
  };

  enum BitState {
    UNSET = 0,
    SET = 1,
    UNCHANGED = 2
  };

  template<typename T> T bitRead(T value, T bit) {
    return ((value) >> (bit)) & 0x01;
  }

  template<typename T> T bitSet(T value, T bit) {
    return (value) |= (1UL << (bit));
  }

  template<typename T> T bitClear(T value, T bit) {
    return (value) &= ~(1UL << (bit));
  }

  template<typename T> T bitWrite(T value, T bit, BitVal bitvalue) {
    return bitvalue ? bitSet(value, bit) : bitClear(value, bit);
  }

};

#endif // HASBASICS_H

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

