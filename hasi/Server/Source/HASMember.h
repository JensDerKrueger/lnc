#pragma once

#ifndef HASMEMBER_H
#define HASMEMBER_H

#include <string>    // std::string

#include <HASConfig.h>

namespace HAS {
  class HASMember {
  public:
    HASMember(const std::string& devID, const std::string& hrname,
              const HASConfigPtr config);
    virtual ~HASMember();
    virtual void init();
    std::string toString() const;
    const std::string& getID() const {return m_devID;}
    const std::string& getName() const {return m_hrname;}
    
    void setIsActive(bool isAcctive) {m_isActive = isAcctive;}
    bool getIsActive() const {return m_isActive;}

  protected:
    std::string m_devID;
    std::string m_hrname;
    const HASConfigPtr m_config;
    virtual std::string getDesc() const;
    
  private:
    bool m_isActive;
    
};

}

#endif // HASMEMBER_H

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

