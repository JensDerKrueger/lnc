#include "NetworkInputOutputServer.h"
#include <HASExceptions.h>
#include <Script/ParserTools.h>
#include <Tools/SysTools.h>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

using namespace HAS;
using namespace IVDA;

NetworkInputOutputServer* NetworkInputOutputServer::deviceFromStrings(const std::vector<std::string>& entries,
                                                                      HASConfigPtr config) {
  if (entries.size() == 7) {
    return new NetworkInputOutputServer(entries[0], entries[1], config,
                                        SysTools::FromString<int>(entries[2]),
                                        SysTools::FromString<int>(entries[3]),
                                        SysTools::FromString<int>(entries[4]),
                                        SysTools::FromString<int>(entries[5]),
                                        entries[6]);
  }
  if (entries.size() == 6) {
    return new NetworkInputOutputServer(entries[0], entries[1], config,
                                        SysTools::FromString<int>(entries[2]),
                                        SysTools::FromString<int>(entries[3]),
                                        SysTools::FromString<int>(entries[4]),
                                        SysTools::FromString<int>(entries[5]),
                                        "");
  }
  return nullptr;
}

NetworkInputOutputServer::NetworkInputOutputServer(const std::string& devID,
                                                   const std::string& hrname,
                                                   HASConfigPtr config,
                                                   uint16_t inPort,
                                                   uint8_t iInChannelCount,
                                                   uint16_t outPort,
                                                   uint8_t iOutChannelCount,
                                                   const std::string& password) :
NetworkInputServer(devID, hrname, config, inPort, iInChannelCount, password),
m_outPort(outPort),
m_iOutChannelCount(iOutChannelCount),
m_pOutServer(nullptr),
m_pOutClient(nullptr),
m_outConnector(nullptr),
m_AESCryptOut(nullptr),
m_bForceUpdate(true)
{
  m_bAnalogOutNeedsUpdate = true;
  m_outVal.resize(m_iOutChannelCount);
}


NetworkInputOutputServer::~NetworkInputOutputServer() {
  if (m_outConnector) {
    m_outConnector->RequestThreadStop();

    // wait a little longer then the TCP timeout to make sure 
    // even in the worst case there is still time for the
    // thread to close
    m_outConnector->JoinThread(uint32_t(m_iTimeout*1.5));

    // if all else fails, kill the thread
    if (m_outConnector->IsRunning()) {
      IVDA_WARNING(m_hrname << " (" << m_devID << ") outConnector join has timed out, killing thread.");
      m_outConnector->KillThread();
    }
  }
}


void NetworkInputOutputServer::prepareShutdown() {
  NetworkInputServer::prepareShutdown();
  if (m_outConnector) m_outConnector->RequestThreadStop();
}


uint8_t NetworkInputOutputServer::getAnalogOutChannelCount() const {
  return m_iOutChannelCount;
}

void NetworkInputOutputServer::setAnalog(uint8_t iChannel, float value) {
  if (m_OutGuard.Lock(2)) {
    m_outVal[iChannel] = value;
    m_bAnalogOutNeedsUpdate = true;
    m_OutGuard.Unlock();
  } else {
    IVDA_WARNING("setAnalog timed out, data change is lost.");
  }
}

void NetworkInputOutputServer::applyAnalogOut() {
  if (m_outConnector) m_outConnector->Resume();
  m_bAnalogOutNeedsUpdate = false;
}

void NetworkInputOutputServer::sendString(const std::string& str) {
  SCOPEDLOCK(m_OutClientGuard);
  std::string s = str + "\n";
  if (m_pOutClient) m_pOutClient->SendData((const int8_t*)(s.data()), uint32_t(s.length()), m_iTimeout);
}

void NetworkInputOutputServer::init() {
  NetworkServer::init();
  
  try {
    std::shared_ptr<TCPServer> pServer(new TCPServer());
    pServer->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
    pServer->SetNoDelay(false);
    pServer->SetNoSigPipe(true);
    pServer->SetReuseAddress(true);
    pServer->Bind(NetworkAddress(NetworkAddress::Any, m_outPort));
    pServer->Listen();
    pServer->GetLocalPort();
    m_pOutServer = pServer;
  } catch (SocketException const& e) {
    std::stringstream ss;
    ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
    throw EDeviceInit(ss.str());
  }
 
  m_outConnector = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&NetworkInputOutputServer::establishConnection, this, std::placeholders::_1, std::placeholders::_2)));
  m_outConnector->StartThread();
}

void NetworkInputOutputServer::sendInit() {
  
  std::stringstream ss;
  for (size_t i = 0;i<m_iOutChannelCount;++i) {
    if (i+1 < m_iOutChannelCount)
      ss << "Value " << i << ", Unit " << i << "\n";
    else
      ss << "Value " << i << ", Unit " << i;
  }
  std::string payload = ss.str();

  if (!m_password.empty()) {
    uint8_t iv[16];
    AESCrypt::genIV(iv);
    m_AESCryptOut = std::make_shared<AESCrypt>(iv, m_password);
    
    std::string strIV = base64_encode(iv, 16);

    payload = testHeader+payload;
    payload = strIV + std::string(";")+m_AESCryptOut->encryptString(payload);
  }
  
  sendString(dataBegin);
  sendString(payload);
  sendString(dataEnd);
}


void NetworkInputOutputServer::establishConnection(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  std::vector<float> outVal;
  {
    SCOPEDLOCK(m_OutGuard);
    outVal = m_outVal;
  }

  if (m_pOutServer == nullptr) {
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
  while (pContinue())
  {
    // accept connection
    try {
      TCPSocket* pClient = nullptr;
      while (pClient == nullptr) {
        
        for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
          while (!m_pOutServer->AcceptNewConnection((ConnectionSocket**)&pClient, subIntervalLength)) {
            if (!pContinue()) {
              shutdownOutServer();
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
        } catch (SocketException const&) {
          delete pClient;
          pClient = nullptr;
          continue;
        }
      }
      {
        SCOPEDLOCK(m_OutClientGuard);
        m_pOutClient.reset(pClient);
        m_pOutClient->SetNoSigPipe(true);
      }
    } catch (SocketException const&) {
      continue;
    }
    try {
      sendInit();
      
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " connected sender");
      }
      
    } catch (SocketException const&) {
      continue;
    }
    bool bJustConnected = true;
    
    bool bOkToContinue = false;
    {
      SCOPEDLOCK(m_OutClientGuard);
      bOkToContinue = m_pOutClient && m_pOutClient->IsConnected() && pContinue();
    }
    
    while (bOkToContinue) {
      bool bUpdate = m_bForceUpdate;
      {
        SCOPEDLOCK(m_OutGuard);
        if (outVal != m_outVal) {
          outVal = m_outVal;
          bUpdate = true;
        }
      }

      if (bUpdate || bJustConnected) {
        m_bForceUpdate = false;
        bJustConnected = false;
        try {
          std::stringstream ss;
          for (size_t i = 0;i<outVal.size();++i) {
            if (i < outVal.size()-1)
              ss << outVal[i] << "\n";
            else
              ss << outVal[i];
          }
          std::string payload = ss.str();
          
          if (!m_password.empty()) {
            payload = testHeader+payload;
            payload = m_AESCryptOut->encryptString(payload);
          }
          
          sendString(dataBegin);
          sendString(payload);
          sendString(dataEnd);
        } catch (SocketException const&) {
          break;
        }
      }

      threadInterface.Suspend(pContinue);
      
      {
        SCOPEDLOCK(m_OutClientGuard);
        bOkToContinue = m_pOutClient && m_pOutClient->IsConnected() && pContinue();
      }
      
    }

    connectionInterrupted("sender");
  }
  shutdownOutServer();
}


void NetworkInputOutputServer::shutdownOutServer()
{
  try {
    if (m_pOutServer) m_pOutServer->Close();
  } catch (SocketException const&  ) {
  }
  m_pOutServer = nullptr;
}

std::string NetworkInputOutputServer::getDesc() const {
  std::stringstream ss;
  ss << "Network input/output device ("<< (isConnected() ? "connected to" : "DISCONNECTED from") << " port: " << m_iPort << (m_password.empty() ? " in plaintext" : " encrypted") << "), delivering:\n" << getDataDescString();
  return ss.str();
}

void NetworkInputOutputServer::connectionInterrupted(const std::string& str) {
  NetworkInputServer::connectionInterrupted(str);
  
  std::shared_ptr<IVDA::TCPSocket> outClient = nullptr;
  {
    SCOPEDLOCK(m_OutClientGuard);
    outClient = m_pOutClient;
    m_pOutClient = nullptr;
  }
  // close socket if necessary before we handle the next client
  try {
    if (outClient) {
      outClient->Close();
    }
  } catch (SocketException const&) {
  }
  
  // make sender-thread aware of the interrupted connection
  applyAnalogOut();
}

void NetworkInputOutputServer::processData(const std::string& str) {
  NetworkInputServer::processData(str);
  
  // once we receive new data from the client, send current state
  // back as keep-alive signal
  m_bForceUpdate = true;
  applyAnalogOut();
}

/*
 The MIT License
 
 Copyright (c) 2013-2015 Jens Krueger
 
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
