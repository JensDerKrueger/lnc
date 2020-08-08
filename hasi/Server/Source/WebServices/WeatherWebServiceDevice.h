#ifndef WEATHERWEBSERVICEDEVICE_H
#define WEATHERWEBSERVICEDEVICE_H

#include <HASMember.h>
#include "WebServiceDevice.h"
#include "WeatherForecast.h"
#include <string>    // std::string
#include <vector>    // std::vector

namespace HAS {
  
  class WeatherWebServiceDevice : public WebServiceDevice {
  public:
    static WeatherWebServiceDevice* deviceFromStrings(const std::vector<std::string>& entries,
                                                      const HASConfigPtr config);
    
    WeatherWebServiceDevice(const std::string& devID, const std::string& hrname,
                            const HASConfigPtr config,
                            double longitude, double latitude,
                            const std::string& apikey,
                            uint32_t intervalLength = 60,
                            uint32_t intervalCount = 12,
                            uint64_t iRefreshMinutes=60);
    
    // IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    virtual uint32_t pollAnalogIn() {return 0;}
    
  protected:
    virtual std::string getDesc() const;
    virtual bool reloadData();

  private:
    uint32_t m_intervalLength;
    uint32_t m_intervalCount;
    WeatherForecast m_forecast;
    std::vector<float> m_forecastData;
    
  };

}

#endif // WEATHERWEBSERVICEDEVICE_H


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
