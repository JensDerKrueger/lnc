#ifndef WEBSERVICEDEVICE_H
#define WEBSERVICEDEVICE_H

#include <string>    // std::string

#include <HASBasics.h>
#include <HASMember.h>
#include <IAnalogIn.h>

namespace HAS {
  
  class WebServiceDevice : public HASMember, public IAnalogIn {
  public:
    WebServiceDevice(const std::string& devID, const std::string& hrname,
                     const HASConfigPtr config,
                     const std::string& server, const std::string& URL,
                     uint64_t iRefreshMinutes=60);
    
    virtual void init() {}
    virtual bool updateData(uint64_t iCurrentTime);

    // partial IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    
  protected:
    std::string m_server;
    std::string m_URL;

    virtual std::string getDesc() const;
    virtual bool reloadData() = 0;
    
    uint64_t getDataAge() const;

  private:
    uint64_t m_iRefreshMinutes;
    uint64_t m_iLastRefresh;
    
  };

}

#endif // WEBSERVICEDEVICE_H


/*
   The MIT License

   Copyright (c) 2014 Jens Krueger

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
