#ifndef SYSINFO_H
#define SYSINFO_H

#include <HASMember.h>
#include <HASConfig.h>

#include <IAnalogIn.h>
#include <array>        // std::array
#include <string>       // std::string
#ifndef _MSC_VER
  #include <sys/time.h>   // timeval
#endif

#include <chrono>       // time_point, system_clock


namespace HAS {

  enum EBoardID {
    EB_UNKNOWN       = 0,
    EB_BANANAPI      = 1,  // sun7i
    EB_ODROIDC       = 2,  // ODROIDC
    EB_ODROIDU2U3    = 3,  // ODROIDU2U3
    EB_BANANAPI2     = 4,  // Allwinner sun6i (A31) Family
    EB_BANANAPI3     = 5,  // sun8i
    EB_ODROIDC2      = 6, // ODROIDC-2
    EB_ODROIDXU3     = 7, // ODROID-XU3
    EB_ODROIDN2      = 8, // ODROID-N2
    
    // BCM2708 & BCM2709 & BCM2837
    EB_RASPBERRY_UNKNOWN   = 101,
    EB_RASPBERRY_1A        = 102,
    EB_RASPBERRY_1B        = 103,
    EB_RASPBERRY_CM1       = 104,
    EB_RASPBERRY_1AP       = 105,
    EB_RASPBERRY_1BP       = 106,
    EB_RASPBERRY_ZERO      = 107,
    EB_RASPBERRY_ZERO_W    = 108,
    EB_RASPBERRY_2B        = 109,
    EB_RASPBERRY_3B        = 110,
    EB_RASPBERRY_CM3       = 111,
    EB_RASPBERRY_3BP       = 112,
    EB_RASPBERRY_3AP       = 113,
    
    EB_WIN64        = 1000,
    EB_WIN32        = 1001,
    EB_IOS_SIM      = 1002,
    EB_IOS          = 1003,
    EB_MACOS        = 1004,
    EB_UNKNOWN_APPLE= 1005,
    EB_UNKNOWN_LINUX= 1006,
    EB_UNKNOWN_UNIX = 1007,
    EB_UNKNOWN_POSIX= 1008
    
  };

  class SysInfo : public HASMember, public IAnalogIn {
  public:
    SysInfo(const HASConfigPtr config, uint8_t iTempDecimals=1, uint32_t iScanIntervall=30000);
    
    // IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual uint32_t pollAnalogIn();
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    
    static EBoardID getBoardID();
    static std::string getBoardName();
    static uint16_t getCoreCount();
    
  private:
    uint8_t m_iTempDecimals;
    uint32_t m_iScanIntervall;
    std::string m_strPathToTempFile;
    
    std::array<float,9> m_fLastMeasurement;
    struct timeval m_lastMeasurementTime;
    
    std::chrono::time_point<std::chrono::system_clock> m_startVal;

  protected:
    // HASMember interface
    virtual std::string getDesc() const;
  };

}

#endif // SYSINFO_H


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
