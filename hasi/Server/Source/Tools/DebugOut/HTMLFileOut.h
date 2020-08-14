#pragma once

#include <string>
#include "AbstrDebugOut.h"
#include <Tools/RingBuffer.h>

namespace IVDA {

class HTMLFileOut : public AbstrDebugOut {
  public:
    HTMLFileOut(std::string strFilename="logfile.html", uint32_t bufferLength=100,
                bool latestsAtTop=true);
    ~HTMLFileOut();
    virtual void printf(enum DebugChannel, const char* source,
                        const char* msg);
    virtual void printf(const char *s) const;

    const std::string& GetFileName() const {return m_strFilename;}

  private:
    HTMLFileOut(const HTMLFileOut &); ///< unimplemented.
    std::string m_strFilename;
    mutable RingBuffer<std::string> m_ringBuffer;
    bool m_latestsAtTop;
    void writeFile() const;
    std::string channelToHTMLID(enum DebugChannel channel) const;

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
