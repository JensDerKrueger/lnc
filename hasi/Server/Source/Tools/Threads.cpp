#include "Threads.h"
#include "DebugOutHandler.h"
#include "SysTools.h"

#ifndef DETECTED_OS_WINDOWS
#include <sys/time.h>
#include <signal.h>
#endif

namespace IVDA
{
  CriticalSection::CriticalSection() {
#ifdef DETECTED_OS_WINDOWS
    InitializeCriticalSection(&m_csIDGuard);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_csIDGuard, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
  }

  CriticalSection::~CriticalSection() {
#ifdef DETECTED_OS_WINDOWS
    DeleteCriticalSection(&m_csIDGuard);
#else
    pthread_mutex_destroy(&m_csIDGuard);
#endif
  }

  bool CriticalSection::Lock(uint32_t secs) {
    uint32_t iAttempts=0;
    do {
      if (TryLock()) return true;
      SysTools::msleep(10);
      iAttempts++;
    } while (secs*100 > iAttempts);
    return false;
  }

  void CriticalSection::Lock() {
#ifdef DETECTED_OS_WINDOWS
    EnterCriticalSection(&m_csIDGuard);
#else
    pthread_mutex_lock(&m_csIDGuard);
#endif
  }

  bool CriticalSection::TryLock() {
#ifdef _WIN32
    return TryEnterCriticalSection(&m_csIDGuard)!=0;
#else
    return pthread_mutex_trylock(&m_csIDGuard) == 0; 
#endif
  }

  void CriticalSection::Unlock() {
#ifdef DETECTED_OS_WINDOWS
    LeaveCriticalSection(&m_csIDGuard);
#else
    pthread_mutex_unlock (&m_csIDGuard);
#endif
  }

  WaitCondition::WaitCondition()
  {
#ifdef DETECTED_OS_WINDOWS
    InitializeConditionVariable(&m_cvWaitCondition);
#else
    pthread_cond_init(&m_cvWaitCondition, NULL);
#endif
  }

  WaitCondition::~WaitCondition()
  {
#ifndef DETECTED_OS_WINDOWS
    pthread_cond_destroy(&m_cvWaitCondition);
#endif
  }

  bool WaitCondition::Wait(CriticalSection& criticalSection, uint32_t timeoutInMilliseconds)
  {
    bool bWaitResult = false;
#ifdef DETECTED_OS_WINDOWS
    if (timeoutInMilliseconds != INFINITE_TIMEOUT)
      bWaitResult = (SleepConditionVariableCS(&m_cvWaitCondition, &criticalSection.m_csIDGuard, (DWORD)timeoutInMilliseconds) != 0);
    else
      bWaitResult = (SleepConditionVariableCS(&m_cvWaitCondition, &criticalSection.m_csIDGuard, INFINITE) != 0);    
#else
    if (timeoutInMilliseconds != INFINITE_TIMEOUT)
    {
      timeval tv;
      timespec ts;
      gettimeofday(&tv, NULL);

      uint64_t intervalToTimeoutInNanoseconds = (tv.tv_usec + uint64_t(timeoutInMilliseconds)*1000)*1000;
      time_t targetTimeSecondPart     = time_t(tv.tv_sec + intervalToTimeoutInNanoseconds/(1000*1000*1000));
      time_t targetTimeNanosecondPart = time_t(intervalToTimeoutInNanoseconds%(1000*1000*1000));
      
      ts.tv_sec  = targetTimeSecondPart;
      ts.tv_nsec = targetTimeNanosecondPart;

      bWaitResult = (pthread_cond_timedwait(&m_cvWaitCondition, &criticalSection.m_csIDGuard, &ts) == 0);
    }
    else
      bWaitResult = (pthread_cond_wait(&m_cvWaitCondition, &criticalSection.m_csIDGuard) == 0);
#endif
    return bWaitResult;
  }

  void WaitCondition::WakeOne()
  {
#ifdef DETECTED_OS_WINDOWS
    WakeConditionVariable(&m_cvWaitCondition);
#else
    pthread_cond_signal(&m_cvWaitCondition);
#endif
  }

  void WaitCondition::WakeAll()
  {
#ifdef DETECTED_OS_WINDOWS
    WakeAllConditionVariable(&m_cvWaitCondition);
#else
    pthread_cond_broadcast(&m_cvWaitCondition);
#endif
  }

  ThreadClass::ThreadClass(std::string const& strName)
    : m_SuspendGuard()
#ifdef DETECTED_OS_WINDOWS
    , m_hThread(NULL)
    , m_strName(strName)
#else
    // do not initialize m_hThread for pthreads!
    , m_bInitialized(false)
    , m_bJoinable(false)
#endif
    , m_bContinue(true)
    , m_pContinue()
    , m_pStartData(NULL)
    , m_bResumable(false)
  {
#ifndef DETECTED_OS_WINDOWS
	  (void)strName;
#endif
    m_pContinue = std::bind(&ThreadClass::Continue, this);
  }

#ifdef DETECTED_OS_WINDOWS
  // setting up proper thread names makes it a lot easier debugging multi-threaded
  // applications with Visual Studio Debugger
  // http://msdn.microsoft.com/en-us/library/xcb2z8hs%28v=vs.100%29.aspx
  //
  // Usage: SetThreadName (-1, "MainThread");
  //
  const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
  } THREADNAME_INFO;
#pragma pack(pop)

  // Note that the thread name is copied to the thread so that the memory for
  // the threadName parameter can be released.
  void SetThreadName(DWORD dwThreadID, char const* threadName)
  {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
      RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
  }
#endif

  bool ThreadClass::StartThread(void * pData)
  {
    if (!IsRunning())
    {
      m_bContinue = true;
      m_bResumable = false;

      delete m_pStartData;
      m_pStartData = new ThreadData;
      m_pStartData->pThread = this;
      m_pStartData->pData = pData;
#ifdef DETECTED_OS_WINDOWS
      if (m_hThread) CloseHandle(m_hThread);
      m_hThread = CreateThread(NULL, 0, StaticStartFunc, m_pStartData, NULL, 0);
      if (m_hThread) {
        if (!m_strName.empty() && IsDebuggerPresent()) {
          DWORD dwThreadID = GetThreadId(m_hThread);
          SetThreadName(dwThreadID, m_strName.c_str());
        }
        return true;
      }
#else
      m_bJoinable = false;
      
      int errorCode = pthread_create(&m_hThread, NULL, StaticStartFunc, (void*)m_pStartData);
      
      if (errorCode == 0)
      {
        m_bInitialized = true;
        m_bJoinable = true;
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        return true;
      } else {
        IVDA_ERROR("unable to create new thread (errorCode=" << errorCode << ")");
      }
#endif
    }
    delete m_pStartData;
    m_pStartData = NULL;
    return false;
  }

  bool ThreadClass::JoinThread(uint32_t timeoutInMilliseconds)
  {
#ifdef DETECTED_OS_WINDOWS
    // IsRunning() test is needed here so this behaves like pthread_join-based implementation below
    // pthread_join will fail on consecutive joins on the same thread (ESRCH, EINVAL) -> tested on Mac (return ESRCH, no such process)
    // while WaitForSingleObject returns success each time
    // consequently, JoinThread will return false if called on a thread not running anymore, even on first call
    if (IsRunning())
      return (WaitForSingleObject(m_hThread, (DWORD)timeoutInMilliseconds) == WAIT_OBJECT_0);
    else
      return false;
#else
    // behavior should be consistent with WaitForSingleObject like this
    // even in case of pthread_join returning immediately with EDEADLK, since we wait before calling it
    // but actually not sure what possible WaitForSingleObject error values are and in what way a deadlock may be handled
    // we assume a deadlock does not cause WaitForSingleObject to return immediately, which goes along with below pthread-based implementation
    bool performJoin = true;
    m_JoinMutex.Lock();
    if (m_bJoinable)
    {
      if (!m_JoinWaitCondition.Wait(m_JoinMutex, timeoutInMilliseconds)) performJoin = false;
    }
    // handle ultra-rare case when thread set itself to non joinable, but has not returned yet (see StaticStartFunc)
    // or thread has not set itself to joinable yet, but is already running (see StartThread)
    // second case could be handled by using m_join_mutex in StartThread, but this handles both, so not necessary
    // we then perform a join as we know the thread will return immediately
    else if (!IsRunning()) performJoin = false;
    m_JoinMutex.Unlock();
    
    if (performJoin)
      return (pthread_join(m_hThread, NULL) == 0);
    else
      return false;
#endif
  }

  bool ThreadClass::KillThread()
  {
#ifdef DETECTED_OS_WINDOWS
    return (TerminateThread(m_hThread, 0) != 0);
#else
    return (pthread_cancel(m_hThread) == 0);
#endif
  }

  bool ThreadClass::IsRunning() 
  {
#ifdef DETECTED_OS_WINDOWS
    // could use GetExitCodeThread here as well
    return (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT);
#else
    if (!m_bInitialized)
      return false;
    else
      return (pthread_kill(m_hThread, 0) == 0);
#endif
  }

  ThreadClass::~ThreadClass()
  {
    if (m_hThread)
    {
#if defined(DETECTED_OS_WINDOWS)
      if (IsRunning()) {
        IVDA_WARNINGV("Attempting to kill thread");
        if(!KillThread()) {
          IVDA_ERRORV("Could not kill thread.");
        } else {
          IVDA_WARNINGV("Thread successfully killed.");
        }
      }
      CloseHandle(m_hThread);
#endif
      delete m_pStartData;
      m_pStartData = NULL;
    }
  }

#ifdef DETECTED_OS_WINDOWS
  DWORD WINAPI ThreadClass::StaticStartFunc(LPVOID pThreadStarterData) 
  {
    ThreadData * d = (ThreadData*)pThreadStarterData;
    d->pThread->ThreadMain(d->pData);
    return 0;
  }
#else
  void * ThreadClass::StaticStartFunc(void * pThreadStarterData) 
  {
    ThreadData * d = (ThreadData*)pThreadStarterData;
    d->pThread->ThreadMain(d->pData);

    d->pThread->m_JoinMutex.Lock();
    d->pThread->m_bJoinable = false;
    d->pThread->m_JoinWaitCondition.WakeOne();
    d->pThread->m_JoinMutex.Unlock();

    return 0;
  }
#endif

  // we may want to have Resume public
  // otherwise, a thread calling Suspend can never be resumed using only ThreadClass methods
  // if both are public and we would use SuspendThread, Suspend would not block on Windows (SuspendThread does not block), but it would on other systems
  // so to be consistent, we use wait condition based implementation in general
  // also, there is no way of suspending a thread from another thread in linux without handling this within the thread main method or using SIGSTOP/SIGCONT (linux only, non-portable)
  // so only way this makes sense and behaves similar on Windows and linux, etc. is with Suspend protected and Resume public/protected
  bool ThreadClass::Suspend(Predicate pPredicate)
  {
    bool bSuspendResult = false;
    bool bSuspendable = false;

    m_SuspendGuard.Lock();
    // check predicate if available and resumable state
    if (pPredicate)
      bSuspendable = pPredicate() && !m_bResumable;
    else
      bSuspendable = !m_bResumable;
    // suspend if there is no work to do
    if (bSuspendable) {
      m_bResumable = true;
      bSuspendResult = m_SuspendWait.Wait(m_SuspendGuard);
    }
    m_SuspendGuard.Unlock();

    return bSuspendResult;
  }

  bool ThreadClass::Resume()
  {
    bool bResumeResult = false;

    m_SuspendGuard.Lock();
    // wake thread if it was suspended before
    if (m_bResumable) {
      m_bResumable = false;
      bResumeResult = true;
      m_SuspendWait.WakeOne();
    }
    m_SuspendGuard.Unlock();

    return bResumeResult;
  }

}
