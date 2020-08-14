#include "HTMLFileOut.h"
#include <sstream>
#include <fstream>
#include <stdarg.h>
#ifdef WIN32
  #define NOMINMAX
  #include <windows.h>
#endif
#include <ctime>

using namespace std;
using namespace IVDA;

HTMLFileOut::HTMLFileOut(std::string strFilename, uint32_t bufferLength,
                         bool latestsAtTop) :
  m_strFilename(strFilename),
  m_ringBuffer(bufferLength),
  m_latestsAtTop(latestsAtTop)
{
  this->Message(_IVDA_func_, "Starting up");
}

HTMLFileOut::~HTMLFileOut() {
  this->Message(_IVDA_func_, "Shutting down");
}

std::string HTMLFileOut::channelToHTMLID(enum DebugChannel channel) const {
  switch (channel) {
    case CHANNEL_ERROR : return "id=\"error\"";
    case CHANNEL_WARNING : return "id=\"warning\"";
    case CHANNEL_MESSAGE : return "id=\"message\"";
    case CHANNEL_OTHER : return "id=\"other\"";
    default : return "";
  }
}

void HTMLFileOut::printf(enum DebugChannel channel, const char* source,
                         const char* buff)
{
  stringstream fs;

  if (DisplayDateAndTime()) {
    fs << "<tr " << channelToHTMLID(channel) << "><td>" << source
       << "</td><td>" << getTodayStr() << "</td><td>" << buff << "</td></tr>\n";
  } else {
    fs << "<tr " << channelToHTMLID(channel) << "><td>" << source
       << "</td><td>" << buff << "</td></tr>\n";
  }
  
  m_ringBuffer.put(fs.str());
  writeFile();  
}

void HTMLFileOut::printf(const char *s) const
{
  stringstream fs;

  if (DisplayDateAndTime()) {
    fs << "<tr><td></td><td>" << getTodayStr() << "</td><td>" << s << "</td></tr>\n";
  } else {
    fs << "<tr><td></td><td>" << s << "</td></tr>\n";
  }

  m_ringBuffer.put(fs.str());
  writeFile();
}

void HTMLFileOut::writeFile() const {
  ofstream fs;
  fs.open(m_strFilename.c_str(), ios_base::out);
  if (fs.fail()) return;

  fs << "<!DOCTYPE html>\n";
  fs << "<html lang=\"en-US\">\n";
  fs << "<head>\n";
  
  fs << "  <style>\n";
  fs << "    th, td {padding: 15px;}\n";
  fs << "    tr#error { color: red; }\n";
  fs << "    tr#warning { color: orange; }\n";
  fs << "    tr#message { color: green; }\n";
  fs << "    tr#other { color: black; }\n";
  fs << "    tr:nth-child(even) {background-color: #f2f2f2}\n";
  fs << "  </style>\n";
  
  fs << "</head>\n";
  fs << "<body>\n";
  fs << "  <title>HTML Log</title>\n";
  fs << "  <div style=\"overflow-x:auto;\">\n";
  fs << "  <table>\n";
  if (DisplayDateAndTime()) {
    fs << "    <tr><th>Source</th><th>Time</th><th>Message</th></tr>\n";
  } else {
    fs << "    <tr><th>Source</th><th>Message</th></tr>\n";
  }
  for (size_t i = 0;i<m_ringBuffer.size();++i) {
    if (m_latestsAtTop)
      fs << "    " << *m_ringBuffer.get(m_ringBuffer.size()-(i+1));
    else
      fs << "    " << *m_ringBuffer.get(i);
  }
  fs << "  </table>\n";
  fs << "  </div>\n";
  fs << "</body>\n";
  fs << "</html>\n";
  
  fs.flush();
  fs.close();
}

/*
 The MIT License
 
 Copyright (c) 2015-2018 Jens Krüger
 
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
