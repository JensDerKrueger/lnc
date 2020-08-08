#include "NetworkInputOutputClient.h"
#include <HASExceptions.h>
#include <Script/ParserTools.h>
#include <Tools/SysTools.h>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE

using namespace HAS;
using namespace IVDA;

NetworkInputOutputClient* NetworkInputOutputClient::deviceFromStrings(const std::vector<std::string>& entries, HASConfigPtr config) {
  if (entries.size() == 8) {
    return new NetworkInputOutputClient(entries[0], entries[1], config,
                                        entries[2],
                                        SysTools::FromString<int>(entries[3]),
                                        SysTools::FromString<int>(entries[4]),
                                        SysTools::FromString<int>(entries[5]),
                                        SysTools::FromString<int>(entries[6]),
                                        entries[7]);
  }
  if (entries.size() == 7) {
    return new NetworkInputOutputClient(entries[0], entries[1], config,
                                        entries[2],
                                        SysTools::FromString<int>(entries[3]),
                                        SysTools::FromString<int>(entries[4]),
                                        SysTools::FromString<int>(entries[5]),
                                        SysTools::FromString<int>(entries[6]),
                                        "");
  }
  return nullptr;
}

NetworkInputOutputClient:: NetworkInputOutputClient(const std::string& devID,
                                                    const std::string& hrname,
                                                    HASConfigPtr config,
                                                    const std::string& serverIP,
                                                    uint16_t inPort,
                                                    uint8_t iInChannelCount,
                                                    uint16_t outPort,
                                                    uint8_t iOutChannelCount,
                                                    const std::string& strPassword) :
NetworkDevice(devID, hrname, config, strPassword),
m_serverIP(serverIP),
m_iReceivePort(inPort),
m_iReceiveChannelCount(iInChannelCount),
m_iSendPort(outPort),
m_iSendChannelCount(iOutChannelCount),
m_bReceiverHasData(false),
m_bReceiverIsInitialized(false),
m_AESCryptIn(nullptr),
m_AESCryptOut(nullptr),
m_pReceiveClient(new TCPSocket()),
m_pSendClient(new TCPSocket()),
m_keepAliveThread(nullptr),
m_receiverThread(nullptr),
m_senderThread(nullptr)
{
  m_bAnalogOutNeedsUpdate = true;
  for (uint8_t i = 0;i<m_iReceiveChannelCount;++i) {
    DataDesc d = {"No Desc","No Unit",0.0f};
    m_receiveDataDesc.push_back(d);
  }
  for (uint8_t i = 0;i<m_iSendChannelCount;++i) {
    DataDesc d = {"RPi Value","No Unit",0.0f};
    m_sendDataDesc.push_back(d);
  }
}

NetworkInputOutputClient::~NetworkInputOutputClient() {
  if (m_keepAliveThread) m_keepAliveThread->RequestThreadStop();
  if (m_senderThread)    m_senderThread->RequestThreadStop();
  if (m_receiverThread)  m_receiverThread->RequestThreadStop();
  
  // wait a little longer then the TCP timeout to make sure 
  // even in the worst case there is still time for the
  // thread to close
  uint32_t shutdownTimeout = uint32_t(m_iTimeout*1.5);

  if (m_keepAliveThread) {
    m_keepAliveThread->JoinThread(shutdownTimeout);
  }
    
  if (m_senderThread) {
    m_senderThread->JoinThread(shutdownTimeout);
  }
    
  if (m_receiverThread) {
    m_receiverThread->JoinThread(shutdownTimeout);
  }
    
  // if all else fails, kill the threads
  if (m_keepAliveThread && m_keepAliveThread->IsRunning()) {
    IVDA_WARNING(m_hrname << " (" << m_devID << ") keepAliveThread join has timed out, killing thread.");
    m_keepAliveThread->KillThread();
  }
  if (m_senderThread && m_senderThread->IsRunning()) {
    IVDA_WARNING(m_hrname << " (" << m_devID << ") senderThread join has timed out, killing thread.");
    m_senderThread->KillThread();
  }
  if (m_receiverThread && m_receiverThread->IsRunning()) {
    IVDA_WARNING(m_hrname << " (" << m_devID << ") receiverThread join has timed out, killing thread.");
    m_receiverThread->KillThread();
  }
}


void NetworkInputOutputClient::prepareShutdown() {
  if (m_keepAliveThread) m_keepAliveThread->RequestThreadStop();
  if (m_senderThread)    m_senderThread->RequestThreadStop();
  if (m_receiverThread)  m_receiverThread->RequestThreadStop();  
}

void NetworkInputOutputClient::init() {
  if (m_config && m_config->getReportNetworkActivities()) {
    IVDA_MESSAGE(m_hrname << " (" << m_devID << ") init");
  }
  
  if (!m_senderThread) {
    m_senderThread =  std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&NetworkInputOutputClient::senderFunc, this, std::placeholders::_1, std::placeholders::_2)));
    m_senderThread->StartThread();
  }
  if (!m_receiverThread) {
    m_receiverThread = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&NetworkInputOutputClient::receiverFunc, this, std::placeholders::_1, std::placeholders::_2)));
    m_receiverThread->StartThread();
  }
  if (!m_keepAliveThread) {
    m_keepAliveThread = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&NetworkInputOutputClient::keepAliveFunc, this, std::placeholders::_1, std::placeholders::_2)));
    m_keepAliveThread->StartThread();
  }
}

void NetworkInputOutputClient::sendString(const std::string& str) {
  std::string s = str + "\n";
  m_pSendClient->SendData((const int8_t*)(s.data()), uint32_t(s.length()), m_iTimeout);
}

void NetworkInputOutputClient::sendInit() {
  std::stringstream ss;
  for (size_t i = 0;i<m_sendDataDesc.size();++i) {
    if (i < m_sendDataDesc.size()-1)
      ss << m_sendDataDesc[i].desc << "," << m_sendDataDesc[i].unit << "\n";
    else
      ss << m_sendDataDesc[i].desc << "," << m_sendDataDesc[i].unit;
  }
  std::string payload = ss.str();
  
  if (!m_password.empty()) {
    uint8_t iv[16];
    AESCrypt::genIV(iv);
    m_AESCryptOut = std::make_shared<AESCrypt>(iv, m_password);
    
    std::string strIV = base64_encode(iv, 16);

    payload = testHeader+payload;
    payload = strIV + std::string(";") + m_AESCryptOut->encryptString(payload);
  }
  
  
  sendString(dataBegin);
  sendString(payload);
  sendString(dataEnd);
}

void NetworkInputOutputClient::sendData() {
  std::vector<DataDesc> sendDataDesc;
  {
    SCOPEDLOCK(m_SendGuard);
    sendDataDesc = m_sendDataDesc;
  }

  std::stringstream ss;
  for (size_t i = 0;i<sendDataDesc.size();++i) {
    if (i < sendDataDesc.size()-1)
      ss << sendDataDesc[i].value << "\n";
    else
      ss << sendDataDesc[i].value;
  }
  std::string payload = ss.str();
  
  if (!m_password.empty()) {
    payload = testHeader+payload;
    payload = m_AESCryptOut->encryptString(payload);
  }

  sendString(dataBegin);
  sendString(payload);
  sendString(dataEnd);
}

void NetworkInputOutputClient::keepAliveFunc(IVDA::Predicate pContinue,
                                             IVDA::LambdaThread::Interface& threadInterface) {

  while (pContinue()) {
    
    // this is a delay(5000) broken down into 10 to speed up shutdown
    for (uint32_t i = 0;i<10;++i) {
      delay(500);
      if (!pContinue()) break;
    }
    
    if (m_pSendClient->IsConnected()) {
      applyAnalogOut();
    } else {
      if (m_senderThread) m_senderThread->Resume();
    }
  }
}


void NetworkInputOutputClient::senderFunc(IVDA::Predicate pContinue,
                                          IVDA::LambdaThread::Interface& threadInterface) {

  m_pSendClient->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
  m_pSendClient->SetNoDelay(true);
  m_pSendClient->SetKeepalive(false);
  m_pSendClient->SetNoSigPipe(true);
  NetworkAddress address(m_serverIP, m_iSendPort);
  
  while (pContinue())
  {
    try {
      if (m_pSendClient->IsConnected()) {
        m_pSendClient->Close();
        if (m_config->getReportNetworkActivities()) {
          IVDA_MESSAGE(m_hrname << " disconnected sender");
        }
      }
    } catch (SocketException const& ) {
    }
    
    uint32_t subIntervalCount = std::max<uint32_t>(1,m_iTimeout/1000);
    uint32_t subIntervalLength = std::max<uint32_t>(1,m_iTimeout/subIntervalCount);
/*
    if (m_config->getReportNetworkActivities()) {
      IVDA_MESSAGE("Connection attempt timeout of " << m_iTimeout/1000.0f << "s is split up into "
                   << subIntervalCount << " subintervals of " << subIntervalLength << " ms length.");
    }
*/
    // try to connect
    try {
      
      // The following lines are the long version of this one,
      // they break the long intervall into multiple shorter ones
      // while (pContinue() && !m_pSendClient->Connect(address, subIntervalLength)) {
      //   delay(100);
      // }

      for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
        while (pContinue() && !m_pSendClient->Connect(address, subIntervalLength)) {
          delay(100);
        }
        if (m_pSendClient->IsConnected() || (pContinue && !pContinue())) break;
      }
      
    } catch (SocketException const& ) {
      delay(100);
      
      // socket is dead, create a new one
      try {
        m_pSendClient = std::shared_ptr<IVDA::TCPSocket>(new TCPSocket());
        m_pSendClient->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
        m_pSendClient->SetNoDelay(true);
        m_pSendClient->SetKeepalive(false);
        m_pSendClient->SetNoSigPipe(true);
      } catch (SocketException const& ) {
      }
      continue;
    }

    // connected! send init
    try {
      sendInit();
      
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " connected sender");
      }
      
    } catch (SocketException const& ) {
      continue;
    }
    // send data loop
    try {
      while (m_pSendClient->IsConnected() && pContinue()) {
        sendData();
        threadInterface.Suspend(pContinue);
      }
    } catch (SocketException const& ) {
      continue;
    }
    
    if (m_config->getReportNetworkActivities()) {
      IVDA_MESSAGE(m_hrname << " disconnected sender");
    }
    
    // close socket if necessary before we handle the next connection
    try {
      m_pSendClient->Close();
    } catch (SocketException const&) {
      continue;
    }
  }
}

std::string NetworkInputOutputClient::decryptFirstMessage(const std::string& input_message) {
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

std::string NetworkInputOutputClient::decryptMessage(const std::string& input_message) {
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


void NetworkInputOutputClient::receiveInit(const std::string& input_data) {
  std::string data = input_data;
  
  if (!m_password.empty()) {
    try {
      data = decryptFirstMessage(data);
    } catch (ECryptError const& e ) {
      throw EDeviceInit(e.what());
    }
  }
  
  std::vector<std::string> lines = stringToLineList(data);
  std::vector<DataDesc> receiveDataDesc;
  for (auto l = lines.begin();l!=lines.end();++l) {
    DataDesc dd;
    
    std::vector<std::string> data;
    ParserTools::tokenize(*l,data, ",");
    
    if (data.size() != 2) {
//      std::stringstream ss;
//      ss << "Invalid initialization line ->" << *l << "<-";
      throw EDeviceInit("Invalid initialization line");
    }
    
    dd.desc = SysTools::TrimStr(data[0]);
    dd.unit = SysTools::TrimStr(data[1]);
    dd.value = 0.0f;
    
    receiveDataDesc.push_back(dd);
  }

  if (m_receiveDataDesc.size() == receiveDataDesc.size()) {
    m_receiveDataDesc = receiveDataDesc;
  } else {
    std::stringstream ss;
    ss << "Data item count mismatch. Expected " << m_receiveDataDesc.size()
       << " but received a description for " << receiveDataDesc.size();
    throw EDeviceInit(ss.str());
  }
  m_bReceiverIsInitialized = true;
  
}

void NetworkInputOutputClient::receiveData(const std::string& input_data) {
  std::string data = input_data;
  
  if (!m_password.empty()) {
    try {
      data = decryptMessage(data);
    } catch (ECryptError const& e ) {
      throw EDeviceParser(e.what());
    }
  }
  
  std::vector<std::string> lines = stringToLineList(data);

  if (lines.size() > m_receiveDataDesc.size()) {
    std::stringstream ss;
    ss << "Too many data lines received for client device " << getID()
    << ". Received " << lines.size()
    << " expected " << m_receiveDataDesc.size();
    throw EDeviceParser(ss.str());
  }

  {
    SCOPEDLOCK(m_ReceiveGuard);
    size_t index = 0;
    for (auto l = lines.begin();l!=lines.end();++l) {
      m_receiveDataDesc[index].value = SysTools::FromString<float>(*l);
      index++;
    }
  }
  m_bReceiverHasData = true;
}


bool NetworkInputOutputClient::receiveLoop(IVDA::Predicate pContinue, bool init) {
  while (m_pReceiveClient->IsConnected() && pContinue()) {
    int8_t datum = 0;
    uint32_t const bytes = m_pReceiveClient->ReceiveData(&datum, 1, m_iTimeout);

    if (pContinue() && bytes > 0) {
      handleIncommingData(datum, init);
      if (init && m_bReceiverIsInitialized) return true;
    } else {
      return false;
    }
  }
  return false;
}

void NetworkInputOutputClient::handleIncommingData(int8_t data, bool init) {
  if (data == '\r') return;
  
  m_receiveBuffer += data;
  
  if (checkPostFix(m_receiveBuffer, dataBeginNL)) {
    // set receive buffer to dataBeginNL, possibly removing any leftover
    // data before that string
    m_receiveBuffer = dataBeginNL;
    return;
  }
  
  if (checkPostFix(m_receiveBuffer, dataEndNL)) {
    const std::string message = removeHeaderAndFooter(m_receiveBuffer);
    
    if (init) {
      receiveInit(message);
    } else {
      receiveData(message);
    }
    
    m_receiveBuffer = "";
  }
}


void NetworkInputOutputClient::receiverFunc(IVDA::Predicate pContinue,
                                            IVDA::LambdaThread::Interface& threadInterface) {

  m_pReceiveClient->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
  m_pReceiveClient->SetNoDelay(true);
  m_pReceiveClient->SetKeepalive(false);
  m_pReceiveClient->SetNoSigPipe(true);
  NetworkAddress address(m_serverIP, m_iReceivePort);
  
  while (pContinue())
  {
    m_bReceiverHasData = false;
    m_bConnectPulseSend = false;
    m_bReceiverIsInitialized = false;
    try {
      if (m_pReceiveClient->IsConnected()) {
        m_pReceiveClient->Close();
      }
    } catch (SocketException const& ) {
    }

    uint32_t subIntervalCount = std::max<uint32_t>(1,m_iTimeout/1000);
    uint32_t subIntervalLength = std::max<uint32_t>(1,m_iTimeout/subIntervalCount);
/*
    if (m_config->getReportNetworkActivities()) {
      IVDA_MESSAGE("Connection attempt timeout of " << m_iTimeout/1000.0f << "s is split up into "
                   << subIntervalCount << " subintervals of " << subIntervalLength << " ms length.");
    }
*/
    // try to connect
    try {
      
      
      // The following lines are the long version of this one,
      // they break the long intervall into multiple shorter ones
      // while (pContinue() && !m_pReceiveClient->Connect(address, m_iTimeout)) {
      //   delay(100);
      // }
      
      for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
        while (pContinue() && !m_pReceiveClient->Connect(address, subIntervalLength)) {
          delay(100);
        }
        if (m_pReceiveClient->IsConnected() || (pContinue && !pContinue())) break;
      }
      
    } catch (SocketException const& ) {
      delay(100);

      // socket is dead, create a new one
      
      try {
        m_pReceiveClient = std::shared_ptr<IVDA::TCPSocket>(new TCPSocket());
        m_pReceiveClient->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
        m_pReceiveClient->SetNoDelay(true);
        m_pReceiveClient->SetKeepalive(false);
        m_pReceiveClient->SetNoSigPipe(true);
      } catch (SocketException const& ) {
      }
      
      continue;
    }

    // connected! receive init
    try {
      if (!receiveLoop(pContinue, true)) continue;
    } catch (SocketException const& ) {
      continue;
    } catch (EDeviceInit const& e ) {
      IVDA_ERROR("Error during handshake: " << e.what());
      continue;
    }
    
    // receive data loop
    try {
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " connected receiver");
      }
      receiveLoop(pContinue, false);
    } catch (SocketException const& ) {
    } catch (EDeviceParser const& e ) {
      IVDA_ERROR("Parser Error during data receive: " << e.what());
      continue;
    }

    // close socket if necessary before we handle the next connection
    try {
      m_pReceiveClient->Close();
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " disconnected receiver");
      }      
    } catch (SocketException const&) {
      if (m_config->getReportNetworkActivities()) {
        IVDA_MESSAGE(m_hrname << " disconnected receiver");
      }
      continue;
    }
  }
}

// IAnalogOut interface
uint8_t NetworkInputOutputClient::getAnalogOutChannelCount() const {
  return m_iSendChannelCount;
}

void NetworkInputOutputClient::setAnalog(uint8_t iChannel, float value) {
  if (iChannel >= m_iSendChannelCount) {
    std::stringstream ss;
    ss << "NetworkInputOutputClient::setAnalog: channel index " << int(iChannel)
    << " out of range [0-" << int(m_iSendChannelCount-1) << "]";
    throw std::out_of_range( ss.str() );
  }
  
  if (m_SendGuard.Lock(2)) {
    m_sendDataDesc[iChannel].value = value;
    m_bAnalogOutNeedsUpdate = true;
    m_SendGuard.Unlock();
  } else {
    IVDA_WARNING("setAnalog timed out, data change is lost");
  }
}

void NetworkInputOutputClient::applyAnalogOut() {
  if (m_senderThread) m_senderThread->Resume();
  m_bAnalogOutNeedsUpdate = false;
}

// IAnalogIn Interface
uint8_t NetworkInputOutputClient::getAnalogInChannelCount() const {
  return m_iReceiveChannelCount+NetworkDevice::getAnalogInChannelCount();
}

uint32_t NetworkInputOutputClient::pollAnalogIn() {
  // NOOP
  return 0;
}

float NetworkInputOutputClient::getAnalog(uint8_t iChannel) {
  if (m_ReceiveGuard.Lock(2)) {
    float value = 0.0f;
    if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
      value = NetworkDevice::getAnalog(iChannel);
    } else {
      iChannel -= NetworkDevice::getAnalogInChannelCount();
      if (iChannel < m_receiveDataDesc.size())
        value = m_receiveDataDesc[iChannel].value;
      else
        value = 0.0f;
    }
    m_ReceiveGuard.Unlock();
    return value;
  } else {
    IVDA_WARNING("getAnalog timed out, retuning 0.0");
    return 0.0f;
  }
}

std::string NetworkInputOutputClient::getAnalogChannelDesc(uint8_t iChannel) const {
  if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
    return NetworkDevice::getAnalogChannelDesc(iChannel);
  } else {
    iChannel -= NetworkDevice::getAnalogInChannelCount();
    if (iChannel < m_receiveDataDesc.size())
      return m_receiveDataDesc[iChannel].desc;
    else
      return "";
  }
}

std::string NetworkInputOutputClient::getAnalogChannelUnit(uint8_t iChannel) const {
  if (iChannel < NetworkDevice::getAnalogInChannelCount()) {
    return NetworkDevice::getAnalogChannelUnit(iChannel);
  } else {
    iChannel -= NetworkDevice::getAnalogInChannelCount();
    if (iChannel < m_receiveDataDesc.size())
      return m_receiveDataDesc[iChannel].unit;
    else
      return "";
  }
}

std::string NetworkInputOutputClient::getDesc() const {
  std::stringstream ss;
  ss << "Network input/output client ("<< (isConnected() ? "connected to" : "DISCONNECTED from") << " server: " << m_serverIP << " Ports: " << m_iSendPort << ", " << m_iReceivePort << (m_password.empty() ? " in plaintext" : " encrypted") << "), delivering:\n" << getDataDescString();
  return ss.str();
}

std::string NetworkInputOutputClient::getDataDescString() const {
  std::stringstream ss;
  for (uint8_t i = 0;i<getAnalogInChannelCount();++i) {
    ss << " \""  << getAnalogChannelDesc(i) << "\""
    << " [" << getAnalogChannelUnit(i) << "]";
  }
  return ss.str();
}


bool NetworkInputOutputClient::isConnected() const {
  return m_bReceiverIsInitialized;
}

bool NetworkInputOutputClient::hasData() const {
  return m_bReceiverHasData && isConnected();
}


/*
 The MIT License
 
 Copyright (c) 2014-2015 Jens Krueger
 
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
