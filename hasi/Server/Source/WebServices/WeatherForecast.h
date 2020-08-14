#ifndef WEATHERFORECAST_H
#define WEATHERFORECAST_H

#include <string>
#include <vector>
#include <stdexcept>
#include <Tools/Threads.h>  // CriticalSection

class WeatherForecastException : public std::exception {
public:
  WeatherForecastException(const std::string& m) : msg(m) {}
  WeatherForecastException() {}
  virtual ~WeatherForecastException() throw() {}
  const char* what() const throw() { return msg.c_str(); }
private:
  std::string msg;
};

class WeatherData {
public:
  int64_t getTimeDifference() const;
  void interpolate(const WeatherData& d1, WeatherData d2,
                   int64_t t);

  int64_t m_time;
  double m_temp;
  double m_humid;
  double m_rain;
  uint8_t m_clouds;

  std::string toString() const;
};

class WeatherForecast  {
public:
  WeatherForecast(double longitude, double latitude, const std::string& apikey);

  void setPosition(double longitude, double latitude);
  void update();

  WeatherData get(int64_t iSeconds) const;
  
  std::string toString() const;

  std::string getServer() const;
  std::string getURL() const;

protected:
  std::vector<WeatherData> m_data;
  double m_longitude;
  double m_latitude;
  std::string m_apikey;
  
  mutable IVDA::CriticalSection m_DataGuard;

};

#endif // WEATHERFORECAST_H


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
