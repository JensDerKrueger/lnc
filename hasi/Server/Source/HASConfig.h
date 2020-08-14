#pragma once

#ifndef HASCONFIG_H
#define HASCONFIG_H

#include <string>
#include "HASBasics.h"
#include "Tools/KeyValueFileParser.h"
#include "Tools/SysTools.h"

namespace HAS {

  class HASConfig {
  public:
    
    enum EDelayMode {
      dm_manual = 0,
      dm_basedOnLoad = 1,
      dm_basedOnOverload = 2
    };
    
    HASConfig(const std::string& configFilename);
    
    const std::string& getHASIFile() const {
      return m_strHASIFile;
    }
    const std::string& getLogFile() const {
      return m_strLogFile;
    }
    const std::string& getStateSerialFile() const {
      return m_strStateSerialFile;
    }
    const std::string& getTimerSerialFile() const {
      return m_strTimerSerialFile;
    }
    const std::string& getStopWatchFile() const {
      return m_strStopWatchFile;
    }
    const std::string& getDeviceFile() const {
      return m_strDeviceFile;
    }
    const std::string& getFromAddress() const {
      return m_strFromAddress;
    }
    const std::string& getToAddress() const {
      return m_strToAddress;
    }
    const std::string& getSystemName() const {
      return m_strSystemName;
    }
    void setEventLoopDelay(uint32_t eventLoopDelay) {
      m_EventLoopDelay = eventLoopDelay;
    }
    uint32_t getEventLoopDelay() const {
      return m_EventLoopDelay;
    }
    EDelayMode getEventLoopDelayMode() const {
      return m_EventLoopDelayMode;
    }
    float getAutoEventLoopDelayMax() const {
      return m_AutoEventLoopDelayMax;
    }
    uint32_t getRestorePointInterval() const {
      return m_RestorePointInterval;
    }
    bool getQueryCPUTemp() const {
      return m_QueryCPUTemp;
    }
    bool getReportInvalidChanges() const {
      return m_ReportInvalidChanges;
    }
    bool getReportNetworkActivities() const {
      return m_ReportNetworkActivities;
    }
    bool getReportHTTPActivities() const {
      return m_ReportHTTPActivities;
    }
    uint16_t getHTTPPort() const {
      return m_HTTPPort;
    }
    const std::string& getHTTPWhitelist() const {
      return m_strHTTPWhitelist;
    }
    const std::string& getCORSAllowOrigin() const {
      return m_strCORSAllowOrigin;
    }
    uint16_t getRemotePort() const {
      return m_RemotePort;
    }
    uint32_t getRemoteTimeout() const {
      return m_RemoteTimeout;
    }
    uint32_t getMaxRemoteTimeoutsToKeepAlive() const {
      return m_MaxRemoteTimeoutsToKeepAlive;
    }
    uint32_t getRemoteConnectionInterval() const {
      return m_RemoteConnectionInterval;
    }
    uint32_t getRemoteUpdateSkip() const {
      return m_RemoteUpdateSkip;
    }
    uint32_t getDisplayRemoteActivity() const {
      return m_DisplayRemoteActivity;
    }
    const std::string& getTXTLogLFilename() const {
      return m_TXTFilename;
    }
    const std::string& getHTMLLogFilename() const {
      return m_HTMLFilename;
    }
    uint32_t getHTMLLogMsgCount() const {
      return m_HTMLMsgCount;
    }
    const std::string& getRemotePassword() const {
      return m_RemotePassword;
    }    
    const uint32_t getNetworkTimeout() const {
      return m_NetworkTimeout;
    }
    const uint32_t getDaemonDelay() const {
      return m_daemonDelay;
    }
    const int32_t getWatchdogInterval() const {
      return m_WatchdogInterval;
    }
    bool getPrintScriptSystemErrors() const {
      return m_PrintScriptSystemErrors;
    }
    
    
  private:
    std::string m_strHASIFile;
    std::string m_strLogFile;
    std::string m_strStateSerialFile;
    std::string m_strTimerSerialFile;
    std::string m_strStopWatchFile;
    std::string m_strDeviceFile;
    std::string m_strFromAddress;
    std::string m_strToAddress;
    std::string m_strSystemName;
    uint32_t m_EventLoopDelay;
    EDelayMode m_EventLoopDelayMode;
    float m_AutoEventLoopDelayMax;
    uint32_t m_RestorePointInterval;
    bool m_QueryCPUTemp;
    bool m_ReportInvalidChanges;
    bool m_ReportHTTPActivities;
    bool m_ReportNetworkActivities;
    uint16_t m_HTTPPort;
    std::string m_strHTTPWhitelist;
    std::string m_strCORSAllowOrigin;
    uint16_t m_RemotePort;
    uint32_t m_RemoteTimeout;
    uint32_t m_MaxRemoteTimeoutsToKeepAlive;
    uint32_t m_RemoteConnectionInterval;
    uint32_t m_RemoteUpdateSkip;
    bool m_DisplayRemoteActivity;
    std::string m_TXTFilename;
    std::string m_HTMLFilename;
    uint32_t m_HTMLMsgCount;
    std::string m_RemotePassword;
    uint32_t m_NetworkTimeout;
    uint32_t m_daemonDelay;
    int32_t m_WatchdogInterval;
    bool m_PrintScriptSystemErrors;

    void readConfig(const std::string& configFilename);
    
    template <typename T>
    bool getValueFromConfig(KeyValueFileParser& parser,
                            const std::string& id,
                            T& result,
                            const T& defaultValue,
                            const std::string& name) const;
    
    bool getStringValueFromConfig(KeyValueFileParser& parser,
                                  const std::string& id,
                                  std::string& result,
                                  const std::string& defaultValue,
                                  const std::string& name) const;
  };
  
  typedef std::shared_ptr<HASConfig> HASConfigPtr;
  
}

#endif // HASCONFIG_H

/*
 The MIT License
 
 Copyright (c) 2014-2018 Jens Krueger
 
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
