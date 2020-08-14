#ifndef NETWORKINPUTOUTPUTCLIENT_H
#define NETWORKINPUTOUTPUTCLIENT_H

#include "../HASMember.h"
#include <string>    // std::string
#include <Tools/Sockets.h>
#include <Tools/Threads.h>

#include <IAnalogIn.h>
#include <IAnalogOut.h>
#include "NetworkDevice.h"  // DataDesc struct

#include <Tools/AES.h>  // AESCrypt

namespace HAS {
  
  class NetworkInputOutputClient : public NetworkDevice, public IAnalogIn, public IAnalogOut {
  public:
    NetworkInputOutputClient(const std::string& devID, const std::string& hrname,
                             HASConfigPtr config, const std::string& serverIP,
                             uint16_t inPort, uint8_t iInChannelCount,
                             uint16_t outPort, uint8_t iOutChannelCount,
                             const std::string& strPassword);
    virtual ~NetworkInputOutputClient();
    
    // HASMember interface
    virtual void init();
    
    // IAnalogOut interface
    virtual uint8_t getAnalogOutChannelCount() const;
    virtual void setAnalog(uint8_t iChannel, float value);
    virtual void applyAnalogOut();
    
    // IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual uint32_t pollAnalogIn();
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    
    static NetworkInputOutputClient* deviceFromStrings(const std::vector<std::string>& entries, HASConfigPtr config);
    
    virtual void prepareShutdown();
    
  protected:
    // HASMember interface
    virtual std::string getDesc() const;
    
    // NetworkDevice interface
    virtual bool isConnected() const;
    virtual bool hasData() const;

  private:
    std::string getDataDescString() const;

    std::string m_serverIP;
    short m_iReceivePort;
    uint8_t m_iReceiveChannelCount;
    short m_iSendPort;
    uint8_t m_iSendChannelCount;
    bool m_bReceiverHasData;
    bool m_bReceiverIsInitialized;
    
    std::shared_ptr<AESCrypt> m_AESCryptIn;
    std::shared_ptr<AESCrypt> m_AESCryptOut;
    
    std::string m_receiveBuffer;

    IVDA::CriticalSection m_ReceiveGuard;
    std::vector<DataDesc> m_receiveDataDesc;
    IVDA::CriticalSection m_SendGuard;
    std::vector<DataDesc> m_sendDataDesc;
    
    
    std::shared_ptr<IVDA::TCPSocket> m_pReceiveClient;
    std::shared_ptr<IVDA::TCPSocket> m_pSendClient;

    std::shared_ptr<IVDA::LambdaThread> m_keepAliveThread;
    std::shared_ptr<IVDA::LambdaThread> m_receiverThread;
    std::shared_ptr<IVDA::LambdaThread> m_senderThread;
    
    void keepAliveFunc(IVDA::Predicate pContinue,
                       IVDA::LambdaThread::Interface& threadInterface);
    void senderFunc(IVDA::Predicate pContinue,
                    IVDA::LambdaThread::Interface& threadInterface);
    void receiverFunc(IVDA::Predicate pContinue,
                      IVDA::LambdaThread::Interface& threadInterface);
    
    
    void sendString(const std::string& str);
    void sendInit();
    void sendData();

    bool receiveLoop(IVDA::Predicate pContinue, bool init);
    void handleIncommingData(int8_t data, bool init);
    
    void receiveInit(const std::string& data);
    void receiveData(const std::string& data);
    
    std::string decryptFirstMessage(const std::string& message);
    std::string decryptMessage(const std::string& message);    
    
  };
  
}

#endif // NETWORKINPUTOUTPUTCLIENT_H


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
