#include "NetworkDevice.h"
#include <Tools/SysTools.h>
#include <HASExceptions.h>
#include <sstream>
#include <algorithm>

using namespace IVDA;
using namespace HAS;


NetworkDevice::NetworkDevice(const std::string& devID,
                             const std::string& hrname,
                             const HASConfigPtr config,
                             const std::string& password) :
HASMember(devID, hrname, config),
m_bConnectPulseSend(false),
m_iTimeout(config->getNetworkTimeout()),
m_password(password),
dataBegin("DATA BEGIN"),
dataEnd("DATA END"),
dataBeginNL(dataBegin + "\n"),
dataEndNL(dataEnd + "\n"),
testHeader("MESSAGE_OK")
{
}

std::string NetworkDevice::getDesc() const {
  std::stringstream ss;
  ss << "Unknown Network device (timeout: " << m_iTimeout << ")";
  return ss.str();
}

uint8_t NetworkDevice::getAnalogInChannelCount() const {
  return 2;
}

float NetworkDevice::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 :
      if (hasData()) {
        if (m_bConnectPulseSend) return 0;
        m_bConnectPulseSend = true;
        return 1;
      }
      return 0;
    default : return hasData() ? 1.0f : 0.0f;
  }
}

std::string NetworkDevice::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "just connected";
    default : return "is connected";
  }
}

std::string NetworkDevice::getAnalogChannelUnit(uint8_t iChannel) const {
  return "boolean";
}


void NetworkDevice::removeChar(std::string& str, char s) const {
  str.erase (std::remove(str.begin(), str.end(), s), str.end());
}

bool NetworkDevice::checkPostFix(std::string str, const std::string& pf) const {
  if (str.length() < pf.length()) return false;
  for (size_t i = 0;i<pf.length();++i) {
    if (pf[i] != str[i + str.length() - pf.length()]) return false;
  }
  
  return true;
}

std::string NetworkDevice::removeHeaderAndFooter(std::string str) const {
  if (str.length() >= dataBeginNL.length()+dataEndNL.length())
    return str.substr(dataBeginNL.length(), str.length() - (dataBeginNL.length()+dataEndNL.length()));
  else
    return "";
}

std::vector<std::string> NetworkDevice::stringToLineList(const std::string& str) const {
  std::vector<std::string> v;
  std::stringstream ss(str);
  std::string line;
  
  while(std::getline(ss,line)){
    removeChar(line, '\n');
    v.push_back(line);
  }
  
  return v;
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
