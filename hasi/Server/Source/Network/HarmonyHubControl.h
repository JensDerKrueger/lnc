#pragma once

#include <exception>
#include <string>
#include <Tools/Sockets.h>

class HarmonyException : public std::exception {
public:
  HarmonyException(const std::string& m) : msg(m) {}
  HarmonyException() throw() {}
  virtual ~HarmonyException() throw() {}
  const char* what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

#define TRANSFER_BUFF_SIZE 200000

class TransferClass {
protected:
  char databuffer[TRANSFER_BUFF_SIZE];
  
  uint32_t receiveDataMax(IVDA::IOSocket& s, int8_t * data,
                          uint32_t size, uint32_t timeout);
  
};

class ExtHarmonyAuth : public TransferClass {
public:
  ExtHarmonyAuth(uint32_t timeout,
                 const std::string& strUserEmail,
                 const std::string& strUserPassword);
  const std::string getToken() const;

private:
  uint32_t    m_timeout;
  std::string m_strUserEmail;
  std::string m_strUserPassword;
  std::string m_strAuthorizationToken;
  
  void requestAuth();
};
typedef std::shared_ptr<ExtHarmonyAuth> ExtHarmonyAuthPtr;

class HarmonyHubControl : public TransferClass {
public:
  HarmonyHubControl(uint32_t timeout,
                    const std::string& strHarmonyIP,
                    const std::string& extAuth="",
                    const std::string& intAuth="");
  void setInternalAuth(const std::string& intAuth);
  void setExternalAuth(const std::string& extHarmonyAuth);
  std::string requestInternalAuthorization();
  
  void setActivity(int32_t activity);
  int32_t getActivity();
  
  const std::string getURI() const {
    return m_strHarmonyURI;
  }

  bool checkHarmonyConnection() const;
  
private:
  uint32_t    m_timeout;
  std::string m_strHarmonyURI;
  std::string m_ExtHarmonyAuth;
  std::string m_IntHarmonyAuth;
  
  void connectToHarmony(IVDA::TCPSocket& socket);
  void genIntAuthorizationToken(IVDA::TCPSocket& authorizationcsocket);
  
  void startCommunication(IVDA::TCPSocket& communicationcsocket,
                          std::string strUserName, std::string strPassword);
  
};

/*
 Copyright (c) 2016 Jens Krueger
 
 Based on the HarmonyHubControl Project
 
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
