#include <cassert>
#include <cstdlib>
#include "Timer.h"

#ifndef DETECTED_OS_WINDOWS
  using namespace std;
#endif

namespace IVDA
{

	Timer::Timer() {
    Init();
	}

  Timer::Timer(double fTimeOffsetInMilliseconds) {
    Init();
#ifndef DETECTED_OS_WINDOWS
    m_iStartTime.tv_sec  = time_t(fTimeOffsetInMilliseconds/1000.0);
    m_iStartTime.tv_usec = suseconds_t((fTimeOffsetInMilliseconds-m_iStartTime.tv_sec*1000.0)*1000.0);
#else
    m_iStartTime.QuadPart = LONGLONG(fTimeOffsetInMilliseconds * m_fTicksPerMillisecond);
#endif
  }

  void Timer::Init() {
    #ifdef DETECTED_OS_WINDOWS
      LARGE_INTEGER ticksPerSecond;
      QueryPerformanceFrequency(&ticksPerSecond);
      m_fTicksPerMillisecond = double(ticksPerSecond.QuadPart) / 1000.0;
    #endif
  }

	double Timer::Start() {
	#ifndef DETECTED_OS_WINDOWS
	  gettimeofday(&m_iStartTime, NULL);
    return (m_iStartTime.tv_sec) * 1000.0 + (m_iStartTime.tv_usec) / 1000.0;
	#else
	  QueryPerformanceCounter(&m_iStartTime);
    return double(m_iStartTime.QuadPart)/m_fTicksPerMillisecond;
	#endif
	}


	double Timer::Elapsed() {
	#ifndef DETECTED_OS_WINDOWS
	  struct timeval timeProbe;
	  gettimeofday(&timeProbe, NULL);

	  return (timeProbe.tv_sec  - m_iStartTime.tv_sec) * 1000.0 +
			     (timeProbe.tv_usec - m_iStartTime.tv_usec) / 1000.0;
	#else
	  LARGE_INTEGER timeProbe;
	  QueryPerformanceCounter(&timeProbe);
	  ULONGLONG ticksPassed = timeProbe.QuadPart -  m_iStartTime.QuadPart;// = tick.QuadPart/ticksPerSecond.QuadPart;
	  return double(ticksPassed)/m_fTicksPerMillisecond;
	#endif
	}
}

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