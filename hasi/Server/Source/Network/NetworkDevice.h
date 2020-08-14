#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H

#include <HASMember.h>
#include <string>    // std::string
#include <vector>    // std::vector

namespace HAS {

  struct DataDesc {
    std::string desc;
    std::string unit;
    float value;
  };
  
  class NetworkDevice : public HASMember {
  public:
    NetworkDevice(const std::string& devID, const std::string& hrname,
                  const HASConfigPtr config, const std::string& password);
    
    // partial IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
  
    virtual void prepareShutdown() = 0;
    
  protected:
    bool m_bConnectPulseSend;
    uint32_t m_iTimeout;
    std::string m_password;
    virtual std::string getDesc() const;
    
    void removeChar(std::string& str, char s) const;
    bool checkPostFix(std::string str, const std::string& pf) const;
    std::string removeHeaderAndFooter(std::string str) const;
    std::vector<std::string> stringToLineList(const std::string& str) const;
    
    std::string dataBegin;
    std::string dataEnd;
    std::string dataBeginNL;
    std::string dataEndNL;
    std::string testHeader;
    
    virtual bool isConnected() const = 0;
    virtual bool hasData() const = 0;
    
  };

}

#endif // NETWORKDEVICE_H


/*
   The MIT License

   Copyright (c) 2013 Jens Krueger

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
