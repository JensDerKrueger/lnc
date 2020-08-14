#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "NetworkDevice.h"
#include <string>    // std::string
#include <Tools/Sockets.h>
#include <Tools/Threads.h>

namespace HAS {
  
  class NetworkServer : public NetworkDevice, public IVDA::ThreadClass {
  public:
    NetworkServer(const std::string& devID, const std::string& hrname,
                  HASConfigPtr config, uint16_t port,
                  const std::string& password);
    virtual ~NetworkServer();
    
    virtual void init();
    virtual void prepareShutdown();
    
  protected:
    virtual std::string getDesc() const;
    uint16_t m_iPort;
    
    std::shared_ptr<IVDA::TCPServer> m_pServer;
    std::shared_ptr<IVDA::TCPSocket> m_pClient;
    
    virtual void ThreadMain(void* pData);
    virtual void shutdownServer();

    virtual void processInitialization(const std::string& str) = 0;
    virtual void processData(const std::string& str) = 0;

    virtual bool isConnected() const;
    virtual bool hasData() const;
    virtual void connectionInterrupted(const std::string& str);
    
  private:
    void handleIncommingData(int8_t data);
    
    std::string m_receiveBuffer;
    bool m_bConnectionUninitialized;
    bool m_bHasData;
        
  };

}

#endif // NETWORKSERVER_H


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
