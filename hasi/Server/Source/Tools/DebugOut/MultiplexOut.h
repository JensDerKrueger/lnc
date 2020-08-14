#pragma once

#include <vector>
#include "AbstrDebugOut.h"

namespace IVDA {

class MultiplexOut : public AbstrDebugOut {
  public:
    MultiplexOut() {}
    ~MultiplexOut();

    void AddDebugOut(AbstrDebugOut* pDebugger);
    void RemoveDebugOut(AbstrDebugOut* pDebugger);

    virtual void printf(enum DebugChannel, const char* source,
                        const char* msg);
    virtual void printf(const char *s) const;

    virtual void SetShowMessages(bool bShowMessages);
    virtual void SetShowWarnings(bool bShowWarnings);
    virtual void SetShowErrors(bool bShowErrors);
    virtual void SetShowOther(bool bShowOther);

    size_t size() const { return m_vpDebugger.size(); }
    bool empty() const { return m_vpDebugger.empty(); }
    void clear();

  private:
    std::vector<AbstrDebugOut*> m_vpDebugger;
};
}; // namespace IVDA


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
