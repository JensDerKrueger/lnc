#include "HASIRemote.h"
#include <HASExceptions.h>
#include <sstream>

#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include <Script/ParserTools.h> // tokenize, removeSpaces

using namespace HAS;
using namespace IVDA;

static const std::string testHeader = "MESSAGE_OK";
static const std::string dataBegin ="DATA BEGIN";
static const std::string dataEnd ="DATA END";
static const std::string dataBeginNL = dataBegin + "\n";
static const std::string dataEndNL = dataEnd + "\n";
static const uint32_t maxMessageLength = 1000000;

static bool checkPostFix(std::string str, const std::string& pf) {
  if (str.length() < pf.length()) return false;
  for (size_t i = 0;i<pf.length();++i) {
    if (pf[i] != str[i + str.length() - pf.length()]) return false;
  }
  
  return true;
}

static std::string removeHeaderAndFooter(std::string str) {
  if (str.length() >= dataBeginNL.length()+dataEndNL.length())
    return str.substr(dataBeginNL.length(), str.length() -
                      (dataBeginNL.length()+dataEndNL.length()));
  else
    return "";
}

HASIRemote::HASIRemote(std::shared_ptr<IVDA::TCPSocket> pClient,
                       std::shared_ptr<RemotePoolThread> poolThread,
                       HASConfigPtr config) :
  m_bDataIsValid(false),
  m_pClient(pClient),
  m_config(config),
  m_remoteID(""),
  m_poolThread(poolThread),
  m_AESCryptIn(nullptr),
  m_AESCryptOut(nullptr)
{
  init();
}

HASIRemote::~HASIRemote(){
  shutdown(true);
}

std::string HASIRemote::toString() {
  std::stringstream ss;
  ss << m_remoteID;

  if (isDisconnected()) {
    ss << " (not connected)";
  }

  return ss.str();
}

std::vector<std::string> HASIRemote::getCommands() {
  SCOPEDLOCK(m_CScommandQueue);
  std::vector<std::string> cc = m_commandQueue;
  m_commandQueue.clear();
  return cc;
}

void HASIRemote::update(const VarStrAssignments& varAssignemts){
  if (isDisconnected()) return;
  
  SCOPEDLOCK(m_CSlastValues);
  m_bDataIsValid = true;
  
  for (auto var = varAssignemts.begin();
       var != varAssignemts.end();
       var++) {
    const std::string& name = var->name;
    
    auto iter = m_ValueMapping.find(name);
    if (iter != m_ValueMapping.end()) {
      if (m_lastValues[iter->second] != var->value) {
        m_lastValues[iter->second] = var->value;
      }
    }
  }
}

void HASIRemote::shutdown(bool waitForTermination) {
  if (m_poolThread->isBusy()) {
    if (m_config->getDisplayRemoteActivity()) {
      IVDA_MESSAGE("Waiting for remote listener " << m_remoteID << " to finish");
    }
    m_poolThread->stopWork(waitForTermination ? uint32_t(m_config->getRemoteTimeout()*1.5) : 0);
  }
}

void HASIRemote::init() {
  if (!m_poolThread->startWork(std::bind(&HASIRemote::listenFunction, this, std::placeholders::_1))) {
    IVDA_ERROR("Unable to start HASI Remote Thread");
  }
}

void HASIRemote::listenFunction(IVDA::Predicate pContinue)
{
  try {
    uint32_t misses = 0;
    
    
    while (pContinue() && m_pClient->IsConnected()) {
      int8_t datum = 0;
      
      // recieve one byte
      uint32_t const bytes = m_pClient->ReceiveData(&datum, 1, m_config->getRemoteTimeout());
      
      if (pContinue() && bytes > 0) {
        misses = 0;
        if (!handleIncommingData(datum)) {
          if (m_remoteID.empty() && m_config->getDisplayRemoteActivity()) {
            IVDA_WARNING("handleIncommingData returned false during init");
          }
          return;
        }
      } else {
        misses++;
        if (misses > m_config->getMaxRemoteTimeoutsToKeepAlive()) {
          break;
        }
      }
    }
    
  } catch (SocketException const&)
  {
    if (m_config->getDisplayRemoteActivity()) {
      IVDA_MESSAGE("Disconnected remote " << m_remoteID);
    }    
  }
  catch (ERemoteInit const& e)
  {
    if (m_config->getDisplayRemoteActivity()) {
      IVDA_WARNING("Remote init failed: " << e.what());
    }
  }
  catch (ERemoteParser const& e)
  {
    if (m_config->getDisplayRemoteActivity()) {
      IVDA_WARNING("Remote message invalid: " << e.what());
    }
  }

  if (m_pClient && m_pClient->IsConnected()) {
    SCOPEDLOCK(m_CSClient);
    // close socket if necessary before we terminate
    try {
      m_pClient->Close();
    } catch (SocketException const& ) {
    }
    m_pClient = nullptr;
    
    if (m_config->getDisplayRemoteActivity()) {
      IVDA_MESSAGE("Disconnected remote " << m_remoteID);
    }
  }
  
}

void HASIRemote::updateInterestingVars(const std::string& data) {
  SCOPEDLOCK(m_CSlastValues);
  
  m_bDataIsValid = false;

  std::vector<std::string> vars;
  ParserTools::tokenize(data, vars, ",");

  m_ValueMapping.clear();
  for (size_t i = 0; i<vars.size(); ++i) {
    const std::string s = ParserTools::removeSpaces(vars[i]);
    m_ValueMapping.insert(std::pair<std::string,size_t>(s,i) );
  }

  m_lastValues.resize(vars.size());
  for (size_t i = 0; i<m_lastValues.size(); ++i) {
    m_lastValues[i] = 0.0;
  }
}

void HASIRemote::receiveCommands(const std::string& cmds) {
  std::vector<std::string> newCmds;
  ParserTools::tokenize(cmds, newCmds, ",");
  {
    SCOPEDLOCK(m_CScommandQueue);
    m_commandQueue.insert(m_commandQueue.end(), newCmds.begin(), newCmds.end());
  }
}

void HASIRemote::processInitialization(const std::string& data) {
  
  std::size_t pos = data.find(";");
  
  if (pos==std::string::npos) {
    std::stringstream ss;
    ss << "No device ID found in init string";
    throw ERemoteInit(ss.str());
  }
  
  m_remoteID = IVDA::SysTools::TrimStr(data.substr(0,pos));
  
  std::string tail = "";
  if (pos+1 < data.length()) {
    tail = IVDA::SysTools::TrimStr(data.substr(pos+1));
  }

  updateInterestingVars(tail);
}


HASIRemote::eCommand HASIRemote::splitIncommingCall(const std::string& data,
                                                    std::string& tail) const {
  tail = "";
  std::size_t pos = data.find(";");

  if (pos==std::string::npos) {
    std::stringstream ss;
    ss << "No command found.";
    throw ERemoteParser(ss.str());
  }
  
  const std::string cmdStr = IVDA::SysTools::TrimStr(data.substr(0,pos));
  
  tail = "";
  if (pos+1 < data.length()) {
    tail = IVDA::SysTools::TrimStr(data.substr(pos+1));
  }
  if (cmdStr == "GET_VARIABLE_VALUES")
    return EC_GET_VARIABLE_VALUES;
  if (cmdStr == "SENDING_COMMANDS")
    return EC_SENDING_COMMANDS;
  if (cmdStr == "DISCONNECT")
    return EC_DISCONNECT;
  if (cmdStr == "SENDING_INTERESTING_VAR_UPDATE")
    return EC_SENDING_INTERESTING_VAR_UPDATE;
  return EC_UNKNOWN;
}

bool HASIRemote::processData(const std::string& data) {
  std::string tail= "";
  eCommand cmd = splitIncommingCall(data, tail);

  switch (cmd) {
    case EC_GET_VARIABLE_VALUES:
      sendUpdateToRemote();
      break;
    case EC_SENDING_INTERESTING_VAR_UPDATE:
      updateInterestingVars(tail);
      break;
    case EC_SENDING_COMMANDS:
      receiveCommands(tail);
      break;
    case EC_DISCONNECT:
      return false;
    case EC_UNKNOWN:
    default:
      std::stringstream ss;
      ss << "processData received unknown command from remote " << m_remoteID;
      throw ERemoteParser(ss.str());
  }
  return true;
}

bool HASIRemote::handleIncommingData(int8_t data) {
  if (data == '\r') return false;
  
  m_receiveBuffer += data;

  // prevent a buffer overflow by limiting the max size
  if (m_receiveBuffer.size() >= maxMessageLength) m_receiveBuffer = "";

  if (checkPostFix(m_receiveBuffer, dataBeginNL)) {
    // set receive buffer to dataBeginNL, possibly removing any leftover
    // data before that string
    m_receiveBuffer = dataBeginNL;
    return true;
  }

  if (checkPostFix(m_receiveBuffer, dataEndNL)) {
    std::string message = removeHeaderAndFooter(m_receiveBuffer);

    if (!m_config->getRemotePassword().empty()) {
      message = decryptMessage(message);
    }
    
    if (m_remoteID.empty()) {
      processInitialization(message);

      if (m_config->getDisplayRemoteActivity()) {
        IVDA_MESSAGE("remote " << m_remoteID << " connected");
      }
    } else {
      if (!processData(message)) return false;
    }
    m_receiveBuffer = "";
  }
  return true;
}


bool HASIRemote::isDisconnected() {
  SCOPEDLOCK(m_CSClient);
  return !m_poolThread->isBusy() || m_pClient == nullptr ||
         !m_pClient->IsConnected();
}

void HASIRemote::sendUpdateToRemote() {
  uint32_t counter = 0;
  while (!m_bDataIsValid && counter < 1000) {
    delay(10);
    counter++;
  }
  
  if (m_pClient && m_pClient->IsConnected()) {
    SCOPEDLOCK(m_CSlastValues);


    // convert array to string
    std::string s;
    for (const double d : m_lastValues) {
      s += SysTools::ToString(d) + " ";
    }
    if (!m_config->getRemotePassword().empty()) {
      s = encryptMessage(s);
    }
    s = dataBeginNL + s + "\n" + dataEndNL;
    
    
    // send string
    try {
      m_pClient->SendData((const int8_t*)(s.data()), uint32_t(s.length()),
                          m_config->getRemoteTimeout());
    } catch (SocketException const&) {
    }
  }
}

std::string HASIRemote::decryptMessage(std::string message) {
  if (m_remoteID.empty()) {
    
    size_t pos = message.find_first_of(";");
    
    if (pos == std::string::npos) {
      std::stringstream ss;
      ss << "No initialization vector found in init string";
      throw ERemoteInit(ss.str());
    }
    
    std::string strIV = IVDA::SysTools::TrimStr(message.substr(0,pos));
    
    SimpleVec iv;
    base64_decode(strIV, iv);
    message = message.substr(pos+1);
    
    
    m_AESCryptIn = std::make_shared<AESCrypt>(iv.constData(), m_config->getRemotePassword());
    m_AESCryptOut = std::make_shared<AESCrypt>(iv.constData(), m_config->getRemotePassword());
  }
  message = m_AESCryptIn->decryptString(IVDA::SysTools::TrimStr(message));
  
  std::size_t pos = message.find(testHeader);
  if (pos==std::string::npos) {
    std::stringstream ss;
    ss << "OK token not found in encrypted string";
    throw ERemoteInit(ss.str());
  }
  
  if (pos+testHeader.length() < message.length()) {
    return IVDA::SysTools::TrimStr(message.substr(pos+testHeader.length()));
  } else {
    return "";
  }
}

std::string HASIRemote::encryptMessage(const std::string& message) {
  if(!m_AESCryptOut) {
    IVDA_WARNING("encryptMessage called without a valid encrypter");
    return "";
  }
  
  return m_AESCryptOut->encryptString(testHeader + message);
}


/*
 The MIT License
 
 Copyright (c) 2015-2016 Jens Krueger
 
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
