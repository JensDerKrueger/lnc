#include "NetworkInputServer.h"
#include <HASExceptions.h>
#include <Script/ParserTools.h>
#include <Tools/SysTools.h>
#include <Tools/DebugOutHandler.h>

using namespace HAS;
using namespace IVDA;


NetworkInputServer* NetworkInputServer::deviceFromStrings(const std::vector<std::string>& entries,
                                                          HASConfigPtr config) {
  if (entries.size() == 5) {
    return new NetworkInputServer(entries[0], entries[1], config,
                                  SysTools::FromString<int>(entries[2]),
                                  SysTools::FromString<int>(entries[3]),
                                  entries[4]);
  }
  if (entries.size() == 4) {
    return new NetworkInputServer(entries[0], entries[1], config,
                                  SysTools::FromString<int>(entries[2]),
                                  SysTools::FromString<int>(entries[3]),
                                  "");
  }
  
  return nullptr;
}


NetworkInputServer::NetworkInputServer(const std::string& devID,
                                       const std::string& hrname,
                                       HASConfigPtr config,
                                       uint16_t port,
                                       uint8_t iChannelCount,
                                       const std::string& password) :
NetworkServer(devID, hrname, config, port, password),
m_iChannelCount(iChannelCount),
m_AESCryptIn(nullptr)
{
}

void NetworkInputServer::prepareShutdown() {
  NetworkServer::prepareShutdown();
}


uint32_t NetworkInputServer::pollAnalogIn() {
  if (m_Guard.Lock(2)) {
    m_dataDesc = m_dataDescFromClient;
    m_Guard.Unlock();
  } else {
    IVDA_WARNING("Lock timed out, poll returns out of date information.");
  }
  return 0;
}

uint8_t NetworkInputServer::getAnalogInChannelCount() const {
  return m_iChannelCount+NetworkServer::getAnalogInChannelCount();
}

float NetworkInputServer::getAnalog(uint8_t iChannel) {
  if (iChannel < NetworkServer::getAnalogInChannelCount())
    return NetworkServer::getAnalog(iChannel);
  else {
    iChannel -= NetworkServer::getAnalogInChannelCount();
    if (iChannel < m_dataDesc.size())
      return m_dataDesc[iChannel].value;
    else
      return 0;
  }
}

std::string NetworkInputServer::getAnalogChannelDesc(uint8_t iChannel) const {
  if (iChannel < NetworkServer::getAnalogInChannelCount())
    return NetworkServer::getAnalogChannelDesc(iChannel);
  else {
    iChannel -= NetworkServer::getAnalogInChannelCount();
    if (iChannel < m_dataDesc.size())
      return m_dataDesc[iChannel].desc;
    else
      return "";
  }
}

std::string NetworkInputServer::getAnalogChannelUnit(uint8_t iChannel) const {
  if (iChannel < NetworkServer::getAnalogInChannelCount())
    return NetworkServer::getAnalogChannelUnit(iChannel);
  else {
    iChannel -= NetworkServer::getAnalogInChannelCount();
    if (iChannel < m_dataDesc.size())
      return m_dataDesc[iChannel].unit;
    else
      return "";
  }
}

std::string NetworkInputServer::getDataDescString() const {
  std::stringstream ss;
  for (uint8_t i = 0;i<getAnalogInChannelCount();++i) {
    ss << " \""  << getAnalogChannelDesc(i) << "\""
    << " [" << getAnalogChannelUnit(i) << "]";
  }
  return ss.str();
}

std::string NetworkInputServer::getDesc() const {
  std::stringstream ss;
  ss << "Network input device ("<< (isConnected() ? "connected to" : "DISCONNECTED from") << " port: " << m_iPort << (m_password.empty() ? " in plaintext" : " encrypted") << "), delivering:\n" << getDataDescString();
  return ss.str();
}

void NetworkInputServer::processInitialization(const std::string& input_str) {
  std::string str = input_str;
  
  if (!m_password.empty()) {
    try {
      str = decryptFirstMessage(str);
    } catch (ECryptError const& e) {
      throw EDeviceInit(e.what());
    }
  }
  
  m_dataDescFromClient.clear();
  std::vector<std::string> lines = stringToLineList(str);
  for (auto l = lines.begin();l!=lines.end();++l) {
    DataDesc dd;
    
    std::vector<std::string> data;
    ParserTools::tokenize(*l,data, ",");
    
    if (data.size() != 2) {
      // std::stringstream ss;
      // ss << "Invalid initialization line ->" << *l << "<-";
      // throw EDeviceInit(ss.str());
      throw EDeviceInit("Invalid initialization line");
    }
    
    dd.desc = SysTools::TrimStr(data[0]);
    dd.unit = SysTools::TrimStr(data[1]);
    dd.value = 0.0;
    
    m_dataDescFromClient.push_back(dd);
  }
}

void NetworkInputServer::processData(const std::string& input_str) {
  std::string str = input_str;
  
  if (!m_password.empty()) {
    try {
      str = decryptMessage(str);
    }  catch (ECryptError const& e ) {
      throw EDeviceParser(e.what());
    }
  }
  
  std::vector<std::string> lines = stringToLineList(str);
  
  if (lines.size() > m_dataDescFromClient.size()) {
    std::stringstream ss;
    ss << "Too many data lines received for device " << getID()
       << ". Received " << lines.size()
       << " expected " << m_dataDesc.size();
    throw EDeviceParser(ss.str());
  }
  
  {
    SCOPEDLOCK(m_Guard);
    size_t index = 0;
    for (auto l = lines.begin();l!=lines.end();++l) {
      m_dataDescFromClient[index].value = SysTools::FromString<float>(*l);
      index++;
    }
  }
}

void NetworkInputServer::connectionInterrupted(const std::string& str) {
  NetworkServer::connectionInterrupted(str);
}

std::string NetworkInputServer::decryptFirstMessage(const std::string& input_message) {
  std::string message = input_message;
  
  size_t pos = message.find_first_of(";");
  
  if (pos == std::string::npos) {
    std::stringstream ss;
    ss << "No initialization vector found in init string";
    throw ECryptError(ss.str());
  }
  
  std::string strIV = IVDA::SysTools::TrimStr(message.substr(0,pos));
  
  SimpleVec iv;
  base64_decode(strIV, iv);
  
  m_AESCryptIn = std::make_shared<AESCrypt>(iv.constData(), m_password);
  
  message = message.substr(pos+1);
  return decryptMessage(message);
}

std::string NetworkInputServer::decryptMessage(const std::string& input_message) {
  std::string message = input_message;

  if(!m_AESCryptIn) {
    std::stringstream ss;
    ss << "decryptMessage called without a valid decrypter";
    throw ECryptError(ss.str());
  }
  message = m_AESCryptIn->decryptString(IVDA::SysTools::TrimStr(message));
  
  std::size_t pos = message.find(testHeader);
  if (pos==std::string::npos) {
    std::stringstream ss;
    ss << "OK token not found in encrypted string";
    throw ECryptError(ss.str());
  }

  if (pos+testHeader.length() < message.length()) {
    return IVDA::SysTools::TrimStr(message.substr(pos+testHeader.length()));
  } else {
    return "";
  }
  
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
