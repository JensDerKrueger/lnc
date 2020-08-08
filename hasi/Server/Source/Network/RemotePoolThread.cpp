#include "RemotePoolThread.h"
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

using namespace HAS;
using namespace IVDA;


RemotePoolThread::RemotePoolThread(HASConfigPtr config) :
IVDA::ThreadClass("RemotePoolThread"),
m_bIsBusy(true),
m_pMain(nullptr),
m_config(config),
m_bWorkContinue(false)
{
  m_pWorkContinue = std::bind(&RemotePoolThread::WorkContinue, this);
  StartThread();
  
  // wait (up to a second) for the thread to start
  for (uint32_t i = 0;i<20;++i) {
    if (!m_bIsBusy) return;
    IVDA::SysTools::msleep(100);
  }
}


RemotePoolThread::~RemotePoolThread() {
  
  if (IsRunning()) {
    uint32_t timeout = 0;
    if (m_config) {
      timeout = uint32_t(m_config->getRemoteTimeout()*1.5);
    } else {
      timeout = 1000;
      IVDA_WARNING("Config invalid, using default timeout");
    }
    
    stopWork(timeout);

    RequestThreadStop();
    JoinThread(timeout);

    if (IsRunning() ) {
      if (m_config && m_config->getDisplayRemoteActivity()) {
        IVDA_WARNING("HASIRemote not responding, killing thread.");
      }
      KillThread();
    }
  }
}

void RemotePoolThread::ThreadMain(void* pData) {
  while (Continue()) {
    m_bIsBusy = false;
    Suspend();
    if (Continue() && m_pMain != nullptr) {
      m_pMain(m_pWorkContinue);
      m_pMain = nullptr;
    }
  }
}

bool RemotePoolThread::startWork(RemotePoolThread::ThreadMainFunction pMain) {
  if (m_bIsBusy) return false;
  
  m_bWorkContinue = true;
  m_bIsBusy = true;
  m_pMain = pMain;
  
  Resume();
  
  return m_pMain != nullptr;
}

bool RemotePoolThread::stopWork(uint32_t totalTimeout) {
  m_bWorkContinue = false;  
  if (totalTimeout == 0) return m_bIsBusy;
  
  uint32_t retry = totalTimeout/200;
  
  uint32_t counter = 0;
  while (m_bIsBusy) {
    IVDA::SysTools::msleep(200);  // give the work some time to stop
    counter++;
    if (counter == retry)
      return false;
  }
  return true;
}
