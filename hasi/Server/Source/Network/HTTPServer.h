#pragma once

#include <string>    // std::string
#include <map>       // std::map
#include <set>       // std::set

#include <HASConfig.h>
#include <Tools/Sockets.h>
#include <Tools/Threads.h>
#include <Script/VarAssignments.h>

namespace HAS {

  struct parameter {
    std::string name;
    std::string value;
  };
  
  struct urlRequest {
    std::string url;
    std::vector<parameter> parameters;
  };
  
  class HTTPServer  {
  public:
    HTTPServer(const HASConfigPtr config);
    virtual ~HTTPServer();
    
    void requestStop();
    void reloadWhitelist() {loadWhitelist();}
    bool isRunning() {return m_serverThread.IsRunning();}
    std::string syncState(const VarAssignments& vas);

  private:
    const HASConfigPtr m_config;
    IVDA::LambdaThread m_serverThread;
    std::shared_ptr<IVDA::TCPServer> m_pServer;
    std::shared_ptr<IVDA::TCPSocket> m_pClient;
    IVDA::CriticalSection m_CSInternalDataLock;
    uint32_t m_iTimeout;
    uint16_t m_iPort;

    VarAssignments m_InternalData;
    std::map<std::string, double> m_changes;
    std::set<std::string> m_whitelist;

    double getValue(const std::string& name);
    double setValue(const std::string& name, double v);
    
    bool reportDebugData();

    parameter parseParameter(const std::string& p);
    std::string parseRequest(const std::string& buffer);
    urlRequest parseURL(const std::string& r);
    double processRequest(const urlRequest& url);
    void respondToRequest(const std::string& request);
    bool processData(const std::string& buffer);
    void handleClients(IVDA::Predicate pContinue);
    void serverThread(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface);
    void loadWhitelist();
  };

}


/*
   The MIT License

   Copyright (c) 2018 Jens Krueger

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
