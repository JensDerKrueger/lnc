#pragma once

#ifndef MAILPLOTTER_H
#define MAILPLOTTER_H

#include "FilePlotter.h"
#include "Threads.h"
#include <sstream>

class Notification {
public:
  Notification();
  Notification(FilePlotter::IntervalType _t, int16_t _intervalLocation,
               const std::string& _title, const std::string& _body,
               const std::string& _receipient);

  static std::string notificatioIntervalToTex(FilePlotter::IntervalType t);
  std::string toString() const;

  FilePlotter::IntervalType t;
  int16_t intervalLocation;
  int16_t lastLocation;
  std::string title;
  std::string body;
  std::string receipient;
};

class MailPlotter : public FilePlotter {
public:
  MailPlotter(const std::string& strFilename);
  virtual ~MailPlotter();

  void AddMailNotification(const std::string& id,
                           const std::string& title,
                           const std::string& body,
                           const std::string& receipient,
                           FilePlotter::IntervalType t=FilePlotter::IT_Daily,
                           int8_t intervalLocation=0);
  virtual bool endLog();
  virtual std::string toString() const;

protected:
  std::vector<Notification> m_Notifications;
  IVDA::LambdaThread notificationChecker;

  void CheckNotifications(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface);

};

#endif // MAILPLOTTER_H

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

