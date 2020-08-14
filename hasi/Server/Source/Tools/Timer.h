#pragma once

#ifndef IVDA_TIMER
#define IVDA_TIMER

#include "StdDefines.h"

#ifdef DETECTED_OS_WINDOWS
  #define NOMINMAX
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

namespace IVDA
{
	/** \class Timer */

#ifdef DETECTED_OS_WINDOWS
  typedef LARGE_INTEGER TimeType;
#else
  typedef timeval TimeType;
#endif

	class Timer {
	public:
    Timer();
    Timer(double fTimeOffsetInMilliseconds);
	  double Start();   // returns milliseconds
	  double Elapsed(); // returns milliseconds
	private:
	  #ifdef DETECTED_OS_WINDOWS
  		double m_fTicksPerMillisecond;
	  #endif
    
    TimeType m_iStartTime;
    void Init();
	};
}

#endif // IVDA_TIMER

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
