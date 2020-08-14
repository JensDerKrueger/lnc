#include "TextfileOut.h"
#include <fstream>
#include <stdarg.h>
#ifdef WIN32
  #define NOMINMAX
  #include <windows.h>
#endif
#include <ctime>

using namespace std;
using namespace IVDA;

TextfileOut::TextfileOut(std::string strFilename, bool bClearFile) :
  m_strFilename(strFilename)
{
  this->Message(_IVDA_func_, "Starting up");

  if (bClearFile) {
    ofstream fs;
    fs.open(m_strFilename.c_str());
    fs.close();
  }
}

TextfileOut::~TextfileOut() {
  this->Message(_IVDA_func_, "Shutting down\n");
}

void TextfileOut::printf(enum DebugChannel channel, const char* source,
                         const char* buff)
{
  ofstream fs;
  fs.open(m_strFilename.c_str(), ios_base::app);
  if (fs.fail()) return;

  if (DisplayDateAndTime()) {
    fs << getTodayStr() << " ";
  }
  
  fs << ChannelToString(channel) << " (" << source << ") " << buff << std::endl;

  fs.flush();
  fs.close();
}

void TextfileOut::printf(const char *s) const
{
  ofstream fs;
  fs.open(m_strFilename.c_str(), ios_base::app);
  if (fs.fail()) return;

  if (DisplayDateAndTime()) {
    fs << getTodayStr() << " ";
  }

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
