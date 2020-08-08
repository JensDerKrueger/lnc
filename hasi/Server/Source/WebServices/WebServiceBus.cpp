#include "WebServiceBus.h"

#include <string>       // std::string
#include <sstream>      // std::stringstream

#include <HASExceptions.h>          // EDeviceParser, EDeviceNotFound
#include <Tools/DebugOutHandler.h>  // IVDA_WARNING
#include <Tools/SysTools.h> // FileExists

#include <ctime>

using namespace IVDA;
using namespace HAS;

WebServiceData::~WebServiceData() {
  for (auto it = m_vpAllDevices.begin(); it != m_vpAllDevices.end(); ++it) {
    delete it->second;
  }
}

void WebServiceData::init() {
  for (auto it = m_vpAllDevices.begin(); it != m_vpAllDevices.end(); ++it) {
    it->second->init();
  }
}

WebServiceBus::WebServiceBus() :
m_BusData(nullptr),
m_updateThread(nullptr),
m_config(nullptr)
{
}

WebServiceBus::~WebServiceBus() {
  IVDA_MESSAGE("Shutting down the WebService Bus");
  
  if (m_updateThread) {
    m_updateThread->RequestThreadStop();

    // there is a 10 second delay in the thread (and also in the HTTP-request)
    // so wait 15 seconds to be on the save side (only one of the two above can
    // cause the delay
    uint32_t shutdownTimeout = 1000*15;

    if (!m_updateThread->JoinThread(shutdownTimeout) && m_updateThread->IsRunning()) {
      IVDA_WARNING("updateThread join has timed out, killing thread.");
      m_updateThread->KillThread();
    }
  }

  m_BusData = nullptr;
  IVDA_MESSAGE("WebService Bus is down");
}


void WebServiceBus::updateFunc(IVDA::Predicate pContinue,
                               IVDA::LambdaThread::Interface& threadInterface) {
  uint8_t iCounter = 6;
  while (pContinue()) {

    // once a minute -> ask services if they want to update
    if (iCounter >= 6) {
      SCOPEDLOCK(m_BusDataGuard);
      if (m_BusData) {
        time_t tCurrentTime = time(NULL);
        uint64_t iCurrentTime = tCurrentTime;
        
        for (auto it = m_BusData->m_vpAllDevices.cbegin();
             it != m_BusData->m_vpAllDevices.cend(); ++it) {
          it->second->updateData(iCurrentTime);
        }
      }
      iCounter = 0;
    }
    
    // now sleep for 10 seonds
    if (pContinue()) delay(10*1000);

    iCounter++;
  }
}

void WebServiceBus::init() {

  if (m_BusData) {
    SCOPEDLOCK(m_BusDataGuard);
    m_BusData->init();
  }
  
  if (m_BusData) {
    if(!m_BusData->m_vpAllDevices.empty() && !m_updateThread) {
      m_updateThread =  std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&WebServiceBus::updateFunc, this, std::placeholders::_1, std::placeholders::_2)));
      m_updateThread->StartThread();
      
      IVDA_MESSAGE("WebService bus is up (Webservice update thread running)");
    } else {
      IVDA_MESSAGE("WebService bus is up (No webservices registered, web service update thread NOT running)");
    }
  } else {
    IVDA_WARNING("Unable to start WebService bus, no bus data specified.");
  }
  
  
}

void WebServiceBus::ParseDevices(HASConfigPtr config) {
  m_config = config;
  const std::string& filename = config->getDeviceFile();
  
  std::shared_ptr<WebServiceData> busData = std::make_shared<WebServiceData>();
  if (!SysTools::FileExists(filename)) {
    std::stringstream ss;
    ss << "Device configuration file (" << filename
       << ") not found, skipping webservice device registration";
    IVDA_MESSAGE(ss.str());

    {
      SCOPEDLOCK(m_BusDataGuard);
      m_BusData = busData;
    }
    return;
  }

  KeyValueFileParser parser(filename, false, "=");
  if (parser.FileReadable()) {

    // parse Weather forecast service devices
    addDevices(parser, busData, busData->m_vpWSDevices, "WeatherService");

    // iff all ok, copy new bus data into class
    {
      SCOPEDLOCK(m_BusDataGuard);
      m_BusData = busData;
    }
  } else {
    throw EDeviceParser(std::string("Unable to parse input file ") + filename);
  }
}

std::string WebServiceBus::toString() const {
  std::stringstream ss;
  if (m_BusData) {
    for (auto it = m_BusData->m_vpAllDevices.cbegin();
         it != m_BusData->m_vpAllDevices.cend(); ++it) {
      ss << it->second->toString() << std::endl;
    }
  }
  return ss.str();
}

WebServiceDevice* WebServiceBus::getDevice(const std::string& strID) const {
  if (m_BusData) {
    auto iter = m_BusData->m_vpAllDevices.find(strID);
    if (iter != m_BusData->m_vpAllDevices.end())
      return m_BusData->m_vpAllDevices.find(strID)->second;
    else
      throw EDeviceNotFound(strID);
  } else {
    throw EDeviceNotFound(strID);
  }
}

template <class T> size_t WebServiceBus::addDevices(KeyValueFileParser& parser,
                                                    std::shared_ptr<WebServiceData>& busData,
                                                    std::vector<T*>& vec,
                                                    const std::string& strID) {
  
  
  std::vector<const KeyValPair*> b = parser.GetDataVec(strID);
  size_t iNewDev = 0;
  for (auto wsDevice = b.begin();wsDevice!=b.end();++wsDevice) {

    T* inDevice = T::deviceFromStrings((*wsDevice)->vstrValue, m_config);

    if (!inDevice) {
      std::stringstream ss;
      ss << "Invalid WebService Device entry ID=" << strID << " '" << (*wsDevice)->strValue << "' detected ";
      throw EDeviceParser(ss.str());
    }
    
    vec.push_back(inDevice);
    iNewDev++;
    busData->m_vpAllDevices.insert(make_pair(inDevice->getID(),inDevice));
  }
  return iNewDev;
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

