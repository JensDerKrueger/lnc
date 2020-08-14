#pragma once

#ifndef REMOTELISTENER_H
#define REMOTELISTENER_H

#include <HASConfig.h>
#include "HASIRemote.h"
#include <Tools/Threads.h>  // LambdaThread, CriticalSection
#include <Tools/Sockets.h>
#include "RemotePoolThread.h"

#define INITIAL_THREAD_POOL_SIZE 10 
namespace HAS {
  
  class RemoteListener {
  public:
    RemoteListener();
    virtual ~RemoteListener();

    void init(HASConfigPtr config);
    void shutdown();
    void restart();

    void update(const VarStrAssignments& vaNew);
    std::vector<std::string> getCommands();

    size_t getRemoteCount() {
      if (!m_CSRemotes.TryLock()) return 0;
      size_t count = m_remotes.size();
      m_CSRemotes.Unlock();
      return count;
    }
    std::string toString();

  private:
    HASConfigPtr m_config;
    std::vector<HASIRemotePtr> m_remotes;
    IVDA::CriticalSection m_CSRemotes;
    std::shared_ptr<IVDA::LambdaThread> m_ConnectionThread;
    std::shared_ptr<IVDA::TCPServer> m_pServerSocket;
    std::vector<std::shared_ptr<RemotePoolThread>> m_threadPool;
    
    std::shared_ptr<RemotePoolThread> getIdleThread();
    
    void startConnectionThread();
    void shutdownConnectionThread();
    
    void initServer();
    void removeDisconnectedRemotes();
    
    void connectionThread(IVDA::Predicate pContinue,
                          IVDA::LambdaThread::Interface& threadInterface);    
  };
  
  typedef std::shared_ptr<RemoteListener> RemoteListenerPtr;

}

#endif // REMOTELISTENER_H


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
