#pragma once

#ifndef MAILREPORTER_H
#define MAILREPORTER_H

#include <string>
#include <vector>

struct Attachment {
  std::string name;
  std::string type;
  bool bDelete;
};

class MailReporter {
public:
  MailReporter(const std::string& subject, const std::string& text="",const std::string& htmlText="");
  ~MailReporter();
  void SetText(const std::string& body);
  void SetHTMLText(const std::string& body);
  void SetFrom(const std::string& address, const std::string& name="");
  void AddAttachment(const std::string& filename,
                     const std::string& type="application",
                     bool bDeleteWhenDone=false);
  void SendTo(const std::string& address, const std::string& name="") const;
  void SetCharset(const std::string& charset);
  
protected:
  std::string m_SenderAddress;
  std::string m_SenderName;
  std::string m_strSubject;
  std::string m_strText;
  std::string m_strHTMLText;
  std::string m_strCharset;
  std::vector<Attachment> m_Attachments;

  std::string QuotedPrintable(const std::string& txt) const;
};

#endif // MAILREPORTER_H

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

