#include <algorithm>
#include <cstdio>
#include <functional>
#include <stdarg.h>
#include "MultiplexOut.h"

#ifdef WIN32
  #define NOMINMAX
  #include <windows.h>
#endif

using namespace std;
using namespace IVDA;

MultiplexOut::~MultiplexOut() {
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    m_vpDebugger[i]->Other(_IVDA_func_, "Shutting down");
    delete m_vpDebugger[i];
  }
}

void MultiplexOut::AddDebugOut(AbstrDebugOut* pDebugger) {
  m_vpDebugger.push_back(pDebugger);
  pDebugger->Other(_IVDA_func_,"Operating as part of a multiplexed debug out now.");

  // Find the maximal set of channels to enable.
  m_bShowMessages |= pDebugger->ShowMessages();
  m_bShowWarnings |= pDebugger->ShowWarnings();
  m_bShowErrors |= pDebugger->ShowErrors();
  m_bShowOther |= pDebugger->ShowOther();
}

void MultiplexOut::RemoveDebugOut(AbstrDebugOut* pDebugger) {
  std::vector<AbstrDebugOut*>::iterator del;

  del = std::find(m_vpDebugger.begin(), m_vpDebugger.end(), pDebugger);

  if(del != m_vpDebugger.end()) {
    delete *del;
    m_vpDebugger.erase(del);
  }
}


void MultiplexOut::printf(enum DebugChannel channel, const char* source,
                          const char* msg)
{
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    if(m_vpDebugger[i]->Enabled(channel)) {
      m_vpDebugger[i]->printf(channel, source, msg);
    }
  }
}

void MultiplexOut::printf(const char *s) const
{
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    m_vpDebugger[i]->printf(s);
  }
}

void MultiplexOut::SetShowMessages(bool bShowMessages) {
  AbstrDebugOut::SetShowMessages(bShowMessages);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowMessages(bShowMessages);
}

void MultiplexOut::SetShowWarnings(bool bShowWarnings) {
  AbstrDebugOut::SetShowWarnings(bShowWarnings);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowWarnings(bShowWarnings);
}

void MultiplexOut::SetShowErrors(bool bShowErrors) {
  AbstrDebugOut::SetShowErrors(bShowErrors);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowErrors(bShowErrors);
}

void MultiplexOut::SetShowOther(bool bShowOther) {
  AbstrDebugOut::SetShowOther(bShowOther);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowOther(bShowOther);
}

template <class T>
struct deleter : std::unary_function<T, void> {
  void operator()(T* p) const {
    delete p;
  }
};

void MultiplexOut::clear()
{
  std::for_each(m_vpDebugger.begin(), m_vpDebugger.end(),
                deleter<AbstrDebugOut>());
  m_vpDebugger.clear();
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
