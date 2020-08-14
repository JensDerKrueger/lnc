#pragma once

#ifndef WEBSERVICEBUS_H
#define WEBSERVICEBUS_H

#include <memory>
#include <map>

#include <HASBasics.h>
#include <HASConfig.h>

#include "WebServiceDevice.h"
#include "WeatherWebServiceDevice.h"
#include <Tools/Threads.h>


#include "../Tools/KeyValueFileParser.h"

namespace HAS {
  class WebServiceData {
  public:
    WebServiceData() {}
    ~WebServiceData();
    
    void init();
    std::map<const std::string, WebServiceDevice*> m_vpAllDevices;
    
    std::vector<WeatherWebServiceDevice*> m_vpWSDevices;
  };
  
  class WebServiceBus {
  public:
    WebServiceBus();
    ~WebServiceBus();
    void ParseDevices(HASConfigPtr config);
    void init();
    std::string toString() const;
    WebServiceDevice* getDevice(const std::string& strID) const;
    
  private:
    std::shared_ptr<WebServiceData> m_BusData;
    
    template <class T> size_t addDevices(KeyValueFileParser& parser,
                                         std::shared_ptr<WebServiceData>& busData,
                                         std::vector<T*>& vec,
                                         const std::string& strID);

    IVDA::CriticalSection m_BusDataGuard;
    std::shared_ptr<IVDA::LambdaThread> m_updateThread;
    HASConfigPtr m_config;

    void updateFunc(IVDA::Predicate pContinue,
                    IVDA::LambdaThread::Interface& threadInterface);
    
  };
}

#endif // WEBSERVICEBUS_H

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

