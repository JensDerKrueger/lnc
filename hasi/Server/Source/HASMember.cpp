#include "HASMember.h"
#include <sstream>   // std::stringstream

using namespace HAS;

HASMember::HASMember(const std::string& devID, const std::string& hrname,
                     const HASConfigPtr config) :
m_devID(devID),
m_hrname(hrname),
m_config(config),
m_isActive(true)
{
}

HASMember::~HASMember() {
}

void HASMember::init() {
}

std::string HASMember::toString() const {
  std::stringstream ss;
  ss << getDesc() << " \"" << m_hrname << "\" ID=" << m_devID
     << " (" << (getIsActive() ? "active" : "inactive") << ")";
  return ss.str();
}

std::string HASMember::getDesc() const {
  return "Unknown HAS device";
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
