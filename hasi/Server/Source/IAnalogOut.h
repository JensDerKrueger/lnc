#pragma once

#ifndef ANALOGOUT_H
#define ANALOGOUT_H

// analog-out interface
class IAnalogOut {
public:
  virtual uint8_t getAnalogOutChannelCount() const = 0;
  virtual void setAnalog(uint8_t iChannel, float value) = 0;
  virtual void applyAnalogOut() = 0;
  virtual bool analogOutNeedsUpdate() const {return m_bAnalogOutNeedsUpdate;}
  
protected:
  bool m_bAnalogOutNeedsUpdate;

};

#endif // ANALOGOUT_H

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

