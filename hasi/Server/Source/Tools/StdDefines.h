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
 \file    StdDefines.h
 \author    Jens Krueger, IVDA, SCI, Intel VCI
 */

#pragma once

#ifndef IVDA_STDDEFINES
#define IVDA_STDDEFINES

// Set faked variadic template argument count to a reasonable value.
// The default value of 5 is pretty low for Visual Studio 2012.
// This is necessary for using std::bind() with more than 5 arguments.
#ifdef _MSC_VER
  #if _MSC_VER == 1700
    #ifdef _VARIADIC_MAX
      #pragma message("    [StdDefines.h] Using previously defined _VARIADIC_MAX value.\n                   Increase this value or include this header first if you run into problems with std::bind(...)!\n")
    #else
      #define _VARIADIC_MAX 8
    #endif
  #endif
#endif

#define NO_MAC_RESOURCE_LOADER

#include "stdint.h"

// Make sure windows doesn't give us stupid macros that interfere with
// functions in 'std'.
#define NOMINMAX
// Disable checked iterators on Windows.
#ifndef _DEBUG
# undef _SECURE_SCL
# define _SECURE_SCL 0
#endif
// Get rid of stupid warnings.
#define _CRT_SECURE_NO_WARNINGS 1

#define UNUSED (0)
#define UNUSED_FLOAT (0.0f)
#define UNUSED_DOUBLE (0.0)

// undef all OS types first
#ifdef DETECTED_OS_WINDOWS
	#undef DETECTED_OS_WINDOWS
#elif defined(DETECTED_OS_APPLE)
	#undef DETECTED_OS_APPLE
	#ifdef DETECTED_IOS
		#undef DETECTED_IOS
	#elif defined(DETECTED_IOS_SIMULATOR)
		#undef DETECTED_IOS_SIMULATOR
	#elif defined(DETECTED_OS_MAC)
		#undef DETECTED_OS_MAC
	#endif
#elif defined(DETECTED_OS_LINUX)
	#undef DETECTED_OS_LINUX
#endif

// now figure out which OS we are compiling on
#ifdef _WIN32
	#define DETECTED_OS_WINDOWS
#elif defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
	#define DETECTED_OS_APPLE
	#include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR > 0
    #define DETECTED_IOS_SIMULATOR
  #elif TARGET_OS_IPHONE > 0
		#define DETECTED_IOS
	#elif TARGET_OS_MAC > 0
		#define DETECTED_OS_MAC
	#endif
#elif defined(__linux__)
	#define DETECTED_OS_LINUX
#endif

// Disabled for now; clean up include ordering at some point before
// re-enabling.
#if 0
# define _POSIX_C_SOURCE 200112L
#endif

// Disable the "secure CRT" garbage warnings.
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
// The above is the documented way to do it, but doesn't work.  This does:
#ifdef DETECTED_OS_WINDOWS
# pragma warning(disable: 4996)
#endif

// allows windows.h to be included after winsock2.h
// do not use here
//#ifdef _WIN32
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif
//#endif

#include <limits>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <functional>

namespace IVDA {
  static const uint32_t INFINITE_TIMEOUT = std::numeric_limits<uint32_t>::max();
  typedef std::function<bool ()> Predicate;
}

#include <memory>

// declare shared and weak pointer typedefs for given type name
#define SHARED_PTR_IVDA(TypeName)\
  typedef std::shared_ptr<TypeName> TypeName##Ptr;\
  typedef std::weak_ptr<TypeName> TypeName##Wtr;

// forward declare class name and declare its shared and weak pointer typedefs
#define SHARED_PTR_IVDA_CLASS(ClassName)\
  class ClassName;\
  SHARED_PTR_IVDA(ClassName);

// forward declare struct name and declare its shared and weak pointer typedefs
#define SHARED_PTR_IVDA_STRUCT(StructName)\
  struct StructName;\
  SHARED_PTR_IVDA(StructName);

#endif // STDDEFINES_H
