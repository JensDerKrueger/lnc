#pragma once

#include <chrono>
#include <string>    // std::string

#include <IAnalogIn.h>
#include <IAnalogOut.h>
#include "NetworkDevice.h"

#include <Tools/Threads.h>

namespace HAS {
  
  class HarmonyHubDevice : public NetworkDevice, public IAnalogIn,
  public IAnalogOut, public IVDA::ThreadClass {
  public:
    HarmonyHubDevice(const std::string& devID, const std::string& hrname,
                     HASConfigPtr config, const std::string& strURI,
                     const std::string& strEmail, const std::string& strPassword,
                     uint32_t iRefreshIntervall);
    virtual ~HarmonyHubDevice();
    
    virtual void init();
    

    // IAnalogIn Interface
    virtual uint8_t getAnalogInChannelCount() const;
    virtual uint32_t pollAnalogIn();
    virtual float getAnalog(uint8_t iChannel);
    virtual std::string getAnalogChannelDesc(uint8_t iChannel) const;
    virtual std::string getAnalogChannelUnit(uint8_t iChannel) const;
    
    // IAnalogOut Interface
    virtual uint8_t getAnalogOutChannelCount() const;
    virtual void setAnalog(uint8_t iChannel, float value);
    virtual void applyAnalogOut();
    
    static HarmonyHubDevice* deviceFromStrings(const std::vector<std::string>& entries,
                                               HASConfigPtr config);
    
    virtual void prepareShutdown();
    
  private:
    std::string m_strURI;
    int32_t     m_currentActivity;
    bool        m_ActivityChange;
    int32_t     m_targetActivity;
    std::chrono::high_resolution_clock::time_point m_lastStart;
    
    std::string m_strEmail;

    bool        m_bConnectionUninitialized;
    bool        m_bHasData;
    uint32_t    m_iRefreshIntervall;
    
    virtual std::string getDesc() const;
    virtual void ThreadMain(void* pData);
    
  protected:
    virtual bool isConnected() const;
    virtual bool hasData() const;
    
  };
  
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
