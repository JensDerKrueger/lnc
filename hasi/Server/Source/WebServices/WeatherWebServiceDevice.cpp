#include "WeatherWebServiceDevice.h"
#include <sstream>
#include <ctime>
#include <Tools/SysTools.h>
#include <Tools/DebugOutHandler.h>  // IVDA_WARNING


using namespace IVDA;
using namespace HAS;

#define VALUE_COUNT 4

WeatherWebServiceDevice* WeatherWebServiceDevice::deviceFromStrings(const std::vector<std::string>& entries,
                                                                    const HASConfigPtr config) {
  if (entries.size() != 8) {
    return nullptr;
  }
  return new WeatherWebServiceDevice(entries[0], entries[1], config,
                                     SysTools::FromString<double>(entries[2]),
                                     SysTools::FromString<double>(entries[3]),
                                     entries[4],
                                     SysTools::FromString<uint32_t>(entries[5]),
                                     SysTools::FromString<uint32_t>(entries[6]),
                                     SysTools::FromString<uint64_t>(entries[7]));
}


WeatherWebServiceDevice::WeatherWebServiceDevice(const std::string& devID,
                                                 const std::string& hrname,
                                                 const HASConfigPtr config,
                                                 double longitude,
                                                 double latitude,
                                                 const std::string& apikey,
                                                 uint32_t intervalLength,
                                                 uint32_t intervalCount,
                                                 uint64_t iRefreshMinutes) :
WebServiceDevice(devID, hrname, config, "", "", iRefreshMinutes),
m_intervalLength(intervalLength),
m_intervalCount(intervalCount),
m_forecast(longitude, latitude, apikey)
{
  m_server = m_forecast.getServer();
  m_URL = m_forecast.getURL();
  
  m_forecastData.resize(m_intervalCount*VALUE_COUNT);
  for (size_t i = 0;i<m_forecastData.size();++i) {
    m_forecastData[i] = 0.0f;
  }
}

std::string WeatherWebServiceDevice::getDesc() const {
  std::stringstream ss;
  ss << "Weather forecast webservice device, delivering " << m_intervalCount
     << " forecasts in " << m_intervalLength << " minute intervals. Data ";


  const uint64_t dataAge = getDataAge();

  if (dataAge == std::numeric_limits<uint64_t>::max()) {
    ss << "has not been acquired yet.";
  } else {
    const uint64_t iMinutes = getDataAge()/60;
  
    if (iMinutes == 1)
      ss << "is one minute old.";
    else
      ss << "is " << iMinutes << " minutes old.";
  }
  
  return ss.str();
}

uint8_t WeatherWebServiceDevice::getAnalogInChannelCount() const {
  return 1+m_intervalCount*VALUE_COUNT;
}

float WeatherWebServiceDevice::getAnalog(uint8_t iChannel) {
  if (iChannel == 0)
    return WebServiceDevice::getAnalog(0);


  if (iChannel >= getAnalogInChannelCount())
    throw std::out_of_range("WeatherWebServiceDevice::getAnalog:"
                            " index out of range");
  
  return m_forecastData[iChannel-1];
}

std::string WeatherWebServiceDevice::getAnalogChannelDesc(uint8_t iChannel) const {
  if (iChannel == 0)
    return WebServiceDevice::getAnalogChannelDesc(iChannel);
  
  iChannel--;
  
  std::stringstream ss;
  switch (iChannel%VALUE_COUNT) {
    case 0 : ss << "Temperature"; break;
    case 1 : ss << "Rain"; break;
    case 2 : ss << "Humidity"; break;
    default : ss << "Cloud density"; break;
  }
  ss << " in " << (m_intervalLength*iChannel/VALUE_COUNT) << " min.";
  return ss.str();
}

std::string WeatherWebServiceDevice::getAnalogChannelUnit(uint8_t iChannel) const {
  if (iChannel == 0)
    return WebServiceDevice::getAnalogChannelDesc(iChannel);
  
  iChannel--;
  
  switch (iChannel%VALUE_COUNT) {
    case 0 : return "deg. C ";
    case 1 : return "l/h";
    default : return "%"; // for both "humidity" & "cloud density"
  }
}

bool WeatherWebServiceDevice::reloadData()  {
  try {
    m_forecast.update();
  } catch (const WeatherForecastException& e) {
    IVDA_WARNING("Error receiving weather data: " << e.what());
    return false;
  }

  for (size_t i = 0;i<m_intervalCount;++i) {
    const WeatherData w = m_forecast.get(i*60*m_intervalLength);
    
    m_forecastData[i*VALUE_COUNT+0] = float(w.m_temp);
    m_forecastData[i*VALUE_COUNT+1] = float(w.m_rain);
    m_forecastData[i*VALUE_COUNT+2] = float(w.m_humid);
    m_forecastData[i*VALUE_COUNT+3] = w.m_clouds;
  }
  return true;
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
