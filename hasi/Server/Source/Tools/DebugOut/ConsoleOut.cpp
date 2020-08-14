#include <cstring>
#include "ConsoleOut.h"
#include "../Console.h"

using namespace IVDA;

/// ANSI escape codes for various colors.
///@{
//static const char C_DGRAY[]  = "\033[01;30m";  // unused
static const char C_NORM[]   = "\033[00m";
static const char C_RED[]    = "\033[01;31m";
static const char C_YELLOW[] = "\033[01;33m";
static const char C_GREEN[]  = "\033[01;32m";
//static const char C_MAG[]    = "\033[01;35m"; // unused
static const char C_LBLUE[]  = "\033[01;36m";
//static const char C_WHITE[]  = "\033[01;27m";  // unused
///@}

ConsoleOut::ConsoleOut()
{
  Message("ConsoleOut::ConsoleOut:","Starting up ConsoleDebug out");
}

ConsoleOut::~ConsoleOut()
{
  Message("ConsoleOut::~ConsoleOut:","Shutting down ConsoleDebug out");
}

void ConsoleOut::printf(enum DebugChannel channel, const char* source, const char* msg)
{
  char buff[16384];
#ifdef DETECTED_OS_WINDOWS
  strncpy_s(buff, 16384, msg, 16384);
#else
  strncpy(buff, msg, 16383);
#endif
#ifdef DETECTED_OS_WINDOWS
  if (DisplayDateAndTime()) {
    Console::printf("%s %s (%s): %s\n", ChannelToString(channel), getTodayStr().c_str(), source, buff);
  } else {
    Console::printf("%s (%s): %s\n", ChannelToString(channel), source, buff);
  }
#else
  const char *color = C_NORM;
  switch(channel) {
    case CHANNEL_FINAL:   /* FALL THROUGH */
    case CHANNEL_NONE:    color = C_NORM; break;
    case CHANNEL_ERROR:   color = C_RED; break;
    case CHANNEL_WARNING: color = C_YELLOW; break;
    case CHANNEL_MESSAGE: color = C_GREEN; break;
    case CHANNEL_OTHER:   color = C_LBLUE; break;
  }
  
  if (DisplayDateAndTime()) {
    Console::printf("%s%s%s %s (%s): %s\n", color, ChannelToString(channel), C_NORM, getTodayStr().c_str(), source, buff);
  } else {
    Console::printf("%s%s%s (%s): %s\n", color, ChannelToString(channel), C_NORM, source, buff);
  }
  
#endif
}

void ConsoleOut::printf(const char *s) const
{
  Console::printf("%s\n", s);
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
