#ifndef NETWORKINPUTSERVER_H
#define NETWORKINPUTSERVER_H

#include "NetworkServer.h"
#include "../IAnalogIn.h"
#include <vector>
#include <Tools/AES.h>  // AESCrypt

namespace HAS {
    
  class NetworkInputServer : public NetworkServer, public IAnalogIn {
  public:
    NetworkInputServer(const std::string& devID, const std::string& hrname,
                       HASConfigPtr config, uint16_t port,
                       uint8_t iChannelCount, const std::string& password);
    virtual ~NetworkInputServer() {}
    
    // IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual uint32_t pollAnalogIn();
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    
    static NetworkInputServer* deviceFromStrings(const std::vector<std::string>& entries,
                                                 HASConfigPtr config);
    
    virtual void prepareShutdown();
    
  protected:
    uint8_t m_iChannelCount;
    std::vector<DataDesc> m_dataDesc;
    std::vector<DataDesc> m_dataDescFromClient;
    
    std::shared_ptr<AESCrypt> m_AESCryptIn;
    
    IVDA::CriticalSection m_Guard;
    
    virtual std::string getDesc() const;
    std::string getDataDescString() const;
    
    virtual void processInitialization(const std::string& str);
    virtual void processData(const std::string& str);
    
    virtual void connectionInterrupted(const std::string& str);
    
    std::string decryptFirstMessage(const std::string& message);
    std::string decryptMessage(const std::string& message);
  };
  
}

#endif // NETWORKINPUTSERVER_H


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
