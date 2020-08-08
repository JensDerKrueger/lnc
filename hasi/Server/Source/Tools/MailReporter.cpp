#include "MailReporter.h"
#include "SysTools.h"
#include "DebugOutHandler.h"
#include <sstream>
#include <iomanip>      // std::setfill, std::setw, std::hex

MailReporter::MailReporter(const std::string& subject, const std::string& text, const std::string& htmlText) :
m_SenderAddress(""),
m_SenderName(""),
m_strSubject(subject),
m_strText(text),
m_strHTMLText(htmlText),
m_strCharset("UTF-8")   // iso-8859-1
{
}

MailReporter::~MailReporter()
{
  for (auto a = m_Attachments.begin();a<m_Attachments.end();++a) {
    if (a->bDelete) {
      remove(a->name.c_str());
    }
  }
}

void MailReporter::SetText(const std::string& body) {
  m_strText = body;
}

void MailReporter::SetHTMLText(const std::string& htmlBody) {
  m_strHTMLText = htmlBody;
}

void MailReporter::SetFrom(const std::string& address, const std::string& name) {
  m_SenderAddress = address;
  m_SenderName = name;
}

void MailReporter::AddAttachment(const std::string& filename, const std::string& type, bool bDeleteWhenDone) {
  Attachment a = {filename, type, bDeleteWhenDone};
  m_Attachments.push_back(a);
}

std::string MailReporter::QuotedPrintable(const std::string& txt) const {
  std::stringstream result;

  for (size_t i = 0;i<txt.length();++i) {
    int c = txt[i];
    if (c < 128) {
      result << txt[i];
    } else {
      if (c != 194)
        result << "=" << std::setfill('0') << std::setw(2) << std::hex << c;
    }
  }

  return result.str();
}

void MailReporter::SetCharset(const std::string& charset) {
  m_strCharset = charset;
}

void MailReporter::SendTo(const std::string& address, const std::string& name) const {
  if (!system(NULL)) {
    IVDA_ERROR("No command processor available, mail not send.");
    return;
  }

  unsigned int iAttachments = 0;
  for (auto a = m_Attachments.begin();a<m_Attachments.end();++a) {
    if (IVDA::SysTools::FileExists(a->name))
      iAttachments++;
  }

  std::stringstream ssCmd;
  ssCmd << "(\necho \"MIME-Version: 1.0\"\n";
  if (!m_SenderAddress.empty()) {
    if (!m_SenderName.empty()) {
      ssCmd << "echo \"From: " << m_SenderName << " <" << m_SenderAddress << ">\"\n";
    } else {
      ssCmd << "echo \"From: " << m_SenderAddress << "\n";
    }
  }

  if (!name.empty()) {
    ssCmd << "echo \"To: " << name << " <" << address << ">\"\n";
  } else {
    ssCmd << "echo \"To: " << address << "\"\n";
  }

  ssCmd << "echo \"Subject: " << m_strSubject << "\"\n";
  ssCmd << "echo 'Content-Type: multipart/mixed; boundary=\"mime-part-boundary\"'\n";
  ssCmd << "echo\n";
  ssCmd << "echo \"This is a MIME-encoded message. If you are seeing this, your mail reader is old.\"\n";
  ssCmd << "echo \"--mime-part-boundary\"\n";
  ssCmd << "echo 'Content-Type: text/plain; charset=\"" << m_strCharset << "\"'\n";
  ssCmd << "echo \"Content-Transfer-Encoding: quoted-printable\"\n";
  ssCmd << "echo \"Content-Disposition: inline\"\n";
  ssCmd << "echo\n";
  ssCmd << "echo \"" << QuotedPrintable(m_strText) << "\"\n";

  if (!m_strHTMLText.empty()) {
    ssCmd << "echo \"--mime-part-boundary\"\n";
    ssCmd << "echo 'Content-Type: text/html; charset=\"" << m_strCharset << "\"'\n";
    ssCmd << "echo \"Content-Transfer-Encoding: quoted-printable\"\n";
    ssCmd << "echo \"Content-Disposition: inline\"\n";
    ssCmd << "echo\n";
    ssCmd << "echo \"" << QuotedPrintable(m_strHTMLText) << "\"\n";
  }

  for (auto a = m_Attachments.begin();a<m_Attachments.end();++a) {
    if (IVDA::SysTools::FileExists(a->name)) {
      ssCmd << "echo \"--mime-part-boundary\"\n";
      ssCmd << "echo 'Content-Type: " << a->type << "; name=\""<< a->name << "\"'\n";
      ssCmd << "echo \"Content-Transfer-Encoding: base64\"\n";
      ssCmd << "echo 'Content-Disposition: attachment; filename=\""<< a->name << "\"'\n";
      ssCmd << "echo\n";
      ssCmd << "base64 \"" << a->name << "\"\n";
    }
  }
  ssCmd << "echo \"--mime-part-boundary--\"\n";
  ssCmd << ") | sendmail " << address;

  if (0 != system(ssCmd.str().c_str())) {
    IVDA_ERROR("Error executing mail send command");
  }
}

/*
 The MIT License

 Copyright (c) 2013-2015 Jens Krueger

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
