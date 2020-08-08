#include "HASBus.h"
#include "HASExceptions.h"

#include <string>       // std::string
#include <Tools/DebugOutHandler.h> // for IVDA_MESSAGE

using namespace HAS;
using namespace IVDA;

HASBus::HASBus() :
m_system(nullptr)
{
}

HASBus::~HASBus() {
  IVDA_MESSAGE("HAS Bus is going down");
}

void HASBus::init() {
  IVDA_MESSAGE("Initializing I2C Bus");
  m_i2cBus.init();

  IVDA_MESSAGE("Initializing Network Bus");
  m_networkBus.init();

  IVDA_MESSAGE("Initializing Webservice Bus");
  m_webServiceBus.init();

  IVDA_MESSAGE("All HAS Buses are up");
}

void HASBus::parseDevices(HASConfigPtr config) {
  m_i2cBus.ParseDevices(config);
  m_networkBus.ParseDevices(config);
  m_webServiceBus.ParseDevices(config);

  m_system = std::make_shared<SysInfo>(config);
}

std::string HASBus::toString() const {
  return m_i2cBus.toString() + std::string("\n") +
         m_networkBus.toString() + std::string("\n") +
         m_webServiceBus.toString();
}

HASMember* HASBus::getDevice(const std::string& strID) const {
  if (strID == m_system->getID())
    return dynamic_cast<HASMember*>(m_system.get());
  
  try {
    return m_i2cBus.getDevice(strID);
  } catch (const EDeviceNotFound& ) {
    try {
      return m_networkBus.getDevice(strID);
    } catch (const EDeviceNotFound& ) {
      return m_webServiceBus.getDevice(strID);
    }
  }
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

