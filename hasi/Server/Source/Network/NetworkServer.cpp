#include "NetworkServer.h"
#include <Tools/SysTools.h>
#include <HASExceptions.h>
#include <sstream>
#include <algorithm>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

using namespace IVDA;
using namespace HAS;

static const uint32_t maxMessageLength = 1000000;

NetworkServer::NetworkServer(const std::string& devID, const std::string& hrname,
                             HASConfigPtr config, uint16_t port,
                             const std::string& password) :
NetworkDevice(devID, hrname, config, password),
m_iPort(port),
m_pServer(nullptr),
m_pClient(nullptr),
m_receiveBuffer(),
m_bConnectionUninitialized(true),
m_bHasData(false)
{
}

NetworkServer::~NetworkServer() {
  RequestThreadStop();
  JoinThread();
}

void NetworkServer::prepareShutdown() {
  RequestThreadStop();
}

std::string NetworkServer::getDesc() const {
  std::stringstream ss;
  ss << "Unknown Network device ("<< (isConnected() ? "connected to" : "DISCONNECTED from") << " sort: " << m_iPort << (m_password.empty() ? " in plaintext" : " encrypted") << ")";
  return ss.str();
}


void NetworkServer::init() {
  if (m_config && m_config->getReportNetworkActivities()) {
    IVDA_MESSAGE(m_hrname << " (" << m_devID << ") init");
  }

  try {
    std::shared_ptr<TCPServer> pServer(new TCPServer());
    pServer->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
    pServer->SetNoDelay(false);
    pServer->SetReuseAddress(true);
    pServer->Bind(NetworkAddress(NetworkAddress::Any, m_iPort));
    pServer->Listen();
    pServer->GetLocalPort();
    m_pServer = pServer;
  } catch (SocketException const& e) {
    std::stringstream ss;
    ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
    throw EDeviceInit(ss.str());
  }
  
  this->StartThread();
}

bool NetworkServer::isConnected() const {
  return !m_bConnectionUninitialized && m_pClient != nullptr && m_pClient->IsConnected();
}

bool NetworkServer::hasData() const {
  return m_bHasData;
}

void NetworkServer::ThreadMain(void*)
{

  if (m_pServer == nullptr) {
    throw EDeviceInit("server not bound, call init() first!");
  }
  
  uint32_t subIntervalCount = std::max<uint32_t>(1,m_iTimeout/1000);
  uint32_t subIntervalLength = std::max<uint32_t>(1,m_iTimeout/subIntervalCount);
/*
  if (m_config->getReportNetworkActivities()) {
    IVDA_MESSAGE("Waiting for connection for " << m_iTimeout/1000.0f << "s is split up into "
                 << subIntervalCount << " subintervals of " << subIntervalLength << " ms length.");
  }
*/
  while (Continue())
  {
    m_bConnectionUninitialized = true;
    m_bHasData = false;
    m_bConnectPulseSend = false;
    
    // accept connection
    try {
      TCPSocket* pClient = nullptr;
      while (pClient == nullptr) {
        
        for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
          while (!m_pServer->AcceptNewConnection((ConnectionSocket**)&pClient, subIntervalLength)) {
            if (!Continue()) {
              shutdownServer();
              return;
            }
          }
          if (pClient != nullptr) break;
        }
        
        try {
          // check if peer already dropped the connection
          if (!pClient->IsConnected()) {
            pClient->Close();
            delete pClient;
            pClient = nullptr;
          }
        } catch (SocketException const& ) {
          delete pClient;
          pClient = nullptr;
        }
      }
      m_pClient.reset(pClient);
    } catch (SocketException const& ) {
    }

    try {
      while (m_pClient->IsConnected() && Continue()) {
        int8_t datum = 0;
        // receive one byte
        uint32_t const bytes = m_pClient->ReceiveData(&datum, 1, m_iTimeout);

        if (Continue() && bytes > 0) {
          handleIncommingData(datum);
        } else {
          break;
        }
      }
    } catch (SocketException const& )
    {
    }
    catch (EDeviceInit const& e)
    {
      IVDA_WARNING("Init Error: "
                   << m_hrname << " (" << e.what() << ")");
    }
    catch (EDeviceParser const& e)
    {
      IVDA_WARNING("Parse Error: "
                   << m_hrname << " (" << e.what() << ")");
    }
    
    // close socket if necessary before we handle the next client
    try {
      if (m_pClient) m_pClient->Close();
    } catch (SocketException const& ) {
    }
    m_pClient = nullptr;

    connectionInterrupted("receiver");
  }

  shutdownServer();
}

void NetworkServer::connectionInterrupted(const std::string& str) {
  if (m_config->getReportNetworkActivities()) {
    IVDA_MESSAGE(m_hrname << " disconnected " << str);
  }
}


void NetworkServer::shutdownServer()
{
  // shutdown server if not done from the main thread yet
  try {
    if (m_pServer) m_pServer->Close();
  } catch (SocketException const&  ) {
  }
  m_pServer = nullptr;
}

void NetworkServer::handleIncommingData(int8_t data) {
  if (data == '\r') return;
  
  m_receiveBuffer += data;

  // prevent a buffer overflow by limiting the max size of the buffer
  if (m_receiveBuffer.size() >= maxMessageLength) m_receiveBuffer = "";
  
  if (checkPostFix(m_receiveBuffer, dataBeginNL)) {
    // set receive buffer to dataBeginNL, possibly removing any leftover
    // data before that string
    m_receiveBuffer = dataBeginNL;
    return;
  }
  
  if (checkPostFix(m_receiveBuffer, dataEndNL)) {
    const std::string message = removeHeaderAndFooter(m_receiveBuffer);

    if (m_bConnectionUninitialized) {
      m_bConnectionUninitialized = false;
      processInitialization(message);
      
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " connected receiver");
      }
      
    } else {
      processData(message);
      m_bHasData = true;
    }
    
    m_receiveBuffer = "";
  }
}


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
