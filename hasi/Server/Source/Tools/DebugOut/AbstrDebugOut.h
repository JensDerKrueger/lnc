#pragma once

#ifdef _MSC_VER
# define _IVDA_func_ __FUNCTION__
#elif defined(__GNUC__)
# define _IVDA_func_ __func__
#else
# warning "unknown compiler!"
# define _IVDA_func_ "???"
#endif

#include "../StdDefines.h"
#include <cstdarg>
#include <deque>
#include <string>
#include <array>

namespace IVDA {

class AbstrDebugOut {
  public:
    AbstrDebugOut() :
        m_bShowMessages(false),
#ifdef _DEBUG
        m_bShowWarnings(true),
#else
        m_bShowWarnings(false),
#endif
        m_bShowErrors(true),
        m_bShowOther(false),
        m_bDisplayDateAndTime(true)
    {
      for(size_t i=0; i < m_bRecordLists.size(); ++i) {
        m_bRecordLists[i] = false;
      }
    }

    virtual ~AbstrDebugOut() {
      for(size_t i=0; i < m_bRecordLists.size(); ++i) {
        m_bRecordLists[i] = false;
      }
    }
    enum DebugChannel {
      CHANNEL_NONE=0,
      CHANNEL_ERROR,
      CHANNEL_WARNING,
      CHANNEL_MESSAGE,
      CHANNEL_OTHER,
      CHANNEL_FINAL, ///< don't use, but must be the last one.
    };
    const char *ChannelToString(enum DebugChannel) const;
    bool Enabled(enum DebugChannel) const;

    virtual void printf(enum DebugChannel, const char* source,
                        const char* msg) = 0;
    virtual void printf(const char *s) const = 0;
    virtual void Other(const char *source, const char* format, ...);
    virtual void Message(const char* source, const char* format, ...);
    virtual void Warning(const char* source, const char* format, ...);
    virtual void Error(const char* source, const char* format, ...);

    void PrintErrorList();
    void PrintWarningList();
    void PrintMessageList();

    virtual void ClearErrorList()   { m_strLists[CHANNEL_ERROR].clear(); }
    virtual void ClearWarningList() { m_strLists[CHANNEL_WARNING].clear(); }
    virtual void ClearMessageList() { m_strLists[CHANNEL_MESSAGE].clear(); }

    virtual void SetListRecordingErrors(bool bRecord)   {m_bRecordLists[0] = bRecord;}
    virtual void SetListRecordingWarnings(bool bRecord) {m_bRecordLists[1] = bRecord;}
    virtual void SetListRecordingMessages(bool bRecord) {m_bRecordLists[2] = bRecord;}
    virtual bool GetListRecordingErrors()   {return m_bRecordLists[0];}
    virtual bool GetListRecordingWarnings() {return m_bRecordLists[1];}
    virtual bool GetListRecordingMessages() {return m_bRecordLists[2];}

    void SetOutput(bool bShowErrors, bool bShowWarnings, bool bShowMessages,
                   bool bShowOther);
    void GetOutput(bool& bShowErrors, bool& bShowWarnings, bool& bShowMessages,
                   bool& bShowOther) const;

    bool ShowMessages() const {return m_bShowMessages;}
    bool ShowWarnings() const {return m_bShowWarnings;}
    bool ShowErrors() const {return m_bShowErrors;}
    bool ShowOther() const {return m_bShowOther;}
    bool DisplayDateAndTime() const {return m_bDisplayDateAndTime;}

    virtual void SetShowMessages(bool bShowMessages);
    virtual void SetShowWarnings(bool bShowWarnings);
    virtual void SetShowErrors(bool bShowErrors);
    virtual void SetShowOther(bool bShowOther);
    virtual void SetDisplayDateAndTime(bool bDisplayDateAndTime);

    static std::string getTodayStr();
protected:
    bool                      m_bShowMessages;
    bool                      m_bShowWarnings;
    bool                      m_bShowErrors;
    bool                      m_bShowOther;
    bool                      m_bDisplayDateAndTime;

    std::array<bool, CHANNEL_FINAL> m_bRecordLists;
    std::array<std::deque<std::string>, CHANNEL_FINAL> m_strLists;

    void ReplaceSpecialChars(char* buff, size_t iSize) const;
};
}; // namespace IVDA

/*
 The MIT License
 
 Copyright (c) 2015-2018 Jens Krüger
 
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

