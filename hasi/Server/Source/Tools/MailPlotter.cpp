#include "MailPlotter.h"
#include "MailReporter.h"
#include "DebugOutHandler.h"
#include "SysTools.h"
#include <sstream>
#include <ctime>

using namespace std::placeholders;

MailPlotter::MailPlotter(const std::string& strFilename) :
  FilePlotter(strFilename),
  notificationChecker(std::bind(&MailPlotter::CheckNotifications, this, _1, _2))
{
  notificationChecker.StartThread();
  IVDA::SysTools::msleep(1000); // give the thread time to start and go to sleep
}

MailPlotter::~MailPlotter() {

  // first stop the notification thread and
  // wait until its finished
  if (notificationChecker.IsRunning()) {
    notificationChecker.RequestThreadStop();
    notificationChecker.JoinThread();
  }

  // then delete all the notification logs
  if (m_strFilenames.size() > 1) {
    for (auto f = m_strFilenames.begin()+1;f<m_strFilenames.end();++f) {
      remove(f->c_str());
    }
  }
}

std::string MailPlotter::toString() const {
  std::stringstream ss;
  ss << "Send notification to";
  for (auto n = m_Notifications.begin();n<m_Notifications.end();++n) {
    ss << "\n" << n->toString();
  }
  ss << "\n(" << FilePlotter::toString() << ")";
  return ss.str();
}

bool MailPlotter::endLog() {
  if (!FilePlotter::endLog()) return false;

  if (!m_Notifications.empty())
    notificationChecker.Resume();

  return true;
}

void MailPlotter::CheckNotifications(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  threadInterface.Suspend(pContinue);
  while (!pContinue || pContinue() ) {
    time_t t = time(0);
    tm *nun = localtime(&t);
    size_t index = 0;
    for (auto n = m_Notifications.begin();n<m_Notifications.end();++n) {
      bool bFireNotification = false;
      switch (n->t) {
        case FilePlotter::IT_Minutely : {
          if (nun->tm_sec >= n->intervalLocation && n->lastLocation != nun->tm_min) {
            bFireNotification = true;
            n->lastLocation = nun->tm_min;
          }
          break;
        }
        case FilePlotter::IT_Hourly : {
          if (nun->tm_min >= n->intervalLocation && n->lastLocation != nun->tm_hour) {
            bFireNotification = true;
            n->lastLocation = nun->tm_hour;
          }
          break;
        }
        case FilePlotter::IT_Daily : {
         if (nun->tm_hour >= n->intervalLocation && n->lastLocation != nun->tm_mday) {
            bFireNotification = true;
            n->lastLocation = nun->tm_mday;
          }
          break;
        }
        case FilePlotter::IT_Weekly : {
          if (nun->tm_wday >= n->intervalLocation && n->lastLocation != nun->tm_mday) {
            bFireNotification = true;
            n->lastLocation = nun->tm_mday;
          }
          break;
        }
        case FilePlotter::IT_Monthly : {
          if (nun->tm_mday >= n->intervalLocation && n->lastLocation != nun->tm_mon) {
            bFireNotification = true;
            n->lastLocation = nun->tm_mon;
          }
          break;
        }
        case FilePlotter::IT_Yearly : {
          if (nun->tm_yday >= n->intervalLocation && n->lastLocation != nun->tm_year) {
            bFireNotification = true;
            n->lastLocation = nun->tm_year;
          }
          break;
        }
        // this case is for IT_NEVER
        default : break;
      }

      if (bFireNotification) {
        std::vector<FileInfo> fi = plotImage(m_strFilenames[index+1],"report", 2000,1000);
        if (fi.empty()) {
          continue;
        }
        remove(m_strFilenames[index+1].c_str());

        std::stringstream info;
        info << n->body << "\n";
        MailReporter m(n->title);
        for (size_t i = 0;i<fi.size();++i) {
          m.AddAttachment(fi[i].name, "image/png", true);
          info << fi[i].desc << " min: " << fi[i].min << " max: " << fi[i].max << "\n";
        }
        m.SetText(info.str());
        m.SendTo(n->receipient);
      }
      index++;
    }
    threadInterface.Suspend(pContinue);
  }
}

void MailPlotter::AddMailNotification(const std::string& id, const std::string& title,const std::string& body,
                                      const std::string& receipient, FilePlotter::IntervalType t,
                                      int8_t intervalLocation) {
  Notification n(t, intervalLocation, title, body, receipient);
  m_Notifications.push_back(n);
  std::stringstream ss;
  ss << IVDA::SysTools::RemoveExt(m_strFilenames[0]) << "_" << id << "." << IVDA::SysTools::GetExt(m_strFilenames[0]);
  m_strFilenames.push_back(ss.str());
}



Notification::Notification() :
  t(FilePlotter::IT_Hourly),
  intervalLocation(0),
  lastLocation(-1)
{
}

Notification::Notification(FilePlotter::IntervalType _t, int16_t _intervalLocation,
               const std::string& _title, const std::string& _body,
               const std::string& _receipient) :
  t(_t),
  intervalLocation(_intervalLocation),
  lastLocation(-1),
  title(_title),
  body(_body),
  receipient(_receipient)
{
}

std::string Notification::notificatioIntervalToTex(FilePlotter::IntervalType t) {
  switch (t) {
    case FilePlotter::IT_Minutely: return "minutely";
    case FilePlotter::IT_Hourly:   return "hourly";
    case FilePlotter::IT_Daily :   return "daily";
    case FilePlotter::IT_Weekly:   return "weekly";
    case FilePlotter::IT_Monthly : return "monthly";
    case FilePlotter::IT_Yearly :  return "yearly";
    case FilePlotter::IT_Never :   return "never";
  }
  
#ifndef __clang__ // never reached but makes GCC happy
  return "";
#endif
  
}

std::string Notification::toString() const {
  std::stringstream ss;
  ss << "sending " << title << " " << notificatioIntervalToTex(t) << " @ " << intervalLocation << " to " << receipient;
  return ss.str();
}

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
