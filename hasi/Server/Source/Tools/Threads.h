#pragma once

#ifndef IVDA_THREADS_H
#define IVDA_THREADS_H

#include "StdDefines.h"
#include <memory>
#include <functional>
#include <cassert>
#include <string>

#ifdef DETECTED_OS_WINDOWS
#define NOMINMAX
#include <windows.h>
#else
#include <pthread.h>
#endif

// TODO
// add timeout to CriticalSection Lock() (use CreateMutex/WaitForSingleObject etc. for this on Windows)
// add exceptions (same style as in Sockets)

namespace IVDA
{
  class CriticalSection {
    friend class WaitCondition;
  public:
    CriticalSection();
    ~CriticalSection();
  
    void Lock();
    bool Lock(uint32_t secs);
    bool TryLock();
    void Unlock();
  
  protected:
#ifdef DETECTED_OS_WINDOWS
    CRITICAL_SECTION m_csIDGuard;
#else
    pthread_mutex_t m_csIDGuard;
#endif

  private:
    // object is non copyable, declare private c'tor and assignment operator to prevent compiler warnings
    CriticalSection(CriticalSection const&);
    CriticalSection& operator=(CriticalSection const&);
  };

  class ScopedLock {
  public:
    ScopedLock(CriticalSection& guard) : m_Guard(guard) { m_Guard.Lock(); }
    ScopedLock(CriticalSection* pGuard) : m_Guard(*pGuard) { assert(pGuard != nullptr); m_Guard.Lock(); }
    ScopedLock(std::shared_ptr<CriticalSection> const& pGuard) : m_Guard(*pGuard.get()) { assert(pGuard.get() != nullptr); m_Guard.Lock(); }
    ~ScopedLock() { m_Guard.Unlock(); }

  private:
    // object is non copyable, declare private c'tor and assignment operator to prevent compiler warnings
    ScopedLock(ScopedLock const& other);
    ScopedLock& operator=(ScopedLock const& other); 
    CriticalSection& m_Guard;
  };

#define SCOPEDLOCK(guard) IVDA::ScopedLock scopedLock(guard);

  class WaitCondition {
  public:
    WaitCondition();
    ~WaitCondition();

    bool Wait(CriticalSection& criticalSection, uint32_t timeoutInMilliseconds = INFINITE_TIMEOUT); // returns false if timeout occurred
    void WakeOne();
    void WakeAll();

  protected:
#ifdef DETECTED_OS_WINDOWS
    CONDITION_VARIABLE m_cvWaitCondition;
#else
    pthread_cond_t m_cvWaitCondition;
#endif

  private:
    // object is non copyable, declare private c'tor and assignment operator to prevent compiler warnings
    WaitCondition(WaitCondition const&);
    WaitCondition& operator=(WaitCondition const&);
  };

  class ThreadClass {
  public:
    ThreadClass(std::string const& strName = "ThreadClass");
    virtual ~ThreadClass(); // will kill thread if not joined in derived class d'tor

    virtual bool StartThread(void* pData = nullptr);
    bool JoinThread(uint32_t timeoutInMilliseconds = INFINITE_TIMEOUT);
    bool KillThread(); // only use in extreme cases at own risk

    void RequestThreadStop() { m_bContinue = false; Resume(); }
    bool IsThreadStopRequested() const { return !m_bContinue; } // indicates if thread should leave its run loop
    bool IsRunning(); // query thread state at OS level
    bool Resume();

    bool Continue() const { return m_bContinue; } // returns true if thread should proceed with its job
    Predicate GetContinue() const { return m_pContinue; } // predicate function that maps ThreadClass::Continue()

  protected:
    virtual void ThreadMain(void* pData = nullptr) = 0;
    bool Suspend(Predicate pPredicate = Predicate());

    CriticalSection   m_SuspendGuard;
#ifdef DETECTED_OS_WINDOWS
    HANDLE            m_hThread;
    std::string       m_strName;
#else
    pthread_t         m_hThread;
#endif

  private:
    // object is non copyable, declare private c'tor and assignment operator to prevent compiler warnings
    ThreadClass(ThreadClass const&);
    ThreadClass& operator=(ThreadClass const&);

    struct ThreadData {
      void* pData;
      ThreadClass* pThread;
    };
#ifdef DETECTED_OS_WINDOWS
    static DWORD WINAPI StaticStartFunc(LPVOID pThreadStarterData);
#else
    static void* StaticStartFunc(void* pThreadStarterData);
    bool              m_bInitialized;
    bool              m_bJoinable;
    CriticalSection   m_JoinMutex;
    WaitCondition     m_JoinWaitCondition;
#endif
    bool              m_bContinue;
    Predicate         m_pContinue;
    ThreadData*       m_pStartData;
    bool              m_bResumable;
    WaitCondition     m_SuspendWait;
  };

  class LambdaThread : public ThreadClass {
  public:
    // interface to be able to call protected thread methods from some function that gets executed by ThreadMain
    class Interface {
      friend class LambdaThread;
    public:
      bool Suspend(Predicate pPredicate = Predicate()) { return m_Parent.Suspend(pPredicate); }
      CriticalSection& GetSuspendGuard() { return m_Parent.m_SuspendGuard; }

    private:
      // object is non copyable, declare private c'tor and assignment operator to prevent compiler warnings
      Interface(Interface const&);
      Interface& operator=(Interface const&);

      Interface(LambdaThread& parent) : m_Parent(parent) {}
      LambdaThread& m_Parent;
    };

    typedef std::function<void (Predicate pContinue, Interface& threadInterface)> ThreadMainFunction;
    LambdaThread(ThreadMainFunction pMain, std::string const& strName = "LambdaThread") : ThreadClass(strName), m_pMain(pMain) {}

  protected:
    virtual void ThreadMain(void*) { Interface threadInterface(*this); m_pMain(GetContinue(), threadInterface); }

  private:
    ThreadMainFunction m_pMain;
  };

  template<class T>
  class AtomicAccess {
  public:
    AtomicAccess() : m_Value(), m_pGuard(new CriticalSection()) {}
    AtomicAccess(T const& value) : m_Value(value), m_pGuard(new CriticalSection()) {}
    AtomicAccess<T>& operator=(T const& rhs) { SetValue(rhs); return *this; }
    CriticalSection& GetGuard() const { return *(m_pGuard.get()); }
    T GetValue() const { SCOPEDLOCK(GetGuard()); return m_Value; }
    void SetValue(T const& value) { SCOPEDLOCK(GetGuard()); /*OUT_IVDA_MESSAGE("setting value from: %s to: %s", SysTools::ToString(m_Value).c_str(), SysTools::ToString(value).c_str());*/ m_Value = value; }

  private:
    T m_Value;
    std::unique_ptr<CriticalSection> m_pGuard;
  };
}

#endif // IVDA_THREADS_H
