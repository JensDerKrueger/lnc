#pragma once

#ifndef NETWORKBUS_H
#define NETWORKBUS_H

#include "NetworkInputServer.h"
#include "NetworkInputOutputServer.h"
#include "NetworkInputOutputClient.h"
#include "HarmonyHubDevice.h"
#include <memory>
#include <map>

#include <HASConfig.h>

#include <Tools/KeyValueFileParser.h>

namespace HAS {
  class NetworkData {
  public:
    NetworkData() {}
    ~NetworkData();
    
    void init();
    std::map<const std::string, NetworkDevice*> m_vpAllDevices;
    
    std::vector<HarmonyHubDevice*> m_vpHarmonyHubDevice;
    std::vector<NetworkInputServer*> m_vpInputServers;
    std::vector<NetworkInputOutputServer*> m_vpInputOutputServers;
    std::vector<NetworkInputOutputClient*> m_vpInputOutputClients;
  };
  
  class NetworkBus {
  public:
    NetworkBus();
    ~NetworkBus();
    void ParseDevices(HASConfigPtr config);
    void init();
    std::string toString() const;
    NetworkDevice* getDevice(const std::string& strID) const;
    
  protected:
    std::shared_ptr<NetworkData> m_BusData;
    
    template <class T> size_t addDevices(KeyValueFileParser& parser,
                                         HASConfigPtr config,
                                         std::shared_ptr<NetworkData>& busData,
                                         std::vector<T*>& vec,
                                         const std::string& strID);
  };
}

#endif // NETWORKBUS_H

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

