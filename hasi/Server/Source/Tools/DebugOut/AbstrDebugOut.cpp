#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "AbstrDebugOut.h"
#include <chrono>
#include <ctime>
#include <sstream>

using namespace IVDA;

const char *AbstrDebugOut::ChannelToString(enum DebugChannel c) const
{
  switch(c) {
    case CHANNEL_NONE:    /* FALL THROUGH */
    case CHANNEL_FINAL:   return "";
    case CHANNEL_ERROR:   return "ERROR";
    case CHANNEL_WARNING: return "WARNING";
    case CHANNEL_MESSAGE: return "MESSAGE";
    case CHANNEL_OTHER:   return "OTHER";
  }
  
#ifndef __clang__ // never reached but makes GCC happy
  return "";
#endif
}

bool AbstrDebugOut::Enabled(enum DebugChannel channel) const
{
  switch(channel) {
    case CHANNEL_NONE:  /* FALL THROUGH */
    case CHANNEL_FINAL: return true;
    case CHANNEL_ERROR: return m_bShowErrors;
    case CHANNEL_WARNING: return m_bShowWarnings;
    case CHANNEL_MESSAGE: return m_bShowMessages;
    case CHANNEL_OTHER: return m_bShowOther;
  }
  
#ifndef __clang__ // never reached but makes GCC happy
  return false;
#endif
}

void AbstrDebugOut::Other(const char *source, const char* format, ...)
{
  if (!m_bShowOther) return;
  char buff[16384];

  va_list args;
  va_start(args, format);
#ifdef DETECTED_OS_WINDOWS
  _vsnprintf_s(buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf(buff, sizeof(buff), format, args);
#endif
  va_end(args);

  this->printf(CHANNEL_OTHER, source, buff);
}

void AbstrDebugOut::Message(const char* source, const char* format, ...)
{
  if (!m_bShowMessages) return;
  char buff[16384];

  va_list args;
  va_start(args, format);
#ifdef DETECTED_OS_WINDOWS
  _vsnprintf_s(buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf(buff, sizeof(buff), format, args);
#endif
  va_end(args);

  this->printf(CHANNEL_MESSAGE, source, buff);
}
void AbstrDebugOut::Warning(const char* source, const char* format, ...)
{
  if (!m_bShowWarnings) return;
  char buff[16384];

  va_list args;
  va_start(args, format);
#ifdef DETECTED_OS_WINDOWS
  _vsnprintf_s(buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf(buff, sizeof(buff), format, args);
#endif
  va_end(args);

  this->printf(CHANNEL_WARNING, source, buff);
}
void AbstrDebugOut::Error(const char* source, const char* format, ...)
{
  if (!m_bShowErrors) return;
  char buff[16384];

  va_list args;
  va_start(args, format);
#ifdef DETECTED_OS_WINDOWS
  _vsnprintf_s(buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf(buff, sizeof(buff), format, args);
#endif
  va_end(args);

  this->printf(CHANNEL_ERROR, source, buff);
}

void AbstrDebugOut::PrintErrorList() {
  printf( "Printing recorded errors:" );
  for (std::deque<std::string>::const_iterator i =
         m_strLists[CHANNEL_ERROR].begin();
       i != m_strLists[CHANNEL_ERROR].end(); ++i) {
    printf( i->c_str() );
  }
  printf( "end of recorded errors" );
}

void AbstrDebugOut::PrintWarningList() {
  printf( "Printing recorded errors:" );
  for (std::deque<std::string>::const_iterator i =
         m_strLists[CHANNEL_WARNING].begin();
       i != m_strLists[CHANNEL_WARNING].end(); ++i) {
    printf( i->c_str() );
  }
  printf( "end of recorded errors" );
}

void AbstrDebugOut::PrintMessageList() {
  printf( "Printing recorded errors:" );
  for (std::deque<std::string>::const_iterator i =
         m_strLists[CHANNEL_MESSAGE].begin();
       i != m_strLists[CHANNEL_MESSAGE].end(); ++i) {
    printf( i->c_str() );
  }
  printf( "end of recorded errors" );
}

void AbstrDebugOut::SetOutput(bool bShowErrors,
                              bool bShowWarnings,
                              bool bShowMessages,
                              bool bShowOther) {
  SetShowMessages(bShowMessages);
  SetShowWarnings(bShowWarnings);
  SetShowErrors(bShowErrors);
  SetShowOther(bShowOther);
}

void AbstrDebugOut::GetOutput(bool& bShowErrors,
                              bool& bShowWarnings,
                              bool& bShowMessages,
                              bool& bShowOther) const {

  bShowMessages = ShowMessages();
  bShowWarnings = ShowWarnings();
  bShowErrors   = ShowErrors();
  bShowOther    = ShowOther();
}


void AbstrDebugOut::SetShowMessages(bool bShowMessages) {
  m_bShowMessages = bShowMessages;
}

void AbstrDebugOut::SetShowWarnings(bool bShowWarnings) {
  m_bShowWarnings = bShowWarnings;
}

void AbstrDebugOut::SetShowErrors(bool bShowErrors) {
  m_bShowErrors = bShowErrors;
}

void AbstrDebugOut::SetShowOther(bool bShowOther) {
  m_bShowOther = bShowOther;
}

void AbstrDebugOut::SetDisplayDateAndTime(bool bDisplayDateAndTime) {
  m_bDisplayDateAndTime = bDisplayDateAndTime;
}

void AbstrDebugOut::ReplaceSpecialChars(char* buff, size_t iSize) const {
  std::string s = buff;

  size_t j=0;
  for (;(j = s.find( "%", j )) != std::string::npos;)
  {
    s.replace( j, 1, "%%" );
    j+=2;
  }

  size_t iLength = std::min(s.length(), iSize-1);
  std::memcpy(buff, s.c_str(), iLength);
  buff[iLength] = 0;
}

std::string AbstrDebugOut::getTodayStr() {
  std::stringstream ss;

  using std::chrono::system_clock;
  system_clock::time_point today = system_clock::now();
  std::time_t tt;
  tt = system_clock::to_time_t ( today );
  ss << ctime(&tt);

  std::string s = ss.str();
  return s.substr(0,s.length()-1); // remove newline
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
