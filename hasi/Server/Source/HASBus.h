#pragma once

#ifndef HASBUS_H
#define HASBUS_H

#include <I2C/I2CBus.h>
#include <Network/NetworkBus.h>
#include <WebServices/WebServiceBus.h>
#include "other-devices/SysInfo.h"

#include <sstream>
#include <vector>
#include <map>
#include <memory>

#include "HASBasics.h"
#include "HASMember.h"
#include "HASConfig.h"
#include "Tools/KeyValueFileParser.h"
#include "Tools/SysTools.h"


namespace HAS {
  class HASBus {
  public:
    HASBus();
    ~HASBus();
    void parseDevices(HASConfigPtr config);
    void init();
    std::string toString() const;
    HASMember* getDevice(const std::string& strID) const;
    
  protected:    
    I2C::I2CBus m_i2cBus;
    NetworkBus m_networkBus;
    WebServiceBus m_webServiceBus;
    
    std::shared_ptr<SysInfo> m_system;

  };
}

#endif // HASBUS_H

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

