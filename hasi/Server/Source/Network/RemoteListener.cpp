#include "RemoteListener.h"
#include <HASExceptions.h>

#include <Tools/SysTools.h>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

using namespace HAS;
using namespace IVDA;


RemoteListener::RemoteListener() :
m_config(nullptr),
m_ConnectionThread(nullptr),
m_pServerSocket(nullptr)
{
}

RemoteListener::~RemoteListener() {
  if (!m_threadPool.empty() ||
      !m_remotes.empty() || m_pServerSocket != nullptr
      || m_ConnectionThread != nullptr) {
    IVDA_WARNING("Shutdown called in destructor");
    shutdown();
  }
}

void RemoteListener::init(HASConfigPtr config) {
  m_config = config;

  if (m_config == nullptr || m_config->getRemotePort() == 0) {
    IVDA_MESSAGE("HASI Remotes disabled");
    return;
  }

  startConnectionThread();
}

std::string RemoteListener::toString() {
  SCOPEDLOCK(m_CSRemotes);
  std::string s = "";
  for (auto r : m_remotes)  {
    s += r->toString() + "\n";
  }
  return s;
}

void RemoteListener::initServer() {
  m_pServerSocket = nullptr;
  
  std::shared_ptr<TCPServer> pServer(new TCPServer());
  pServer->SetNonBlocking(m_config->getRemoteTimeout() == INFINITE_TIMEOUT ? false : true);
  pServer->SetNoDelay(false);
  pServer->SetReuseAddress(true);
  pServer->Bind(NetworkAddress(NetworkAddress::Any, m_config->getRemotePort()));
  pServer->Listen();

  // we don't use the result of this call, but we call it to make sure we
  // actually have a valid port assignment (otherwise this would throw an
  // exception)
  pServer->GetLocalPort();

  m_pServerSocket = pServer;
}

void RemoteListener::removeDisconnectedRemotes() {
  SCOPEDLOCK(m_CSRemotes);
  std::vector<HASIRemotePtr>::iterator iter;
  for (iter = m_remotes.begin(); iter != m_remotes.end(); ) {
    if ((*iter)->isDisconnected()) {
      (*iter)->shutdown(false);
      iter = m_remotes.erase(iter);
    } else
      ++iter;
  }
}

void RemoteListener::connectionThread(IVDA::Predicate pContinue,
                                      IVDA::LambdaThread::Interface&
                                      threadInterface) {

  if (m_pServerSocket == nullptr) {
    IVDA_ERROR("server not bound, call init() first!");
    return;
  }

  uint32_t subIntervalCount = std::max<uint32_t>(1,m_config->getRemoteConnectionInterval()/1000);
  uint32_t subIntervalLength = std::max<uint32_t>(1,m_config->getRemoteConnectionInterval()/subIntervalCount);
  
  /*
   if (m_config->getReportNetworkActivities()) {
   IVDA_MESSAGE("Wait time for remote connection of " << m_config->getRemoteConnectionInterval()/1000.0f << "s is split up into "
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
          while (!m_pServerSocket->AcceptNewConnection((ConnectionSocket**)&pClient, subIntervalLength)) {
            if (!pContinue()) {
              // we are supposed to terminate and only just realized it
              return;
            }
          }
          if (pClient != nullptr) break;
        }
        
        if (pContinue() && pClient == nullptr) {
          // periodically check if we have disconnected remotes
          removeDisconnectedRemotes();
        }

        if (pClient == nullptr) continue;
        
        pClient->SetNoSigPipe(true);

        // check if peer already dropped the connection
        try {
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

      if (pClient == nullptr) continue;
      
      // now it looks like we have a good connection, let's create the remote
      try {
        removeDisconnectedRemotes();

        // find idle thread in pool
        std::shared_ptr<RemotePoolThread> t = getIdleThread();
        
        
        SCOPEDLOCK(m_CSRemotes);
        std::shared_ptr<IVDA::TCPSocket> sp(pClient);
        HASIRemotePtr remote = std::make_shared<HASIRemote>(sp, t ,m_config);
        
        uint32_t counter = 0;
        while (remote->isDisconnected()) {
          IVDA::SysTools::msleep(200);  // give the thread some time to start
          counter++;
          
          if (counter == 20) {
            break;
          }
        }

        m_remotes.push_back(remote);
      } catch (SocketException const&) {
        continue;
      }
    } catch (SocketException const&) {
      continue;
    }
  }
}


std::shared_ptr<RemotePoolThread> RemoteListener::getIdleThread() {
  for (uint32_t i = 0;i<m_threadPool.size();i++) {
    if (!m_threadPool[i]->IsRunning()) {
      IVDA_WARNING("detected terminated thread in pool, replacing with new thread");
      std::shared_ptr<RemotePoolThread> t = std::make_shared<RemotePoolThread>(m_config);
      m_threadPool[i] = t;
      return t;
    }
    if (!m_threadPool[i]->isBusy())
      return m_threadPool[i];
  }
  
  // all threads in the pool are busy, increase pool size
  std::shared_ptr<RemotePoolThread> t = std::make_shared<RemotePoolThread>(m_config);
  m_threadPool.push_back(t);
  return t;
}

void RemoteListener::restart() {
  shutdown();
  startConnectionThread();
}

void RemoteListener::startConnectionThread() {
  m_threadPool.clear();
  for (uint32_t i = 0;i<INITIAL_THREAD_POOL_SIZE;i++) {
    std::shared_ptr<RemotePoolThread> t = std::make_shared<RemotePoolThread>(m_config);
    m_threadPool.push_back(t);
  }
  
  try {
    initServer();
  } catch (SocketException const& e) {
    std::stringstream ss;
    ss << "Unable to start Remote Listener. SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
    return;
  }
  
  m_ConnectionThread = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&RemoteListener::connectionThread, this, std::placeholders::_1, std::placeholders::_2)));
  m_ConnectionThread->StartThread();
  IVDA_MESSAGE("Listening to remotes on port " << m_config->getRemotePort());
}

void RemoteListener::shutdown() {
  if (m_config == nullptr || m_config->getRemotePort() == 0) return;
  
  shutdownConnectionThread();
  
  // disconnect remotes, if any
  if (!m_remotes.empty()) {
    if (m_remotes.size() == 1)
      IVDA_MESSAGE("Initiating shutdown of one remote.");
    else
      IVDA_MESSAGE("Initiating shutdown of " << m_remotes.size() << " remotes.");
    
    for (auto r : m_remotes)  {
      r->shutdown(false);  // at ths point send shutdown to all but don't wait
    }
    
    m_remotes.clear();
  }
  
  // Shutdown connection socket
  try {
    if (m_pServerSocket) m_pServerSocket->Close();
  } catch (SocketException const&  e) {
    IVDA_WARNING("Unable to properly close the connection socket. SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")");
  }
  m_pServerSocket = nullptr;
  
  // Empty thread pool
  m_threadPool.clear();  // now wait for all connections to terminate
}

void RemoteListener::shutdownConnectionThread() {

  if (m_ConnectionThread) {
    
    if (m_ConnectionThread->IsRunning() )
      IVDA_MESSAGE("Waiting for remote connection thread to finish");
    else
      IVDA_MESSAGE("Cleaning up remote connection thread");
    
    m_ConnectionThread->RequestThreadStop();
    m_ConnectionThread->JoinThread(uint32_t(m_config->getRemoteTimeout()*1.5));
    
    if (m_ConnectionThread->IsRunning()) {
      IVDA_WARNING("Remote connection thread not responding, killing thread.");
      m_ConnectionThread->KillThread();
    }    
    m_ConnectionThread = nullptr;
    
    IVDA_MESSAGE("Remote connection thread terminated");
  }

}

void RemoteListener::update(const VarStrAssignments& vaNew) {
  if (!m_CSRemotes.TryLock()) return;
  for (auto r : m_remotes)  {
    r->update(vaNew);
  }
  m_CSRemotes.Unlock();
}

std::vector<std::string> RemoteListener::getCommands() {
  std::vector<std::string> cc;

  if (!m_CSRemotes.TryLock()) return cc;

  for (auto r : m_remotes)  {
    std::vector<std::string> a = r->getCommands();
    cc.insert(cc.end(), a.begin(), a.end());
  }
  m_CSRemotes.Unlock();
  
  return cc;
}

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
