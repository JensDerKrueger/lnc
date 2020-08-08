#ifndef NETWORKINPUTOUTPUTSERVER_H
#define NETWORKINPUTOUTPUTSERVER_H

#include "NetworkInputServer.h"
#include "../IAnalogOut.h"

namespace HAS {
  
  class NetworkInputOutputServer : public NetworkInputServer, public IAnalogOut {
  public:
    NetworkInputOutputServer(const std::string& devID, const std::string& hrname,
                             HASConfigPtr config,
                             uint16_t inPort, uint8_t iInChannelCount,
                             uint16_t outPort, uint8_t iOutChannelCount,
                             const std::string& password);
    virtual ~NetworkInputOutputServer();
    
    virtual void init();
    
    // digital-out interface
    virtual uint8_t getAnalogOutChannelCount() const;
    virtual void setAnalog(uint8_t iChannel, float value);
    virtual void applyAnalogOut();
    
    static NetworkInputOutputServer* deviceFromStrings(const std::vector<std::string>& entries,
                                                       HASConfigPtr config);
    
    virtual void prepareShutdown();
    
  protected:
    uint16_t m_outPort;
    uint8_t m_iOutChannelCount;
    
    IVDA::CriticalSection m_OutGuard;
    IVDA::CriticalSection m_OutClientGuard;
    std::vector<float> m_outVal;
    std::shared_ptr<IVDA::TCPServer> m_pOutServer;
    std::shared_ptr<IVDA::TCPSocket> m_pOutClient;
    std::shared_ptr<IVDA::LambdaThread> m_outConnector;
    
    std::shared_ptr<AESCrypt> m_AESCryptOut;
    
    bool m_bForceUpdate;
    
    void shutdownOutServer();
    void establishConnection(IVDA::Predicate pContinue,
                             IVDA::LambdaThread::Interface& threadInterface);

    void sendString(const std::string& str);
    void sendInit();
    
    virtual std::string getDesc() const;
    
    virtual void processData(const std::string& str);
    
    virtual void connectionInterrupted(const std::string& str);
  };
  
}

#endif // NETWORKINPUTOUTPUTSERVER_H


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
