#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "HTTPRequest.h"
#include "WeatherForecast.h"
#include "JSON.h"

WeatherForecast::WeatherForecast(double latitude, double longitude,
                                 const std::string& apikey) :
m_longitude(longitude),
m_latitude(latitude),
m_apikey(apikey)
{
}

void WeatherForecast::setPosition(double latitude, double longitude) {
  m_longitude = longitude;
  m_latitude = latitude;
  update();
}

std::string WeatherForecast::getServer() const {
  return "api.openweathermap.org";
}

std::string WeatherForecast::getURL() const {
  std::stringstream ss;
  ss.precision(15);
  ss << "/data/2.5/forecast?lat=" << m_latitude << "&lon=" << m_longitude << "&appid=" << m_apikey;
  return ss.str();
}

void WeatherForecast::update() {
  HTTPRequest r(getServer(), getURL());

  std::string result = "";
  bool ok = r.send(result);

  if (!ok) {
    throw WeatherForecastException("Unable to load data from server");
  }
  
  JSONObjectPtr o = nullptr;
  try {
    o = std::make_shared<JSONObject>(result);
  } catch (const JSONParseException& e) {
    throw WeatherForecastException(std::string("Error parsing JSON data: ") +
                                   std::string(e.what()));
  }

  JSONValuePtr listField = o->getValue("list");
  if (listField == nullptr || listField->valueType != JSONValue::JVT_ARRAY) {
    throw WeatherForecastException(std::string("Array field \"list\" "
                                               "not found in JSON data."));
  }
  
  
  {
    SCOPEDLOCK(m_DataGuard);
    m_data.clear();
    
    const std::vector<JSONValuePtr>& list = listField->aValue;
    for (auto elem : list) {
      WeatherData d;
      if (elem->valueType == JSONValue::JVT_OBJ) {
        const JSONObjectPtr elemObj = elem->oValue;
        
        JSONValuePtr time = elemObj->getValue("dt");
        if (time == nullptr || time->valueType != JSONValue::JVT_INT) {
          throw WeatherForecastException(std::string("Time field "
                                                     "not found in JSON data."));
        }
        
        d.m_time = time->iValue;
        
        JSONValuePtr mainField = elemObj->getValue("main");
        
        if (mainField && mainField->valueType == JSONValue::JVT_OBJ) {
          const JSONObjectPtr mainObj = mainField->oValue;
          const JSONValuePtr temp = mainObj->getValue("temp");
          const JSONValuePtr humidity = mainObj->getValue("humidity");
          const JSONValuePtr clouds = elemObj->getValue("clouds");
          const JSONValuePtr rain = elemObj->getValue("rain");
          
          if (temp == nullptr) {
            throw WeatherForecastException(std::string("Temp field not "
                                                       "found in JSON data."));
          }
          if (humidity == nullptr) {
            throw WeatherForecastException(std::string("Humidity field not "
                                                       "found in JSON data."));
          }
          if (rain == nullptr) {
            d.m_rain = 0;
          } else {
            const JSONObjectPtr rainObj = rain->oValue;
            const JSONValuePtr precipitation = rainObj->getValue("3h");
            if (precipitation == nullptr) {
              d.m_rain = 0;
            } else {
              if (precipitation->valueType == JSONValue::JVT_FLOAT) {
                d.m_rain = precipitation->fValue/3.0;
              } else {
                if (precipitation->valueType == JSONValue::JVT_INT) {
                  d.m_rain = precipitation->iValue/3.0;
                } else {
                  d.m_rain = 0;
                }
              }
            }
          }
          
          if (clouds == nullptr) {
            throw WeatherForecastException(std::string("Clouds field not "
                                                       "found in JSON data."));
          }
          const JSONObjectPtr cloudsObj = clouds->oValue;
          const JSONValuePtr cloudDensity = cloudsObj->getValue("all");
          if (cloudDensity == nullptr) {
            throw WeatherForecastException(std::string("All field not "
                                                       "found in JSON data."));
          }
          
          if (temp->valueType == JSONValue::JVT_FLOAT) {
            d.m_temp = temp->fValue-273.15;
          } else {
            if (temp->valueType == JSONValue::JVT_INT) {
              d.m_temp = double(temp->iValue)-273.15;
            } else {
              throw WeatherForecastException(std::string("Temp field has wrong  "
                                                         "format in JSON data."));
            }
          }
          
          if (humidity->valueType == JSONValue::JVT_FLOAT) {
            d.m_humid = humidity->fValue;
          } else {
            if (humidity->valueType == JSONValue::JVT_INT) {
              d.m_humid = humidity->iValue;
            } else {
              throw WeatherForecastException(std::string("humidity field has"
                                                         " wrong  "
                                                         "format in JSON data."));
            }
          }
          
          if (cloudDensity->valueType == JSONValue::JVT_FLOAT) {
            d.m_clouds = uint8_t(cloudDensity->fValue);
          } else {
            if (cloudDensity->valueType == JSONValue::JVT_INT) {
              d.m_clouds = uint8_t(cloudDensity->iValue);
            } else {
              throw WeatherForecastException(std::string("cloudDensity field has"
                                                         " wrong  "
                                                         "format in JSON data."));
            }
          }
          
        } else {
          throw WeatherForecastException(std::string("Main field "
                                                     "not found in JSON data."));
        }
        
      }
      m_data.push_back(d);
    }
  }
}

WeatherData WeatherForecast::get(int64_t iSeconds) const {
  SCOPEDLOCK(m_DataGuard);

  WeatherData d;

  size_t nextLargerEntry = m_data.size();

  for (size_t i = 0;i<m_data.size();++i) {
    if (m_data[i].getTimeDifference() >  iSeconds) {
      nextLargerEntry = i;
      break;
    }
  }

  if (nextLargerEntry < m_data.size()) {
    if (nextLargerEntry > 0) {
      // "normal" case, i.e., entry is between two known entries
      const WeatherData& d1 = m_data[nextLargerEntry-1];
      const WeatherData& d2 = m_data[nextLargerEntry];
      d.interpolate(d1,d2,iSeconds);
    } else {
      // special case: time is smaller than first value
      const WeatherData& d1 = m_data[0];
      const WeatherData& d2 = m_data[1];
      d.interpolate(d1,d2,iSeconds);
    }
  } else {
    // special case: time is bigger than last value
    const WeatherData& d1 = m_data[m_data.size()-2];
    const WeatherData& d2 = m_data[m_data.size()-1];
    d.interpolate(d1,d2,iSeconds);
  }

  return d;
}

std::string WeatherForecast::toString() const {
  SCOPEDLOCK(m_DataGuard);

  std::stringstream ss;
  for (size_t i = 0;i<m_data.size();++i) {
    ss << m_data[i].toString() << std::endl;
  }
  return ss.str();
}

template <typename T>
static T clamp(T val, T minVal, T maxVal) {
  return std::min<T>(maxVal, std::max<T>(minVal, val));  
}

void WeatherData::interpolate(const WeatherData& d1, WeatherData d2,
                              int64_t t) {
  int64_t td1 = d1.getTimeDifference();
  int64_t td2 = d2.getTimeDifference();

  double alpha = double(t-td1) / double(td2-td1);
  
  m_time = int64_t(alpha * d2.m_time + (1.0-alpha) * d1.m_time);
  m_temp = alpha * d2.m_temp + (1.0-alpha) * d1.m_temp;
  m_humid = alpha * d2.m_humid + (1.0-alpha) * d1.m_humid;
  m_clouds = uint8_t(alpha * d2.m_clouds + (1.0-alpha) * d1.m_clouds);
  m_rain = alpha * d2.m_rain + (1.0-alpha) * d1.m_rain;

  m_humid = clamp<double>(m_humid,0.0,100.0);
  m_clouds = clamp<uint8_t>(m_clouds,0,100);
}


int64_t WeatherData::getTimeDifference() const {
  time_t currentTime = time(NULL);
  return int64_t((m_time+1.5*60*60) - currentTime); // service returns 3 hour intervals shift to center 
}

std::string WeatherData::toString() const {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << "Time offset: "
     << std::setfill(' ') << std::setw(4) << getTimeDifference()/60/60
     << " h, Temperature: " << m_temp << "°C, Humidity: " << m_humid
     << " %, Cloud coverage: " << std::setfill(' ') << std::setw(2)
     << int(m_clouds) << " %, Rain: " << m_rain << " l/h";
  return ss.str();
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
