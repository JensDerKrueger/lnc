#pragma once

#ifndef HASEXCEPTIONS_H
#define HASEXCEPTIONS_H

#include <stdexcept> // std::runtime_error

namespace HAS {
  class EHASBus : public std::runtime_error {
  public:
    explicit EHASBus (const std::string& what_arg) :
    std::runtime_error(what_arg) {}
    explicit EHASBus (const char* what_arg) :
    std::runtime_error(what_arg) {}
  };

  class ECryptError : public EHASBus {
  public:
    explicit ECryptError (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit ECryptError (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EDeviceInit : public EHASBus {
  public:
    explicit EDeviceInit (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit EDeviceInit (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EAccessDenied : public EHASBus {
  public:
    explicit EAccessDenied (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit EAccessDenied (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EDeviceParser : public EHASBus {
  public:
    explicit EDeviceParser (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit EDeviceParser (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EReportParser : public EHASBus {
  public:
    explicit EReportParser (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit EReportParser (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EDeviceNotFound : public EHASBus {
  public:
    explicit EDeviceNotFound (const std::string& what_arg) :
    EHASBus(what_arg) {}
    explicit EDeviceNotFound (const char* what_arg) :
    EHASBus(what_arg) {}
  };

  class EHASRemote : public std::runtime_error {
  public:
    explicit EHASRemote (const std::string& what_arg) :
    std::runtime_error(what_arg) {}
    explicit EHASRemote (const char* what_arg) :
    std::runtime_error(what_arg) {}
  };

  
  class ERemoteInit : public EHASRemote {
  public:
    explicit ERemoteInit (const std::string& what_arg) :
    EHASRemote(what_arg) {}
    explicit ERemoteInit (const char* what_arg) :
    EHASRemote(what_arg) {}
  };
  
  class ERemoteParser : public EHASRemote {
  public:
    explicit ERemoteParser (const std::string& what_arg) :
    EHASRemote(what_arg) {}
    explicit ERemoteParser (const char* what_arg) :
    EHASRemote(what_arg) {}
  };

}

#endif // HASEXCEPTIONS_H

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
