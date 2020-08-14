#include "SysInfo.h"
#include <fstream>   // std::ifstream
#include <stdlib.h>  // atoi
#include <cmath>     // floor
#include <Script/ParserTools.h> // fromString
#include <Has.h> // getBaseVersion, getRevisionVersion
#include <Tools/SysTools.h> // trimString


#ifdef _MSC_VER

#include "pdh.h"
#include "TCHAR.h"

#pragma comment(lib,"pdh.lib")

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

static void initPdh(){
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

static double getCurrentPerformanceValue(){
    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

static int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif


using namespace HAS;

void getCPUInfo(std::string& hardwareID, std::string& revisionID) {
  hardwareID ="";
  revisionID="";

  std::ifstream cpuInfo("/proc/cpuinfo");
  if (cpuInfo.is_open()) {
    std::string line;

    // fill variables
    while (getline(cpuInfo, line)) {
      size_t cPos = line.find_first_of(':');
      if (cPos == std::string::npos) continue;
      std::string varName = IVDA::SysTools::TrimStr(line.substr(0,cPos));
      std::string varValue = IVDA::SysTools::TrimStr(line.substr(cPos+1));

      if (varName == "Hardware") {
        hardwareID = varValue;
      }
      if (varName == "Revision") {
        revisionID = varValue;
      }
    }
  }

}

EBoardID SysInfo::getBoardID() {
#ifdef _WIN32
#ifdef _WIN64
  return EB_WIN64;
#else
  return EB_WIN32;
#endif
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
  // iOS Simulator
  return EB_IOS_SIM;
#elif TARGET_OS_IPHONE
  // iOS device
  return EB_IOS;
#elif TARGET_OS_MAC
  // Other kinds of Mac OS
  return EB_MACOS;
#else
  return EB_UNKNOWN_APPLE;
#endif
#elif __linux__
  std::string hardwareID, revisionID;
  getCPUInfo(hardwareID, revisionID);

  // find out type
  if (hardwareID == "BCM2708" || hardwareID == "BCM2709" || hardwareID == "BCM2835" || hardwareID == "BCM2837") {
    if (revisionID == "0007" || revisionID == "0008" || revisionID == "0009")
      return EB_RASPBERRY_1A;
    else if (revisionID == "0002" || revisionID == "0003" || revisionID == "0004" || revisionID == "0005" ||
             revisionID == "0006" || revisionID == "000d" || revisionID == "000e" || revisionID == "000f")
      return EB_RASPBERRY_1B;
    else if (revisionID == "900021" || revisionID == "0012" || revisionID == "0015")
      return EB_RASPBERRY_1AP;
    else if (revisionID == "900032" || revisionID == "0010" || revisionID == "0013")
      return EB_RASPBERRY_1BP;
    else if (revisionID == "0011" || revisionID == "0014")
      return EB_RASPBERRY_CM1;
    else if (revisionID == "900092" || revisionID == "900093")
      return EB_RASPBERRY_ZERO;
    else if (revisionID == "9000c1")
      return EB_RASPBERRY_ZERO_W;
    else if (revisionID == "a01040" || revisionID == "a01041" || revisionID == "a01041" || revisionID == "a22042")
      return EB_RASPBERRY_2B;
    else if (revisionID == "a02082" || revisionID == "a22082" || revisionID == "a32082" || revisionID == "a52082")
      return EB_RASPBERRY_3B;
    else if (revisionID == "a020a0")
      return EB_RASPBERRY_CM3;
    else if (revisionID == "a020d3")
      return EB_RASPBERRY_3BP;
    else if (revisionID == "9020e0")
      return EB_RASPBERRY_3AP;
    else
     return EB_RASPBERRY_UNKNOWN;
  } else if (hardwareID == "sun7i")
    return EB_BANANAPI;
  else if (hardwareID == "ODROIDC")
    return EB_ODROIDC;
  else if (hardwareID == "ODROID-C2")
    return EB_ODROIDC2;
  else if (hardwareID == "ODROID-XU3")
    return EB_ODROIDXU3;
  else if (hardwareID == "Hardkernel ODROID-N2")
    return EB_ODROIDN2;
  else if (hardwareID == "ODROID-U2/U3")
    return EB_ODROIDU2U3;
  else if (hardwareID == "Allwinner sun6i (A31) Family")
     return EB_BANANAPI2;
  else if (hardwareID == "sun8i")
    return EB_BANANAPI3;
  else
    return EB_UNKNOWN_LINUX;

#elif __unix__ // all unices not caught above
  // Unix
  return EB_UNKNOWN_UNIX;
#elif defined(_POSIX_VERSION)
  // POSIX
  return EB_UNKNOWN_POSIX;
#else
  return EB_UNKNOWN;
#endif
}


std::string SysInfo::getBoardName() {
  std::string id = "";
  switch (getBoardID()) {
    case EB_BANANAPI:     id = "Banana Pi M1"; break;
    case EB_BANANAPI2:    id = "Banana Pi M2"; break;
    case EB_BANANAPI3:    id = "Banana Pi M3"; break;
    case EB_ODROIDC:      id = "Odroid C1"; break;
    case EB_ODROIDC2:     id = "Odroid C2"; break;
    case EB_ODROIDXU3:    id = "Odroid XU3/XU4"; break;
    case EB_ODROIDN2:     id = "Odroid N2"; break;
    case EB_ODROIDU2U3:   id = "Odroid U2/U3"; break;
      
    case EB_RASPBERRY_UNKNOWN:  id = "Unknown Raspberry Pi"; break;
    case EB_RASPBERRY_1A:       id = "Raspberry Pi 1A"; break;
    case EB_RASPBERRY_1B:       id = "Raspberry Pi 1B"; break;
    case EB_RASPBERRY_CM1:      id = "Raspberry Pi CM1"; break;
    case EB_RASPBERRY_1AP:      id = "Raspberry Pi 1A+"; break;
    case EB_RASPBERRY_1BP:      id = "Raspberry Pi 1B+"; break;
    case EB_RASPBERRY_ZERO:     id = "Raspberry Pi Zero"; break;
    case EB_RASPBERRY_ZERO_W:   id = "Raspberry Pi Zero W"; break;
    case EB_RASPBERRY_2B:       id = "Raspberry Pi 2B"; break;
    case EB_RASPBERRY_3B:       id = "Raspberry Pi 3B"; break;
    case EB_RASPBERRY_CM3:      id = "Raspberry Pi CM3"; break;
    case EB_RASPBERRY_3BP:      id = "Raspberry Pi 3B+"; break;
    case EB_RASPBERRY_3AP:      id = "Raspberry Pi 3A+"; break;
      
    case EB_WIN64:         return "Microsoft Windows 64bit";
    case EB_WIN32:         return "Microsoft Windows 32bit";
    case EB_IOS_SIM:       return "Apple iOS Simulator";
    case EB_IOS:           return "Apple iOS";
    case EB_MACOS:         return "Apple Mac OS";
    case EB_UNKNOWN_APPLE: return "unknown Apple platform";
    case EB_UNKNOWN_LINUX: id = "unknown Linux platform"; break;
    case EB_UNKNOWN_UNIX:  id = "unknown Unix platform"; break;
    case EB_UNKNOWN_POSIX: id = "unknown POSIX platform"; break;
    case EB_UNKNOWN:
    default :
      return "unknown platform"; break;
  }

  std::string hardwareID, revisionID;
  getCPUInfo(hardwareID, revisionID);
  return id + " (" + hardwareID + ", " + revisionID + ")";
}


uint16_t SysInfo::getCoreCount() {
#ifndef _MSC_VER
  uint16_t iProcCount = 0;
  std::ifstream cpuInfo("/proc/cpuinfo");

  if (cpuInfo.is_open()) {
    std::string line;
    while (getline(cpuInfo, line)) {
      line = ParserTools::removeSpaces(line);
      std::string tail;
      if (ParserTools::startsWith(line,"processor:", tail)) {
        iProcCount = std::max<uint16_t>(iProcCount, atoi(tail.c_str()));
      }
    }
  }
  return iProcCount+1;
#else
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return uint16_t(sysInfo.dwNumberOfProcessors);
#endif
}


SysInfo::SysInfo(const HASConfigPtr config, uint8_t iTempDecimals, uint32_t iScanIntervall) :
HASMember("CPU0", "The systems's internal data", config),
m_iTempDecimals(iTempDecimals),
m_iScanIntervall(iScanIntervall)
{
#ifdef _MSC_VER
  initPdh();
#endif

  std::fill (m_fLastMeasurement.begin(),m_fLastMeasurement.end(), 0.0f);

  switch (SysInfo::getBoardID()) {
    case EB_RASPBERRY_UNKNOWN:
    case EB_RASPBERRY_1A:
    case EB_RASPBERRY_1B:
    case EB_RASPBERRY_CM1:
    case EB_RASPBERRY_1AP:
    case EB_RASPBERRY_1BP:
    case EB_RASPBERRY_ZERO:
    case EB_RASPBERRY_ZERO_W:
    case EB_RASPBERRY_2B:
    case EB_RASPBERRY_3B:
    case EB_RASPBERRY_CM3:
    case EB_RASPBERRY_3BP:
    case EB_RASPBERRY_3AP:
      m_strPathToTempFile = "/sys/class/thermal/thermal_zone0/temp";
      break;
    case EB_BANANAPI :
      m_strPathToTempFile = "/sys/devices/platform/sunxi-i2c.0/i2c-0/0-0034/temp1_input";
      break;
    case EB_BANANAPI2 :
      m_strPathToTempFile = "/sys/class/thermal/thermal_zone0/temp";
      break;
    case EB_BANANAPI3 :
      m_strPathToTempFile = "/sys/class/thermal/thermal_zone0/temp";
      break;
    case EB_ODROIDC :
      m_strPathToTempFile = "/sys/devices/virtual/thermal/thermal_zone0/temp";
      break;
    case EB_ODROIDC2 :
      m_strPathToTempFile = "/sys/devices/virtual/thermal/thermal_zone0/temp";
      break;
    case EB_ODROIDXU3 :
      m_strPathToTempFile = "/sys/devices/virtual/thermal/thermal_zone0/temp";
      break;
    case EB_ODROIDN2 :
      m_strPathToTempFile = "/sys/devices/virtual/thermal/thermal_zone0/temp";
      break;
    case EB_ODROIDU2U3 :
      m_strPathToTempFile = "/sys/devices/virtual/thermal/thermal_zone0/temp"; // untested
      break;
    default :
      m_strPathToTempFile = "";
      break;
  }

  m_fLastMeasurement[4] = float(getCoreCount());
  m_fLastMeasurement[5] = float(getBoardID());
  
  m_startVal=std::chrono::system_clock::now();
}

uint8_t SysInfo::getAnalogInChannelCount() const {
  return uint8_t(m_fLastMeasurement.size());
}

uint32_t SysInfo::pollAnalogIn() {
  if (m_iScanIntervall > 0) {
    struct timeval current;
    long mtime, seconds, useconds;
    gettimeofday(&current, NULL);
    seconds  = current.tv_sec  - m_lastMeasurementTime.tv_sec;
    useconds = current.tv_usec - m_lastMeasurementTime.tv_usec;
    mtime = long(((seconds) * 1000 + useconds/1000.0) + 0.5);

    if (uint32_t(mtime) < m_iScanIntervall) {
      return 0;
    }
    m_lastMeasurementTime = current;
  }

  if (m_config->getQueryCPUTemp() && !m_strPathToTempFile.empty()) {
    // measure system temp
   std::ifstream tFile;
   tFile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

    std::string sTemp;
    try {
      tFile.open (m_strPathToTempFile);
      getline (tFile,sTemp);
      tFile.close();
    }
    catch (const std::ifstream::failure& e) {
      return 0;
    }

    int iTemp = ParserTools::fromString<int>(sTemp);
    m_fLastMeasurement[0] = iTemp/1000.0f;
    // round to iTempDecimals
    m_fLastMeasurement[0] *= m_iTempDecimals+1;
    m_fLastMeasurement[0] = floor(m_fLastMeasurement[0]+0.5f);
    m_fLastMeasurement[0] /= m_iTempDecimals+1;
  }

  // measure system load
#ifndef _MSC_VER
  double load[3];
  if (getloadavg(load, 3) != -1) {
    m_fLastMeasurement[1] = float(load[0]);
    m_fLastMeasurement[2] = float(load[1]);
    m_fLastMeasurement[3] = float(load[2]);
  } else {
    m_fLastMeasurement[1] = 0.0f;
    m_fLastMeasurement[2] = 0.0f;
    m_fLastMeasurement[3] = 0.0f;
  }
#else
    float d = float(getCurrentPerformanceValue());
    m_fLastMeasurement[1] = d;
    m_fLastMeasurement[2] = d;
    m_fLastMeasurement[3] = d;
#endif
  
  auto current=std::chrono::system_clock::now();
  auto waitedFor=current-m_startVal;
  auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(waitedFor).count();
  
  m_fLastMeasurement[6] = float(elapsedMillis / (60.0*1000.0) );
  
  m_fLastMeasurement[7] = float(Has::getBaseVersion());
  m_fLastMeasurement[8] = float(Has::getRevisionVersion());
  
  return 0;
}

float SysInfo::getAnalog(uint8_t iChannel) {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogInChannelCount()-1))
    throw std::out_of_range("SysInfo::getAnalog: index out of range");

  return m_fLastMeasurement[iChannel];
}

std::string SysInfo::getAnalogChannelDesc(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogInChannelCount()-1))
    throw std::out_of_range("IAnalogIn::getAnalogChannelDesc: index out of range");
  switch (iChannel) {
    case 0 : return "The system's core temperature";
    case 1 : return "The system's core load in the last minute";
    case 2 : return "The system's core load in the last 5 minutes";
    case 3 : return "The system's core load in the last 15 minutes";
    case 4 : return "The system's core count";
    case 5 : return "The system ID";
    case 6 : return "The system's uptime";
    case 7 : return "The has base version";
    case 8 : return "The has svn revision";
    default : return "unknown quantity";
  }
}

std::string SysInfo::getAnalogChannelUnit(uint8_t iChannel) const {
  if (!checkRange<uint8_t>(iChannel, 0, getAnalogInChannelCount()-1))
    throw std::out_of_range("IAnalogIn::getAnalogChannelUnit: index out of range");

  switch (iChannel) {
    case 0 : return "°C";
    case 1 : return "scheduler queue size/core";
    case 2 : return "scheduler queue size/core";
    case 3 : return "scheduler queue size/core";
    case 4 : return "";
    case 5 : return "";
    case 6 : return "minutes";
    case 7 : return "";
    case 8 : return "";
    default : return "unknown quantity";
  }
}

std::string SysInfo::getDesc() const {
  return "The systems's internal data";
}


/*
   The MIT License

   Copyright (c) 2013-2015 Jens Krueger

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
