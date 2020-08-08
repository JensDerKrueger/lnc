/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


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

/**
  \file    Console.h
  \author    Thomas Schiwietz
        Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    July 2008
*/

#pragma once

#ifndef IVDA_CONSOLE_H
#define IVDA_CONSOLE_H

#include <cstdarg>
#include <cstdio>
#ifdef WIN32
  #pragma warning(disable:4505)
#endif
#include <iostream>

#include "StdDefines.h"

#if defined(__GNUC__) && __GNUC__ > 3 || \
    (__GNUC__ == 3 && __GNUC_MINOR_ >= 4)
#   define FQN_UNUSED __attribute__ ((unused))
#   define FQN_PRINTF __attribute__ ((format (printf, 1, 2)))
#else
#   define FQN_UNUSED /* nothing */
#   define FQN_PRINTF /* nothing */
#endif

namespace IVDA {

namespace Console {
  #ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>

    static void printf(const WCHAR* format, ...)
    {
      // output string
      WCHAR buff[16384];

      // arguments
      va_list args;
      va_start(args, format);

      // build string
      _vsnwprintf_s( buff, 16384, sizeof(buff), format, args);

      // convert to unicode
      OutputDebugStringW(buff);
#ifndef DEBUG_CONSOLE_ONLY
      std::cout << buff;
#endif
    }

    static void printfd(const WCHAR* format, ...)
    {
      #ifdef _DEBUG
          // output string
          WCHAR buff[16384];

          // arguments
          va_list args;
          va_start(args, format);

          // build string
          _vsnwprintf_s( buff, 16384, sizeof(buff), format, args);

          // convert to unicode
          OutputDebugStringW(buff);
#ifndef DEBUG_CONSOLE_ONLY
      std::cout << buff;
#endif
      #else
        UNREFERENCED_PARAMETER(format);
      #endif
    }


    static void printf(const CHAR* format, ...)
    {
      // output string
      CHAR buff[16384];

      // arguments
      va_list args;
      va_start(args, format);

      // build string
      _vsnprintf_s( buff, 16384, sizeof(buff), format, args);

      // convert to unicode
      OutputDebugStringA(buff);
#ifndef DEBUG_CONSOLE_ONLY
      std::cout << buff;
#endif
    }

    static void printfd(const CHAR* format, ...)
    {
      #ifdef _DEBUG
          // output string
          CHAR buff[16384];

          // arguments
          va_list args;
          va_start(args, format);

          // build string
          _vsnprintf_s( buff, 16384, sizeof(buff), format, args);

          // convert to unicode
          OutputDebugStringA(buff);
#ifndef DEBUG_CONSOLE_ONLY
      std::cout << buff;
#endif
      #else
        UNREFERENCED_PARAMETER(format);
      #endif
    }
#else
    #include <wchar.h>
    static void printf(const wchar_t *, ...) FQN_UNUSED;
    static void printfd(const wchar_t*, ...) FQN_UNUSED;

    static void printf(const char* , ...) FQN_UNUSED FQN_PRINTF;
    static void printfd(const char*, ...) FQN_UNUSED FQN_PRINTF;

    static void printf(const wchar_t* format, ...)
    {
      // output string
      wchar_t buff[16384];

      // arguments
      va_list args;
      va_start(args, format);

      vswprintf( buff, sizeof(buff), format, args);
      std::cout << buff;
    }

    #ifdef _DEBUG
      static void printfd(const wchar_t* format, ...)
      {
          // output string
          wchar_t buff[16384];

          // arguments
          va_list args;
          va_start(args, format);

          vswprintf( buff, sizeof(buff), format, args);
          std::cout << buff;
      }
    #else
      static void printfd(const wchar_t*, ...) {}
    #endif

    static void printf(const char* format, ...)
    {
      // output string
      char buff[16384];

      // arguments
      va_list args;
      va_start(args, format);

      vsnprintf( buff, sizeof(buff), format, args);
      std::cout << buff;
    }

    #ifdef _DEBUG
      static void printfd(const char* format, ...)
      {
          // output string
          char buff[16384];

          // arguments
          va_list args;
          va_start(args, format);

          vsnprintf( buff, sizeof(buff), format, args);
          std::cout << buff;
      }
    #else
    static void printfd(const char*, ...) {}
    #endif


#endif
};
}; // namespace IVDA
#endif // IVDA_CONSOLE_H
