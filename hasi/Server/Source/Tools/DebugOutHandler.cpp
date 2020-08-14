#include "DebugOutHandler.h"

using namespace IVDA;

DebugOutHandler::DebugOutHandler()
  : m_DebugOut()
  , m_DefaultOut()
{

}

void DebugOutHandler::AddDebugOut(AbstrDebugOut* debugOut)
{
  if (debugOut != NULL) {
    m_DebugOut.AddDebugOut(debugOut);
    debugOut->Other(_IVDA_func_, "Connected to this debug out");
  } else {
    m_DebugOut.Warning(_IVDA_func_,
      "New debug is a NULL pointer, ignoring it.");
  }
}

void DebugOutHandler::RemoveDebugOut(AbstrDebugOut* debugOut)
{
  if (debugOut)
    debugOut->Other(_IVDA_func_, "Disconnecting from this debug out");
  m_DebugOut.RemoveDebugOut(debugOut);
}

/// Access the currently-active debug stream.
AbstrDebugOut* DebugOutHandler::DebugOut()
{
  return (m_DebugOut.empty())
    ? static_cast<AbstrDebugOut*>(&m_DefaultOut)
    : static_cast<AbstrDebugOut*>(&m_DebugOut);
}

const AbstrDebugOut *DebugOutHandler::DebugOut() const
{
  return (m_DebugOut.empty())
    ? static_cast<const AbstrDebugOut*>(&m_DefaultOut)
    : static_cast<const AbstrDebugOut*>(&m_DebugOut);
}