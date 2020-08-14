#pragma once

#ifndef FILEPLOTTER_H
#define FILEPLOTTER_H

#include <string>
#include <sstream>
#include <vector>
#include <ctime>

struct FileInfo {
  std::string name;
  std::string desc;
  double min;
  double max;
};

struct PlotterEntry {
  std::string desc;
  std::string value;
  std::string unit;
};

class FilePlotter {
public:
  enum IntervalType {
    IT_Minutely,
    IT_Hourly,
    IT_Daily,
    IT_Weekly,
    IT_Monthly,
    IT_Yearly,
    IT_Never,
  };

  FilePlotter(const std::string& strFilename);
  virtual ~FilePlotter() {}

  void beginLog();
  template <typename T> void log(const std::string& desc, T value, const std::string& unit="") {
    PlotterEntry p;
    std::stringstream ss;
    ss << value;

    p.desc = desc;
    p.value = ss.str();
    p.unit = unit;

    m_values.push_back(p);
  }
  virtual bool endLog();

  virtual std::string toString() const;

  std::vector<FileInfo> plotImage(const std::string& strFilename, unsigned int w, unsigned int h) const;
  static std::vector<FileInfo> plotImage(const std::string& strSourceFilename, const std::string& strFilename, unsigned int w, unsigned int h);

  static void logLine(tm *nun, std::ofstream& logFile, std::vector<PlotterEntry>& values);
  static void logLine(std::ofstream& logFile, std::vector<PlotterEntry>& values);
protected:
  std::vector<std::string> m_strFilenames;
  std::vector<PlotterEntry> m_values;

};

#endif // FILEPLOTTER_H

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
