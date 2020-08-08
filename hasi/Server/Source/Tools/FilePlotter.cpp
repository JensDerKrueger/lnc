#include "FilePlotter.h"
#include "SmallImage.h"
#include "SysTools.h"
#include "DebugOutHandler.h"
#include <iomanip>
#include <fstream>
#include <sstream>

FilePlotter::FilePlotter(const std::string& strFilename)
{
  m_strFilenames.push_back(strFilename);
}

void FilePlotter::beginLog() {
  m_values.clear();
}

std::string FilePlotter::toString() const {
  std::stringstream ss;
  ss << "Logging to";
  for (auto name = m_strFilenames.begin();name<m_strFilenames.end();++name) {
    ss << " " << *name;
  }
  return ss.str();
}

void FilePlotter::logLine(std::ofstream& logfile, std::vector<PlotterEntry>& values) {
  time_t t = time(0);
  tm *nun = localtime(&t);
  
  logLine(nun, logfile, values);
}

void FilePlotter::logLine(tm *nun, std::ofstream& logfile, std::vector<PlotterEntry>& values) {
  logfile << std::setw(2) << std::setfill('0') << nun->tm_mday << '.'
  << std::setw(2) << std::setfill('0') << nun->tm_mon+1 << '.'
  << nun->tm_year+1900 << "\t" << nun->tm_hour
  << ':' << std::setw(2) << std::setfill('0') << nun->tm_min
  << ':' << std::setw(2) << std::setfill('0') << nun->tm_sec;
  for (auto entry = values.begin();entry<values.end();++entry) {
    if (entry->unit.empty())
      logfile << "\t" << entry->desc << "\t" << entry->value;
    else
      logfile << "\t" << entry->desc << " (" << entry->unit << ")\t" << entry->value;
  }
  logfile << "\n";
}

bool FilePlotter::endLog() {
  time_t t = time(0);
  tm *nun = localtime(&t);

  for (auto name = m_strFilenames.begin();name<m_strFilenames.end();++name) {
    std::ofstream logfile;
    logfile.open (name->c_str(), std::ios::out | std::ios::app);

    if (!logfile.is_open()) {
      return false;
    }

    logLine(nun, logfile, m_values);

    logfile.close();
  }
  return true;
}

std::vector<FileInfo> FilePlotter::plotImage(const std::string& strFilename, unsigned int w, unsigned int h) const {
  return plotImage(m_strFilenames[0], strFilename, w, h);
}

static std::string fixNonFilenameChars(const std::string& str) {
  std::string result = str;
  IVDA::SysTools::ReplaceAll(result, "/", "_");
  IVDA::SysTools::ReplaceAll(result, "\\", "_");
  IVDA::SysTools::ReplaceAll(result, " ", "_");
  return result;
}

std::vector<FileInfo> FilePlotter::plotImage(const std::string& strSourceFilename, const std::string& strFilename, unsigned int w, unsigned int h) {
  std::vector<FileInfo> info;

  uint32_t lines = 0;
  std::vector<std::pair<double, double>> minMax;
  std::vector<std::string> names;

  // figure out how many entries we have in the file by counting the lines
  std::ifstream logfile;
  logfile.open (strSourceFilename.c_str(), std::ios::in);
  if (!logfile.is_open()) {
    IVDA_WARNING("Unable to open log file " << strSourceFilename);
    return info;
  }

  std::string line;
  while(std::getline(logfile,line)) {
    std::vector<std::string> elems = IVDA::SysTools::Tokenize(line, IVDA::SysTools::PM_CUSTOM_DELIMITER, '\t');

    if (lines == 0) {
      minMax.resize((elems.size()-2) / 2);
      names.resize(minMax.size());
      for (uint32_t i = 0; i<minMax.size();++i) {
        double v = IVDA::SysTools::FromString<double>(elems[3+i*2]);
        minMax[i].first = v;
        minMax[i].second = v;
        names[i] = elems[2+i*2];
      }
    } else {
     for (uint32_t i = 0; i<minMax.size();++i) {
        double v = IVDA::SysTools::FromString<double>(elems[3+i*2]);
        minMax[i].first = std::min(minMax[i].first, v);
        minMax[i].second = std::max(minMax[i].second, v);
      }
    }
    lines++;
  }

  if (lines == 0) {
    IVDA_WARNING("Log file " << strSourceFilename << " was empty");
    return info;
  }

  if (minMax.size() == 0) {
    IVDA_WARNING("Log file " << strSourceFilename << " did not contain valid data in the first line, deleting file.");
    remove(strSourceFilename.c_str());
    return info;
  }


  // make sure names are unique
  for (size_t elementA = 0;elementA<names.size();++elementA) {
    for (size_t elementB = elementA+1;elementB<names.size();++elementB) {
      if (names[elementA] == names[elementB]) {
        names[elementB] += "_";
      }
    }
  }

  for (size_t element = 0;element<minMax.size();++element) {
    logfile.close();
    logfile.open (strSourceFilename.c_str(), std::ios::in);

    std::stringstream ss;
    ss << IVDA::SysTools::GetFilename(strFilename) << "_" << fixNonFilenameChars(names[element])  << ".bmp";
    std::string elemFilename = ss.str();

    float aspect = float(h)/float(w);

    if (lines < w) w = lines;
    unsigned int ratio = (unsigned int)ceil(lines/float(w));
    w = lines / ratio;
    h = int( w*aspect );

    if (h*w == 0) continue;

    IVDA::SmallImage image(w,h,3);
    uint8_t* data = image.GetDataPtrRW();
    std::fill(data, data+w*h*3, 255);  // white image

    unsigned int last = 0;
    for (unsigned int x = 0;x<w;++x) {
      double val = 0;

      for (unsigned int l = 0;l<ratio;++l) {
        std::string line;
        if (!std::getline(logfile,line)) {
          IVDA_WARNING("Log file " << strSourceFilename << " truncated during plot");
          return info;
        }
        std::vector<std::string> elems = IVDA::SysTools::Tokenize(line, IVDA::SysTools::PM_CUSTOM_DELIMITER, '\t');
        val += IVDA::SysTools::FromString<double>(elems[3+element*2]) / ratio;
      }

       // first check if min is differnt from max, if they are the same all values are equal and we can't construct a scaling factor
      unsigned int y = (minMax[element].second == minMax[element].first) ? h/2 : (unsigned int)( double(h-1) * (1.0-(val-minMax[element].first) / (minMax[element].second-minMax[element].first)));

      image.SetPixel(x,y,0,0,0);
      if (x > 0) {
        int d = (last < y) ? 1 : -1;
        for (int p = int(last); abs(p-int(y)) > 0; p += d)
          image.SetPixel(x,p,0,0,0);
      }
      last = y;
    }
    
    if ( ! image.SaveToBMPFile(elemFilename) ) {
      IVDA_WARNING("Unable to save " << elemFilename);
      return info;
    }
    
    elemFilename = image.Convert(elemFilename, "png");
    
    if (elemFilename == "") {
      IVDA_WARNING("Error during image creation.");
    } else {
      FileInfo fi = {elemFilename, names[element], minMax[element].first, minMax[element].second};
      info.push_back(fi);
    }
  }
  return info;
}

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
