#pragma once

#ifndef IVDA_LOG_H
#define IVDA_LOG_H

#include "Singleton.h"
#include "DebugOut/MultiplexOut.h"
#include "DebugOut/ConsoleOut.h"
#include "Threads.h"
#include <sstream>

namespace IVDA
{
  class DebugOutHandler
  {
    SINGLETON_IVDA_HEAP(DebugOutHandler);
  public:

    /// Add another debug output
    /// @param debugOut      the new stream (will be deleted on RemoveDebugOut)
    void AddDebugOut(AbstrDebugOut* debugOut);

    /// Removes the given debug output stream.
    /// The stream must be the currently connected/used one.
    void RemoveDebugOut(AbstrDebugOut* debugOut);

    /// Access the currently-active debug stream.
    AbstrDebugOut* DebugOut();
    const AbstrDebugOut *DebugOut() const;

    IVDA::CriticalSection m_CSOutput;
    
  private:
    MultiplexOut m_DebugOut;
    ConsoleOut m_DefaultOut;
  };

} // namespace IVDA

#define IVDA_ERRORV(...)                                                                        \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    IVDA::DebugOutHandler::Instance().DebugOut()->Error(_IVDA_func_, __VA_ARGS__);              \
  } while( __LINE__ == -1)
#define IVDA_WARNINGV(...)                                                                      \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    IVDA::DebugOutHandler::Instance().DebugOut()->Warning(_IVDA_func_, __VA_ARGS__);            \
  } while( __LINE__ == -1)
#define IVDA_MESSAGEV(...)                                                                      \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    IVDA::DebugOutHandler::Instance().DebugOut()->Message(_IVDA_func_, __VA_ARGS__);            \
  } while( __LINE__ == -1)
#define IVDA_OTHERV(...)                                                                        \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    IVDA::DebugOutHandler::Instance().DebugOut()->Other(_IVDA_func_, __VA_ARGS__);              \
  } while( __LINE__ == -1)

#define IVDA_ERROR(ostream)                                                                     \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    std::stringstream ss;                                                                       \
    ss << ostream;                                                                              \
    IVDA::DebugOutHandler::Instance().DebugOut()->Error(_IVDA_func_, "%s", ss.str().c_str());   \
  } while( __LINE__ == -1)
#define IVDA_WARNING(ostream)                                                                   \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    std::stringstream ss;                                                                       \
    ss << ostream;                                                                              \
    IVDA::DebugOutHandler::Instance().DebugOut()->Warning(_IVDA_func_, "%s", ss.str().c_str()); \
  } while( __LINE__ == -1)
#define IVDA_MESSAGE(ostream)                                                                   \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    std::stringstream ss;                                                                       \
    ss << ostream;                                                                              \
    IVDA::DebugOutHandler::Instance().DebugOut()->Message(_IVDA_func_, "%s", ss.str().c_str()); \
  } while( __LINE__ == -1)
#define IVDA_OTHER(ostream)                                                                     \
  do {                                                                                          \
    SCOPEDLOCK(IVDA::DebugOutHandler::Instance().m_CSOutput);                                   \
    std::stringstream ss;                                                                       \
    ss << ostream;                                                                              \
    IVDA::DebugOutHandler::Instance().DebugOut()->Other(_IVDA_func_, "%s", ss.str().c_str());   \
  } while( __LINE__ == -1)


#endif // IVDA_LOG_H
