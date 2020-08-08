#include "NetworkBus.h"

#include <string>       // std::string
#include <sstream>      // std::stringstream

#include <HASExceptions.h>          // EDeviceParser, EDeviceNotFound
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include <Tools/SysTools.h> // FileExists

using namespace IVDA;
using namespace HAS;

NetworkData::~NetworkData() {
  const size_t count = m_vpAllDevices.size();
  size_t counter = 1;

  for (auto it = m_vpAllDevices.begin(); it != m_vpAllDevices.end(); ++it) {
    IVDA_MESSAGE(" notifying " << counter << " of " << count << " (" << it->first << ")");
    it->second->prepareShutdown();
    counter++;
  }

  counter = 1;
  for (auto it = m_vpAllDevices.begin(); it != m_vpAllDevices.end(); ++it) {
    IVDA_MESSAGE(" waiting for " << counter << " of " << count << " (" << it->first << ")");
    delete it->second;
    counter++;
  }
}

void NetworkData::init() {
  for (auto it = m_vpAllDevices.begin(); it != m_vpAllDevices.end(); ++it) {
    it->second->init();
  }
}

NetworkBus::NetworkBus() :
m_BusData(nullptr)
{
}

NetworkBus::~NetworkBus() {
  IVDA_MESSAGE("Shutting down Network Bus");
  m_BusData = nullptr;
  IVDA_MESSAGE("Network Bus is down");
}

void NetworkBus::init() {
  if (m_BusData) {
    m_BusData->init();
  }
  IVDA_MESSAGE("Network Bus is up");
}

void NetworkBus::ParseDevices(HASConfigPtr config) {
  const std::string& filename = config->getDeviceFile();
  
  std::shared_ptr<NetworkData> busData(new NetworkData);
  if (!SysTools::FileExists(filename)) {
    std::stringstream ss;
    ss << "Network device configuration file (" << filename
       << ") not found, skipping network device registration";
    IVDA_MESSAGE(ss.str());
    m_BusData = busData;
    return;
  }

  KeyValueFileParser parser(filename, false, "=");
  if (parser.FileReadable()) {

    // parse Harmony Hub devices
    addDevices(parser, config, busData, busData->m_vpHarmonyHubDevice, "HarmonyHub");

    // parse Network i devices
    addDevices(parser, config, busData, busData->m_vpInputServers, "NetworkInputServer");
    
    // parse Network io devices
    addDevices(parser, config, busData, busData->m_vpInputOutputServers, "NetworkInputOutputServer");
    
    // parse Network io clients
    addDevices(parser, config, busData, busData->m_vpInputOutputClients, "NetworkInputOutputClient");

    // iff all ok, copy new bus data into class
    m_BusData = busData;
  } else {
    throw EDeviceParser(std::string("Unable to parse input file ") + filename);
  }
}

std::string NetworkBus::toString() const {
  std::stringstream ss;
  if (m_BusData) {
    for (auto it = m_BusData->m_vpAllDevices.cbegin();
         it != m_BusData->m_vpAllDevices.cend(); ++it) {
      ss << it->second->toString() << std::endl << std::endl;
    }
  }
  return ss.str();
}

NetworkDevice* NetworkBus::getDevice(const std::string& strID) const {
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

template <class T> size_t NetworkBus::addDevices(KeyValueFileParser& parser,
                                                 HASConfigPtr config,
                                                 std::shared_ptr<NetworkData>& busData,
                                                 std::vector<T*>& vec,
                                                 const std::string& strID) {
  
  
  std::vector<const KeyValPair*> b = parser.GetDataVec(strID);
  size_t iNewDev = 0;
  for (auto inputDevice = b.begin();inputDevice!=b.end();++inputDevice) {

    T* inDevice = T::deviceFromStrings((*inputDevice)->vstrValue, config);

    if (!inDevice) {
      std::stringstream ss;
      ss << "Invalid Network Device entry ID=" << strID << " '" << (*inputDevice)->strValue << "' detected ";
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

