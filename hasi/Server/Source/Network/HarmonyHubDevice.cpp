#include "HarmonyHubDevice.h"
#include <Tools/SysTools.h>
#include <HASExceptions.h>
#include <sstream>
#include <algorithm>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include "HarmonyHubControl.h"

using namespace IVDA;
using namespace HAS;

HarmonyHubDevice::HarmonyHubDevice(const std::string& devID, const std::string& hrname,
                                   HASConfigPtr config, const std::string& strURI,
                                   const std::string& strEmail, const std::string& strPassword,
                                   uint32_t iRefreshIntervall) :
NetworkDevice(devID, hrname, config, strPassword),
m_strURI(strURI),
m_currentActivity(0),
m_ActivityChange(false),
m_targetActivity(0),
m_strEmail(strEmail),
m_bConnectionUninitialized(true),
m_bHasData(false),
m_iRefreshIntervall(iRefreshIntervall)
{
}

HarmonyHubDevice::~HarmonyHubDevice() {
  RequestThreadStop();

  if (!JoinThread(m_iTimeout) && IsRunning()) {
    IVDA_WARNING("Shutdown of Harmony Hub " << m_hrname << " timed out, killing thread");
    KillThread();
  }
}


void HarmonyHubDevice::prepareShutdown() {
  RequestThreadStop();
}

std::string HarmonyHubDevice::getDesc() const {
  std::stringstream ss;
  ss << "Harmony hub "<< (isConnected() ? "connected" : "DISCONNECTED") << " at " << m_strURI << " Current State: " << m_currentActivity;
  return ss.str();
}

void HarmonyHubDevice::init() {
  if (m_config && m_config->getReportNetworkActivities()) {
    IVDA_MESSAGE(m_hrname << " (" << m_devID << ") init");
  }
    
  this->StartThread();
}

bool HarmonyHubDevice::isConnected() const {
  return !m_bConnectionUninitialized;
}

bool HarmonyHubDevice::hasData() const {
  return m_bHasData;
}

void HarmonyHubDevice::ThreadMain(void*) {
  std::string extAuth = "";
  std::string intAuth = "";
  HarmonyHubControl hhc(m_iTimeout/30, m_strURI);
  m_currentActivity = -2;
  
  while (Continue()) {
    try {
      m_bConnectionUninitialized = true;
      m_bHasData = false;
      
      // if we cannot connect to the hub, don't even start
      while(!hhc.checkHarmonyConnection()) {
        if (!Continue()) break;
        Suspend();
      }
      if (!Continue()) break;
      
      if (extAuth.empty()) {
        try {
          ExtHarmonyAuth externalAuth(m_iTimeout/30, m_strEmail, m_password);
          extAuth = externalAuth.getToken();
          
        } catch (const HarmonyException& e) {
/*
          if (m_config && m_config->getReportNetworkActivities())
            IVDA_WARNING("Unable to get external Harmony Hub authorization: "
                         << m_hrname << " (" << e.what() << ")");
 */
          continue;
        }
      }
      
      if (!Continue()) break;
      
      hhc.setExternalAuth(extAuth);
      
      if (intAuth.empty()) {
        try {
          intAuth = hhc.requestInternalAuthorization();
        } catch (const HarmonyException& e) {
          /*
           if (m_config && m_config->getReportNetworkActivities())
            IVDA_WARNING("Unable to get internal Harmony Hub authorization for "
                         << m_hrname << " (" << e.what() << ")");
          */
          continue;
        }
      }
      
      if (!Continue()) break;
      
      hhc.setInternalAuth(intAuth);
      m_bConnectionUninitialized = false;
      
      try {
        m_currentActivity = hhc.getActivity();
      } catch (const HarmonyException& e) {
        intAuth = "";
        extAuth = "";
/*
        if (m_config && m_config->getReportNetworkActivities())
          IVDA_WARNING("Unable to get initial activity for Harmony Hub " << m_hrname
                       << " (" << e.what() << ") resetting communication.");
 */
        continue;
      }
      m_bHasData = true;
      if (!Continue()) break;
      
      try {
        while (Continue()) {
          m_lastStart = std::chrono::high_resolution_clock::now();
          m_currentActivity = hhc.getActivity();
          if (m_ActivityChange && m_currentActivity != m_targetActivity) {
            hhc.setActivity(m_targetActivity);
          }
          m_ActivityChange = false;
          
          if (!Continue()) break;
          Suspend();
        }
      } catch (const HarmonyException&) {
        intAuth = "";
        extAuth = "";
        m_bHasData = false;
        continue;
      }
    } catch (const std::exception &ex) {
      IVDA_ERROR("Caught unexpected exception in HarmonyHubDevice " << ex.what());
      continue;
    } catch (...) {
      IVDA_ERROR("Caught unknown exception in HarmonyHubDevice.");
    }
  }
}

uint8_t HarmonyHubDevice::getAnalogInChannelCount() const {
  return 1+NetworkDevice::getAnalogInChannelCount();
}

uint32_t HarmonyHubDevice::pollAnalogIn() {

  // check the state of the harmony no more often than once every
  // 10 seconds
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed =std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStart);
  if (elapsed.count() >= m_iRefreshIntervall)
    Resume();
  
  return 0;
}

float HarmonyHubDevice::getAnalog(uint8_t iChannel) {
  if (iChannel >= getAnalogInChannelCount()) {
    std::stringstream ss;
    ss << "HarmonyHubDevice::getAnalog: channel index " << int(iChannel)
    << " out of range [0-" << int(getAnalogInChannelCount()-1) << "]";
    throw std::out_of_range( ss.str() );
  }

  float value = 0.0f;
  if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
    value = NetworkDevice::getAnalog(iChannel);
  } else {
    value = (float)m_currentActivity;
  }
  return value;
}

std::string HarmonyHubDevice::getAnalogChannelDesc(uint8_t iChannel) const {
  if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
    return NetworkDevice::getAnalogChannelDesc(iChannel);
  } else {
    return "Current Activity";
  }
}

std::string HarmonyHubDevice::getAnalogChannelUnit(uint8_t iChannel) const {
  if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
    return NetworkDevice::getAnalogChannelUnit(iChannel);
  } else {
    return "";
  }
}


// IAnalogOut Interface
uint8_t HarmonyHubDevice::getAnalogOutChannelCount() const {
  return 1;
}

void HarmonyHubDevice::setAnalog(uint8_t iChannel, float value) {
  if (iChannel != 0) {
    std::stringstream ss;
    ss << "HarmonyHubDevice::setAnalog: channel index " << int(iChannel)
    << " out of range [0-" << int(getAnalogOutChannelCount()-1) << "]";
    throw std::out_of_range( ss.str() );
  }

  m_targetActivity = int32_t(value);
  m_ActivityChange = true;
  Resume();
}

void HarmonyHubDevice::applyAnalogOut() {
}

HarmonyHubDevice* HarmonyHubDevice::deviceFromStrings(const std::vector<std::string>& entries, HASConfigPtr config) {
  if (entries.size() != 6) {
    return nullptr;
  }

  return new HarmonyHubDevice(entries[0], entries[1], config,
                              entries[2], entries[3], entries[4],
                              SysTools::FromString<uint32_t>(entries[5]));
}


/*
 The MIT License
 
 Copyright (c) 2016 Jens Krueger
 
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
