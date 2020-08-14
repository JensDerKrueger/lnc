#pragma once

#ifndef LOGWATCHER_H
#define LOGWATCHER_H

#include <string>

#include <Tools/DebugOut/TextfileOut.h>
#include <Tools/DebugOutHandler.h>
#include <Tools/MailReporter.h>


class LogWatcher {
public:
  
  
  LogWatcher(const std::string& strFromAddress,
             const std::string& strSystemName,
             const std::string& strToAddress) :
    m_filename(""),
    m_strFromAddress(strFromAddress),
    m_strSystemName(strSystemName),
    m_strToAddress(strToAddress)
  {
    IVDA::DebugOutHandler::Instance().DebugOut()->SetOutput(true, true, true, true);
  }

  ~LogWatcher() {
    if (!m_filename.empty() && !m_strFromAddress.empty() && !m_strToAddress.empty()) {
      MailReporter rep("HAS Termination Event","Has is terminating, log is attached.");
      rep.AddAttachment(m_filename, "text/plain", true);
      rep.SetFrom(m_strFromAddress, m_strSystemName);
      rep.SendTo(m_strToAddress);
    }
  }
  
  void SetFilename(const std::string& filename) {
    m_filename = filename;
    IVDA::DebugOutHandler::Instance().AddDebugOut(new IVDA::TextfileOut(m_filename));
    IVDA::DebugOutHandler::Instance().DebugOut()->SetOutput(true, true, true, true);
  }

private:
  std::string m_filename;
  std::string m_strFromAddress;
  std::string m_strSystemName;
  std::string m_strToAddress;
  
};

#endif // LOGWATCHER_H

/*
   The MIT License

   Copyright (c) 2013 Jens Krueger

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
