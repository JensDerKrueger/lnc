#pragma once

#ifndef HASIREMOTE_H
#define HASIREMOTE_H

#include <string>    // std::string
#include <map>    // std::map
#include <HASConfig.h>
#include <Script/VarAssignments.h>   // VarStrAssignments
#include <Tools/Sockets.h>
#include <Tools/Threads.h>  // LambdaThread, CriticalSection
#include <Tools/AES.h>  // AESCrypt
#include "RemotePoolThread.h"

namespace HAS {
  
  class HASIRemote {
  public:
    HASIRemote(std::shared_ptr<IVDA::TCPSocket> pClient,
               std::shared_ptr<RemotePoolThread> poolThread,
               HASConfigPtr config);
    virtual ~HASIRemote();
    
    std::vector<std::string> getCommands();
    void update(const VarStrAssignments& vaNew);
    void shutdown(bool waitForTermination);
    bool isDisconnected();
    std::string toString();

  private:
    enum eCommand {
      EC_UNKNOWN,
      EC_GET_VARIABLE_VALUES,
      EC_SENDING_INTERESTING_VAR_UPDATE,
      EC_SENDING_COMMANDS,
      EC_DISCONNECT
    };
    
    IVDA::CriticalSection m_CSlastValues;
    IVDA::CriticalSection m_CScommandQueue;
    IVDA::CriticalSection m_CSClient;
    std::vector<std::string> m_commandQueue;
    std::vector<double> m_lastValues;
    bool m_bDataIsValid;
    std::map<const std::string, size_t> m_ValueMapping;
    std::shared_ptr<IVDA::TCPSocket> m_pClient;
    HASConfigPtr m_config;
    std::string m_remoteID;
    std::string m_receiveBuffer;
    std::shared_ptr<RemotePoolThread> m_poolThread;
    std::shared_ptr<AESCrypt> m_AESCryptIn;
    std::shared_ptr<AESCrypt> m_AESCryptOut;
    
    void sendUpdateToRemote();
    void init();
    
    void listenFunction(IVDA::Predicate pContinue);
    
    bool handleIncommingData(int8_t data);
    void processInitialization(const std::string& data);
    bool processData(const std::string& data);
    void updateInterestingVars(const std::string& data);
    void receiveCommands(const std::string& data);
    HASIRemote::eCommand splitIncommingCall(const std::string& data,
                                            std::string& tail) const;
    
    std::string decryptMessage(std::string message);
    std::string encryptMessage(const std::string& message);
  };

  
  typedef std::shared_ptr<HASIRemote> HASIRemotePtr;

}

#endif // HASIREMOTE_H


/*
 The MIT License
 
 Copyright (c) 2015 Jens Krueger
 
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
