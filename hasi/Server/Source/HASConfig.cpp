#include <sstream>
#include <vector>

#include "HASConfig.h"
#include <Tools/DebugOutHandler.h>
#include <Script/ParserTools.h>

using namespace HAS;
using namespace IVDA;

HASConfig::HASConfig(const std::string& configFilename) :
m_strHASIFile("program.hasi"),
m_strLogFile("event.log"),
m_strStateSerialFile("states.save"),
m_strTimerSerialFile("timers.save"),
m_strStopWatchFile("stopWatches.save"),
m_strDeviceFile("devices.ini"),
m_strFromAddress(""),
m_strToAddress(""),
m_strSystemName("Home automation system"),
m_EventLoopDelay(20),
m_EventLoopDelayMode(dm_manual),
m_AutoEventLoopDelayMax(0.05f),
m_RestorePointInterval(60),
m_QueryCPUTemp(false),
m_ReportInvalidChanges(false),
m_ReportHTTPActivities(false),
m_ReportNetworkActivities(false),
m_HTTPPort(0),
m_strHTTPWhitelist(""),
m_RemotePort(0),
m_RemoteTimeout(30000),
m_MaxRemoteTimeoutsToKeepAlive(4),
m_RemoteConnectionInterval(10000),
m_RemoteUpdateSkip(25),
m_DisplayRemoteActivity(false),
m_TXTFilename(""),
m_HTMLFilename(""),
m_HTMLMsgCount(0),
m_RemotePassword(""),
m_NetworkTimeout(30000),
m_daemonDelay(0),
m_WatchdogInterval(0),
m_PrintScriptSystemErrors(true)
{
  readConfig(configFilename);
}


template <typename T>
bool HASConfig::getValueFromConfig(KeyValueFileParser& parser,
                                   const std::string& id,
                                   T& result,
                                   const T& defaultValue,
                                   const std::string& name) const {
  
  std::string strTemp;
  std::string strDef = SysTools::ToString(defaultValue);
  if (!getStringValueFromConfig(parser, id, strTemp, strDef, name)) {
    return false;
  } else {
    result = SysTools::FromString<T>(strTemp);
    return true;
  }
  
  
}

bool HASConfig::getStringValueFromConfig(KeyValueFileParser& parser,
                                         const std::string& id,
                                         std::string& result,
                                         const std::string& defaultValue,
                                         const std::string& name) const {
  
  std::vector<const KeyValPair*> fileEntry = parser.GetDataVec(id);

  if (fileEntry.size() == 0) {
    IVDA_WARNING("Entry for " << name << " (" << id << ") not found in config file. Using default value " << defaultValue);
    result = defaultValue;
    return false;
  }

  if (fileEntry.size() > 1) {
    IVDA_WARNING("Multiple entries for " << name << " (" << id << ") found in config file. Using first value " << fileEntry[0]->strValue);
  } else {
    IVDA_MESSAGE("Using " << fileEntry[0]->strValue << " as " << name;);
  }

  result = fileEntry[0]->strValue;
  return true;
}

void HASConfig::readConfig(const std::string& configFilename) {
  
  if (configFilename.empty() || !SysTools::FileExists(configFilename)) {
    IVDA_WARNING("Config file " << configFilename << " not found. Using default filenames and settings.");
    return;
  }
  
  KeyValueFileParser parser(configFilename, false, "=");
  if (parser.FileReadable()) {
    
    getStringValueFromConfig(parser, "HASIFile", m_strHASIFile, m_strHASIFile, "HASI program");
    getStringValueFromConfig(parser, "LogFile", m_strLogFile, m_strLogFile, "Event logfile");
    getStringValueFromConfig(parser, "StateSave", m_strStateSerialFile, m_strStateSerialFile, "state serialization file");
    getStringValueFromConfig(parser, "TimersSave", m_strTimerSerialFile, m_strTimerSerialFile, "timer serialization file");
    getStringValueFromConfig(parser, "StopWatchSave", m_strStopWatchFile, m_strStopWatchFile, "stopwatch serialization file");
    getStringValueFromConfig(parser, "DeviceFile", m_strDeviceFile, m_strDeviceFile, "device description file");
    getStringValueFromConfig(parser, "FromAddress", m_strFromAddress, m_strFromAddress, "report sender address");
    getStringValueFromConfig(parser, "FromName", m_strSystemName, m_strSystemName, "report sender name");
    getStringValueFromConfig(parser, "ToAddress", m_strToAddress, m_strToAddress, "report recipient address");
    getValueFromConfig(parser, "EventLoopDelay", m_EventLoopDelay, m_EventLoopDelay, "event loop delay");
    uint32_t temp;
    getValueFromConfig(parser, "EventLoopDelayMode", temp, uint32_t(m_EventLoopDelayMode), "how to determine the event loop delay, 0=based on user set delay, 1=based on the max load, 2=based on max overload");
    m_EventLoopDelayMode = EDelayMode(temp);
    getValueFromConfig(parser, "AutomaticEventLoopDelayMax", m_AutoEventLoopDelayMax, m_AutoEventLoopDelayMax, "the maximum target overload or load in automatic mode");
    getValueFromConfig(parser, "QueryCPUTemp", m_QueryCPUTemp, false, "read CPU temperature");
    getValueFromConfig(parser, "ReportInvalidChanges", m_ReportInvalidChanges, false, "report invalid changes");
    getValueFromConfig(parser, "ReportHTTPActivities", m_ReportHTTPActivities, false, "report HTTP activities");
    getValueFromConfig(parser, "ReportNetworkActivities", m_ReportNetworkActivities, false, "report network activities");
    getValueFromConfig(parser, "RestorePointInterval", m_RestorePointInterval, uint32_t(60), "restore point interval");
    getValueFromConfig(parser, "RemotePort", m_RemotePort, uint16_t(0), "port for HASI remotes");
    getValueFromConfig(parser, "HTTPPort", m_HTTPPort, uint16_t(0), "port for HTTP state server");
    getStringValueFromConfig(parser, "m_strHTTPWhitelist", m_strHTTPWhitelist, m_strHTTPWhitelist, "allow only specific values to be accessed via the http server, one such parameter per line in the whitelist file, empty filename disables this feature, default is empty filename");
    getValueFromConfig(parser, "RemoteTimeout", m_RemoteTimeout, uint32_t(30000), "timeout in ms for HASI remotes");
    getValueFromConfig(parser, "MaxRemoteTimeoutsToKeepAlive", m_MaxRemoteTimeoutsToKeepAlive, uint32_t(4), "max number of timeouts before a message is expected to keep the connection alive");
    getValueFromConfig(parser, "RemoteConnectionInterval", m_RemoteConnectionInterval, uint32_t(10000), "interval in ms to check for disconnected remotes");
    getValueFromConfig(parser, "RemoteUpdateSkip", m_RemoteUpdateSkip, uint32_t(25), "event loops to skip until remote update");
    getValueFromConfig(parser, "DisplayRemoteActivity", m_DisplayRemoteActivity, false, "report connect and disconnect events of remotes");
    getStringValueFromConfig(parser, "TXTLogfilename", m_TXTFilename, m_TXTFilename, "name of an text file to write the log into, leave empty to disable txt logging in interactive mode, in daemon mode logs are always written disk to haslog.txt if no other filename is provided");
    getStringValueFromConfig(parser, "HTMLLogfilename", m_HTMLFilename, m_HTMLFilename, "name of an HTML file to write the log into, leave empty to disable HTML logging");
    getValueFromConfig(parser, "HTMLLoglength", m_HTMLMsgCount, m_HTMLMsgCount, "how many log messages to store in the HTML file, leave empty to disable HTML logging");
    getStringValueFromConfig(parser, "RemotePassword", m_RemotePassword, m_RemotePassword, "password for remote controller connections");
    getValueFromConfig(parser, "NetworkTimeout", m_NetworkTimeout, m_NetworkTimeout, "timeout in ms for Network devices' TCP/IP calls");
    getValueFromConfig(parser, "DaemonDelay", m_daemonDelay, m_daemonDelay, "time in ms to wait before has starts in daemon mode");
    getValueFromConfig(parser, "WatchdogInterval", m_WatchdogInterval, m_WatchdogInterval, "specify the watchdoch interval in seconds, (kicking is executed after 60% of the specified interval), to use the system default interval set this to 0");
    
    
    getValueFromConfig(parser, "PrintScriptSystemErrors", m_PrintScriptSystemErrors, m_PrintScriptSystemErrors, "report when a system call in a script returns a non-zero value");
 
    
  } else {
    IVDA_WARNING("Config file " << configFilename << " not readable. Using default filenames and settings.");
  }
  
}


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

