#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <cstring> // memset

#include "HarmonyHubControl.h"

#include <Tools/Base64.h>  // base64_encode
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

#define LOGITECH_AUTH_URL "https://svcs.myharmony.com/CompositeSecurityServices/Security.svc/json/GetUserAuthToken"
#define LOGITECH_AUTH_HOSTNAME "svcs.myharmony.com"
#define LOGITECH_AUTH_PATH "/CompositeSecurityServices/Security.svc/json/GetUserAuthToken"
#define HARMONY_COMMUNICATION_PORT 5222
#define CONNECTION_ID "12345678-1234-5678-1234-123456789012-1"

ExtHarmonyAuth::ExtHarmonyAuth(uint32_t timeout,
                               const std::string& strUserEmail,
                               const std::string& strUserPassword) :
m_timeout(timeout),
m_strUserEmail(strUserEmail),
m_strUserPassword(strUserPassword),
m_strAuthorizationToken("")
{
  requestAuth();
}

const std::string ExtHarmonyAuth::getToken() const {
  return m_strAuthorizationToken;
}

void ExtHarmonyAuth::requestAuth() {
  if(m_strUserEmail.empty() || m_strUserPassword.empty()) {
    throw HarmonyException("harmonyWebServiceLogin : Empty email or password provided");
  }
  
  // Build JSON request
  std::stringstream ssJSON;
  ssJSON << "{\"email\":\"" << m_strUserEmail << "\",\"password\":\"" << m_strUserPassword << "\"}";
  std::string strJSONText = ssJSON.str();
  
  
  IVDA::TCPSocket authcsocket;
  authcsocket.SetNonBlocking(true);
  authcsocket.SetNoDelay(true);
  authcsocket.SetKeepalive(false);
  authcsocket.SetNoSigPipe(true);
  
  IVDA::NetworkAddress address("svcs.myharmony.com", 80);
  try {
    authcsocket.Connect(address);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("harmonyWebServiceLogin: Connect failed");
  }
  
  if (!authcsocket.IsConnected()) {
    throw HarmonyException("harmonyWebServiceLogin : Unable to connect to Logitech server");
  }
  
  std::stringstream ssRequ;
  ssRequ << "POST " << LOGITECH_AUTH_URL << " HTTP/1.1\r\nHost: " << LOGITECH_AUTH_HOSTNAME
  << "\r\nAccept-Encoding: identity\r\nContent-Length: " << strJSONText.length()
  << "\r\ncontent-type: application/json;charset=utf-8\r\n\r\n";
  std::string strHttpRequestText = ssRequ.str();
  
  
  try {
    authcsocket.SendData((const int8_t*)(strHttpRequestText.data()), (uint32_t)strHttpRequestText.length());
    authcsocket.SendData((const int8_t*)(strJSONText.data()), (uint32_t)strJSONText.length());

    receiveDataMax(authcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("harmonyWebServiceLogin : Logitech web service connection interrupted");
  }
  
  std::string strHttpPayloadText = databuffer;/* <- Expect: 0x00def280 "HTTP/1.1 200 OK Server: nginx/1.2.4 Date: Wed, 05 Feb 2014 17:52:13 GMT Content-Type: application/json; charset=utf-8 Content-Length: 127 Connection: keep-alive Cache-Control: private X-AspNet-Version: 4.0.30319 X-Powered-By: ASP.NET  {"GetUserAuthTokenResult":{"AccountId":0,"UserAuthToken":"KsRE6VVA3xrhtbqFbh0jWn8YTiweDeB\/b94Qeqf3ofWGM79zLSr62XQh8geJxw\/V"}}"*/
  
  // Parse the login authorization token from the response
  std::string strAuthTokenTag = "UserAuthToken\":\"";
  size_t pos = strHttpPayloadText.find(strAuthTokenTag);
  
  if(pos == std::string::npos) {
    throw HarmonyException("harmonyWebServiceLogin : Logitech web service response does not contain a login authorization token");
  }
  
  m_strAuthorizationToken = strHttpPayloadText.substr(pos + strAuthTokenTag.length());
  pos = m_strAuthorizationToken.find("\"}}");
  m_strAuthorizationToken = m_strAuthorizationToken.substr(0, pos);
  
  // Remove forward slashes
  m_strAuthorizationToken.erase(std::remove(m_strAuthorizationToken.begin(), m_strAuthorizationToken.end(), '\\'), m_strAuthorizationToken.end());
}

HarmonyHubControl::HarmonyHubControl(uint32_t timeout,
                                     const std::string& strHarmonyIP,
                                     const std::string& extAuth,
                                     const std::string& intAuth) :
m_timeout(timeout),
m_strHarmonyURI(strHarmonyIP),
m_ExtHarmonyAuth(extAuth),
m_IntHarmonyAuth(intAuth)
{}

void HarmonyHubControl::setExternalAuth(const std::string& extHarmonyAuth) {
  m_ExtHarmonyAuth = extHarmonyAuth;
}

void HarmonyHubControl::setInternalAuth(const std::string& intAuth) {
  m_IntHarmonyAuth = intAuth;
}

std::string HarmonyHubControl::requestInternalAuthorization() {
  IVDA::TCPSocket authorizationcsocket;
  authorizationcsocket.SetNonBlocking(true);
  authorizationcsocket.SetNoDelay(true);
  authorizationcsocket.SetKeepalive(false);
  authorizationcsocket.SetNoSigPipe(true);
  
  connectToHarmony(authorizationcsocket);
  genIntAuthorizationToken(authorizationcsocket);
  
  return m_IntHarmonyAuth;
}

void HarmonyHubControl::setActivity(int32_t activity) {
  IVDA::TCPSocket commandcsocket;
  commandcsocket.SetNonBlocking(true);
  commandcsocket.SetNoDelay(true);
  commandcsocket.SetKeepalive(false);
  commandcsocket.SetNoSigPipe(true);
  
  connectToHarmony(commandcsocket);
  
  startCommunication(commandcsocket, m_IntHarmonyAuth, m_IntHarmonyAuth);
  
  if(m_IntHarmonyAuth.empty()) {
    throw HarmonyException("setActivity : Empty authorization token provided");
  }
  
  std::stringstream ss;
  
  ss << "<iq type=\"get\" id=\"" << CONNECTION_ID
  << "\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.harmony/vnd.logitech.harmony.engine?"
  << "startactivity\">activityId=" << activity << ":timestamp=0</oa></iq>";
  
  std::string sendData = ss.str();
  try {
    commandcsocket.SendData((const int8_t*)(sendData.data()), (uint32_t)sendData.length());
    receiveDataMax(commandcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("setActivity: connection interrupted");
  }


  std::string strData = databuffer; // <- Expect: strData  == <iq/>
  
  std::string iqTag = "<iq/>";
  size_t pos = strData.find(iqTag);
  
  if(pos != 0) {
    throw HarmonyException("setActivity: Invalid Harmony response");
  }
  
}

int32_t HarmonyHubControl::getActivity() {
  IVDA::TCPSocket commandcsocket;
  commandcsocket.SetNonBlocking(true);
  commandcsocket.SetNoDelay(true);
  commandcsocket.SetKeepalive(false);
  commandcsocket.SetNoSigPipe(true);
  
  connectToHarmony(commandcsocket);
  
  startCommunication(commandcsocket, m_IntHarmonyAuth, m_IntHarmonyAuth);
  
  if(m_IntHarmonyAuth.empty()) {
    throw HarmonyException("getActivity : Empty authorization token provided");
  }
  
  std::stringstream ss;
  
  ss << "<iq type=\"get\" id=\"" << CONNECTION_ID
  << "\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.harmony/vnd.logitech.harmony.engine?"
  << "getCurrentActivity\" /></iq>";
  
  std::string sendData = ss.str();;
  
  try {
    commandcsocket.SendData((const int8_t*)(sendData.data()), (uint32_t)sendData.length());
    receiveDataMax(commandcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("getActivity: connection interrupted");
  }
  
  std::string strData = databuffer; // <- Expect: strData  == <iq/>
  
  std::string iqTag = "<iq/>";
  size_t pos = strData.find(iqTag);
  
  if(pos != 0) {
    throw HarmonyException("getActivity: Invalid Harmony response");
  }
    
  size_t resultStartPos = strData.find("result=");
  size_t resultEndPos = strData.find("]]>");
  
  if(resultStartPos != std::string::npos && resultEndPos != std::string::npos) {
    return atoi(strData.substr(resultStartPos + 7, resultEndPos - resultStartPos - 7).c_str());
  }
  
  throw HarmonyException("getActivity: Invalid Harmony response");
}

bool HarmonyHubControl::checkHarmonyConnection() const {
  IVDA::TCPSocket testSocket;
  testSocket.SetNonBlocking(true);
  testSocket.SetNoDelay(true);
  testSocket.SetKeepalive(false);
  testSocket.SetNoSigPipe(true);
  
  IVDA::NetworkAddress address(m_strHarmonyURI, HARMONY_COMMUNICATION_PORT);
  try {
    testSocket.Connect(address);
  } catch (const IVDA::SocketException&) {
    return false;
  }
  
  return testSocket.IsConnected();
}

void HarmonyHubControl::connectToHarmony(IVDA::TCPSocket& socket) {
  if(m_strHarmonyURI.empty()) {
    throw HarmonyException("connectToHarmony : Empty Harmony IP Address or Hostname");
  }
  
  IVDA::NetworkAddress address(m_strHarmonyURI, HARMONY_COMMUNICATION_PORT);
  
  try {
    socket.Connect(address);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("connectToHarmony: Connect failed");
  }
  
  if (!socket.IsConnected()) {
    throw HarmonyException("connectToHarmony : Unable to connect to specified IP Address on specified Port");
  }
}

void HarmonyHubControl::genIntAuthorizationToken(IVDA::TCPSocket& authorizationcsocket) {
  if(m_ExtHarmonyAuth.empty()) {
    throw HarmonyException("genIntAuthorizationToken : NULL csocket or empty authorization token provided");
  }
  
  startCommunication(authorizationcsocket, "guest", "gatorade.");
  
  std::string strData;
  std::string sendData;
  
  // GENERATE A LOGIN ID REQUEST USING THE HARMONY ID AND LOGIN AUTHORIZATION TOKEN
  sendData = "<iq type=\"get\" id=\"";
  sendData.append(CONNECTION_ID);
  sendData.append("\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.connect/vnd.logitech.pair\">token=");
  sendData.append(m_ExtHarmonyAuth.c_str());
  sendData.append(":name=foo#iOS6.0.1#iPhone</oa></iq>");
  
  std::string strIdentityTokenTag = "identity=";
  size_t pos = std::string::npos;
  
  try {
    authorizationcsocket.SendData((const int8_t*)(sendData.data()), (uint32_t)sendData.length());
    receiveDataMax(authorizationcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("genIntAuthorizationToken: connection interrupted");
  }

  
  strData = databuffer; /* <- Expect: <iq/> ... <success xmlns= ... identity=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:status=succeeded ... */
  
  if(strData.find("<iq/>") != 0) {
    throw HarmonyException("genIntAuthorizationToken : Invalid Harmony response");
  }
  
  bool bIsDataReadable = false;
  bIsDataReadable = authorizationcsocket.WaitForConnected(m_timeout);
  if(!bIsDataReadable && strData == "<iq/>") {
    bIsDataReadable = true;
  }
  
  while(bIsDataReadable) {
    try {
      receiveDataMax(authorizationcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
    } catch (const IVDA::SocketException&) {
      throw HarmonyException("genIntAuthorizationToken : Logitech Harmony response connection interrupted");
    }
    strData.append(databuffer);
    bIsDataReadable = authorizationcsocket.WaitForConnected(m_timeout);
  };
  
  // Parse the session authorization token from the response
  pos = strData.find(strIdentityTokenTag);
  if(pos == std::string::npos) {
    throw HarmonyException("genIntAuthorizationToken : Logitech Harmony response does not contain a session authorization token");
  }
  
  m_IntHarmonyAuth = strData.substr(pos + strIdentityTokenTag.length());
  pos = m_IntHarmonyAuth.find(":status=succeeded");
  if(pos == std::string::npos) {
    throw HarmonyException("genIntAuthorizationToken : Logitech Harmony response does not contain a valid session authorization token");
  }
  m_IntHarmonyAuth = m_IntHarmonyAuth.substr(0, pos);
}

void HarmonyHubControl::startCommunication(IVDA::TCPSocket& communicationcsocket, std::string strUserName, std::string strPassword) {
  if(strUserName.length() == 0 || strPassword.length() == 0) {
    throw HarmonyException("startCommunication : Invalid communication parameter(s) provided");
  }
  
  // Start communication
  std::string data = "<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>";
  
  try {
    communicationcsocket.SendData((const int8_t*)(data.data()), (uint32_t)data.length());
    receiveDataMax(communicationcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("startCommunication : connection interrupted");
  }

  std::string strData = databuffer;/* <- Expect: <?xml version='1.0' encoding='iso-8859-1'?><stream:stream from='' id='XXXXXXXX' version='1.0' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'><stream:features><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms></stream:features> */
  
  data = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">";
  std::string tmp = "\0";
  tmp.append(strUserName);
  tmp.append("\0");
  tmp.append(strPassword);
  data.append(base64_encode((const unsigned char*)tmp.c_str(), uint32_t(tmp.length())));
  data.append("</auth>");
  

  try {
    communicationcsocket.SendData((const int8_t*)(data.data()), (uint32_t)data.length());
    receiveDataMax(communicationcsocket,(int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("startCommunication : connection interrupted");
  }
  
  strData = databuffer; /* <- Expect: <success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/> */
  if(strData != "<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>") {
    throw HarmonyException("startCommunication : connection error");
  }
  
  data = "<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>";
  
  try {
    communicationcsocket.SendData((const int8_t*)(data.data()), (uint32_t)data.length());
    receiveDataMax(communicationcsocket, (int8_t *)databuffer, TRANSFER_BUFF_SIZE, m_timeout);
  } catch (const IVDA::SocketException&) {
    throw HarmonyException("startCommunication : connection interrupted");
  }
  
  strData = databuffer; /* <- Expect: <?xml version='1.0' encoding='iso-8859-1'?><stream:stream from='' id='057a30bd' version='1.0' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'><stream:features><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms></stream:features> */
}

uint32_t TransferClass::receiveDataMax(IVDA::IOSocket& s, int8_t * data, uint32_t size, uint32_t timeout) {
  memset(data, 0, size); size--;  // reduce size by one to make sure an eventual string is always zero terminated
  uint32_t receivedTotal = 0;
  do {
    uint32_t received = s.ReceiveData(data+receivedTotal, 1, timeout);
    if (received == 0) return receivedTotal;
    receivedTotal += received;
  } while (receivedTotal < size);
  return receivedTotal;
}

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
