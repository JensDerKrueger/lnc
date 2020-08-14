#pragma once

#include "../Tools/StdDefines.h"
#include <Tools/Threads.h>  // ThreadClass
#include <functional>       // std::function
#include <HASConfig.h>

namespace HAS {
  
  class RemotePoolThread : public IVDA::ThreadClass {
  public:
    RemotePoolThread(HASConfigPtr config);
    virtual ~RemotePoolThread();
    
    typedef std::function<void (IVDA::Predicate pContinue)> ThreadMainFunction;
    bool startWork(ThreadMainFunction pMain);
    bool stopWork(uint32_t totalTimeout);
    
    bool isBusy() const {return m_bIsBusy;}
    
  protected:
    virtual void ThreadMain(void* pData = nullptr);

  private:
    bool                m_bIsBusy;
    ThreadMainFunction  m_pMain;
    HASConfigPtr        m_config;
    IVDA::Predicate     m_pWorkContinue;
    bool                m_bWorkContinue;
    
    bool WorkContinue() const { return GetContinue() && m_bWorkContinue; }
    
  };
}