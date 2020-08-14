#include "WebServiceDevice.h"
#include <sstream>
#include <ctime>
#include <limits>

using namespace HAS;

WebServiceDevice::WebServiceDevice(const std::string& devID,
                                   const std::string& hrname,
                                   const HASConfigPtr config,
                                   const std::string& server,
                                   const std::string& URL,
                                   uint64_t iRefreshMinutes) :
HASMember(devID, hrname, config),
m_server(server),
m_URL(URL),
m_iRefreshMinutes(iRefreshMinutes),
m_iLastRefresh(0)
{
}

std::string WebServiceDevice::getDesc() const {
  std::stringstream ss;
  ss << "Unknown Webservice device (interval: " << m_iRefreshMinutes << " min.)";
  return ss.str();
}

uint8_t WebServiceDevice::getAnalogInChannelCount() const {
  return 1; // every WebServiceDevice has at least one channel
}

float WebServiceDevice::getAnalog(uint8_t iChannel) {
  switch (iChannel) {
    case 0 :
      return m_iLastRefresh ? 1.0f : 0.0f;
    default :
      return 0.0f;
  }
}

std::string WebServiceDevice::getAnalogChannelDesc(uint8_t iChannel) const {
  switch (iChannel) {
    case 0 : return "data received";
    default : return "unknown";
  }
}

std::string WebServiceDevice::getAnalogChannelUnit(uint8_t iChannel) const {
  return "boolean";
}

bool WebServiceDevice::updateData(uint64_t iCurrentTime) {
  if (m_iLastRefresh + m_iRefreshMinutes*60 < iCurrentTime) {
    if (reloadData()) {
      m_iLastRefresh = iCurrentTime;
      return true;
    }
  }
  return false;
}

uint64_t WebServiceDevice::getDataAge() const {
  if (m_iLastRefresh == 0)
    return std::numeric_limits<uint64_t>::max();
  
  uint64_t iCurrentTime = uint64_t(time(NULL));
  return iCurrentTime-m_iLastRefresh;
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
