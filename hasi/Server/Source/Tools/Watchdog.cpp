#include "Watchdog.h"

#ifndef NWDOG
#include <system_error>             // system_error, error_code, system_category
#include <fcntl.h>                  // O_RDWR, O_NOCTTY
#include <sys/ioctl.h>              // ioctl
#include <linux/watchdog.h>         // WDIOC_GETTIMEOUT, WDIOC_KEEPALIVE
#include <unistd.h>                 // open, write, close
#endif
#include <Tools/DebugOutHandler.h>  // IVDA_WARNING

Watchdog::Watchdog() :
  m_iTimeout(0),
  m_iFileHandle(0)
{
#ifndef NWDOG
  IVDA_MESSAGE("Hardware watchdog starting.");
  if ((m_iFileHandle = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "Unable to open watchdog device.");
  }

  int bootstatus=0;
  if (ioctl(m_iFileHandle, WDIOC_GETBOOTSTATUS, &bootstatus) == 0) {
    if (bootstatus) {
      IVDA_WARNING("Last reboot was caused by the watchdog.");
    }
  } else {
    throw std::system_error(std::error_code(errno, std::system_category()), "Unable to read boot status");
  }

  IVDA_MESSAGE("Hardware watchdog activated.");
  ioctl(m_iFileHandle, WDIOC_GETTIMEOUT, &m_iTimeout);
  sendKeepalive();
  IVDA_MESSAGE("Watchdog timeout is " << m_iTimeout << " seconds." );
#else
  IVDA_MESSAGE("Watchdog is disabled via build parameters." );
#endif
}

Watchdog::~Watchdog()
{
#ifndef NWDOG
  if (m_iFileHandle >= 0) {
    disable();
    close(m_iFileHandle);
  }
#endif
}

void Watchdog::disable()
{
#ifndef NWDOG
  if (1 == write(m_iFileHandle, "V", 1))
    IVDA_MESSAGE("Hardware watchdog deactivated.");
  else
    IVDA_WARNING("Unable to write to watchdog file.");
#endif
}

void Watchdog::sendKeepalive()
{
#ifndef NWDOG
  ioctl(m_iFileHandle, WDIOC_KEEPALIVE, 0);
#endif
}

int Watchdog::setTimeout(int iTimeout) {
#ifndef NWDOG
  if (ioctl(m_iFileHandle, WDIOC_SETTIMEOUT, &iTimeout) != 0) {
    IVDA_WARNING("Unable to set watchdog timeout.");
  }
  ioctl(m_iFileHandle, WDIOC_GETTIMEOUT, &m_iTimeout);
#else
  m_iTimeout = iTimeout;
#endif
  return m_iTimeout;
}


int Watchdog::getInterval() const
{
  return m_iTimeout;
}

int Watchdog::getSaveInterval() const
{
  // set the interval to 2/3s of the watchdog's timeout
  return int(m_iTimeout*0.66666);
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
