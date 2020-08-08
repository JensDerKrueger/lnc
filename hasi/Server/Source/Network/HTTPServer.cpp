#include "HTTPServer.h"

#include <sstream>    // std::stringstream
#include <iterator>

#include <Tools/SysTools.h>
#include <Script/ParserTools.h>
#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include <Script/Variable.h>

using namespace HAS;
using namespace IVDA;


HTTPServer::HTTPServer(const HASConfigPtr config)
: m_config(config)
, m_serverThread(std::bind(&HTTPServer::serverThread, this, std::placeholders::_1, std::placeholders::_2))
, m_iTimeout(10000)
, m_iPort(5566)
{
  
  if (m_config) {
    m_iTimeout = m_config->getNetworkTimeout();
    m_iPort = m_config->getHTTPPort();
    loadWhitelist();
  }
  
  m_serverThread.StartThread();
  IVDA::SysTools::msleep(1000); // give the thread time to start
}

HTTPServer::~HTTPServer() {
  if (!m_serverThread.IsRunning()) return;
  
  requestStop();
  
  if (!m_serverThread.JoinThread(m_iTimeout) && m_serverThread.IsRunning()) {
    IVDA_WARNING("Shutdown of HTTP Server thread timed out, killing thread");
    m_serverThread.KillThread();
  }
}

void HTTPServer::requestStop() {
  if (reportDebugData()) IVDA_MESSAGE("Shutdown of HTTP Server initiated");
  m_serverThread.RequestThreadStop();
}

bool HTTPServer::reportDebugData() {
  return !m_config || m_config->getReportHTTPActivities();
}

std::string HTTPServer::syncState(const VarAssignments& vas) {
  SCOPEDLOCK(m_CSInternalDataLock);
  
  std::stringstream changeCmd;

  if (m_whitelist.empty()) {
    for (auto it=m_changes.begin(); it!=m_changes.end(); ++it) {
      if (it!=m_changes.begin()) changeCmd << ",";
      changeCmd << "[" << it->first << "] =" << it->second;
    }
  } else {
    for (auto it=m_changes.begin(); it!=m_changes.end(); ++it) {
      if (m_whitelist.find(it->first) != m_whitelist.end()) {
        if (it!=m_changes.begin()) changeCmd << ",";
        changeCmd << "[" << it->first << "] =" << it->second;
      } else {
        if (reportDebugData())
          IVDA_WARNING("Change of non-whitelistest parameter " << it->first << " requested.");
      }
    }
  }
  
  m_changes.clear();
  m_InternalData = vas;
  
  return changeCmd.str();
}

double HTTPServer::getValue(const std::string& name) {
  SCOPEDLOCK(m_CSInternalDataLock);
 
  /*
  auto it = m_changes.find(name);
  if (it != m_changes.end()) {
    if (reportDebugData()) IVDA_WARNING("Reporting unapplied change");
    return m_changes[name];
  }
  */
  
  for (VarAssignment v : m_InternalData) {
    if (v.var->getRAWName() == name)
      return v.value;
  }
  return 0;
}

double HTTPServer::setValue(const std::string& name, double v) {
  SCOPEDLOCK(m_CSInternalDataLock);

  if (reportDebugData()) {
    auto it = m_changes.find(name);
    if (it != m_changes.end()) {
      IVDA_WARNING("HTTPServer: Previous change was never applied");
    }
  }
  
  m_changes[name] = v;  
  return v;
}

std::string HTTPServer::parseRequest(const std::string& buffer) {
  if (reportDebugData()) IVDA_MESSAGE(buffer);
  
  std::vector<std::string> lines;
  ParserTools::tokenize(buffer, lines, "\r\n");
  
  for (std::string s : lines) {
    std::vector<std::string> items;
    ParserTools::tokenize(s, items);
    
    if (items.size() == 3 && items[0] == "GET")
      return items[1];
  }
  
  
  return "";
}


parameter HTTPServer::parseParameter(const std::string& p) {
  parameter result;
  std::vector<std::string> items;
  ParserTools::tokenize(p, items, "=");
  if (items.size() == 2) {
    result.name = items[0];
    result.value = items[1];
  }
  return result;
}

urlRequest HTTPServer::parseURL(const std::string& r) {
  urlRequest result;
  
  std::vector<std::string> urlAndParameter;
  ParserTools::tokenize(r, urlAndParameter, "?");
  
  if (urlAndParameter.size() > 1) {
    std::vector<std::string> parameters;
    ParserTools::tokenize(urlAndParameter[1], parameters, "&");
    for (std::string s : parameters) {
      parameter p = parseParameter(s);
      if (p.name != "")
        result.parameters.push_back(p);
    }
  }
  
  if (urlAndParameter.size() > 0)
    result.url = urlAndParameter[0];
  
  return result;
}

double HTTPServer::processRequest(const urlRequest& url) {
  
  std::string variable = "";
  
  if (reportDebugData()) IVDA_MESSAGE("HTTPServer Searching variable parameter");

  for (parameter p : url.parameters) {
    if (p.name == "variable") {
      variable =p.value;
      break;
    }
  }

  if (variable == "") {
    if (reportDebugData()) IVDA_ERROR("HTTPServer Invalid request. Variable name not found.");
    return 0;
  }
  
  
  if (url.url == "/get") {
    if (reportDebugData()) IVDA_MESSAGE("HTTPServer Reporting value of variable " << variable);
    return getValue(variable);
  }

  if (url.url == "/set") {
    for (parameter p : url.parameters) {
      if (p.name == "value") {
        double value = SysTools::FromString<double>(p.value);
        if (reportDebugData()) IVDA_MESSAGE("HTTPServer Changing value of variable " << variable << " to " << value);
        return setValue(variable, value);
      }
    }
    return 0;
  }

  if (reportDebugData()) IVDA_ERROR("HTTPServer Invalid request.");
  return 0;
}

void HTTPServer::respondToRequest(const std::string& request) {
  if (reportDebugData()) IVDA_MESSAGE("HTTPServer Responding to request " << request);
  
  std::stringstream ss;
  
  // write header
  ss << "HTTP/1.1 200 OK\n"
  // << "Date: Fri, 13 Jul 2018 23:38:21 GMT\n"
  << "Server: HAS-Server\n"
  << "Accept-Ranges: bytes\n"
  << "Content-Type: text/plain\n"
  << "\n" << SysTools::ToString(processRequest(parseURL(request)));
  
  try {
    m_pClient->SendData((const int8_t*)(ss.str().data()), uint32_t(ss.str().length()), m_iTimeout);
  } catch (SocketException const& e) {
    if (reportDebugData()) {
      std::stringstream ss;
      ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
      std::string s = ss.str();
      IVDA_ERROR(s);
    }
  }
}

bool HTTPServer::processData(const std::string& buffer) {
  if (buffer.length() >= 4 &&
      (int)buffer[buffer.length()-1] == 10 &&
      (int)buffer[buffer.length()-2] == 13 &&
      (int)buffer[buffer.length()-3] == 10 &&
      (int)buffer[buffer.length()-4] == 13) {
    const std::string request = parseRequest(buffer);
    respondToRequest(request);
    return true;
  } else {
    return false;
  }
}

void HTTPServer::handleClients(IVDA::Predicate pContinue) {
  std::string buffer;
  
  uint32_t subIntervalCount = std::max<uint32_t>(1,m_iTimeout/1000);
  uint32_t subIntervalLength = std::max<uint32_t>(1,m_iTimeout/subIntervalCount);
  
  try {
    if (reportDebugData()) IVDA_MESSAGE("HTTPServer Waiting for connection");
    
    TCPSocket* pClient = nullptr;
    while (pClient == nullptr) {
      
      for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
        while (!m_pServer->AcceptNewConnection((ConnectionSocket**)&pClient, subIntervalLength)) {
          if (!pContinue()) {
            IVDA_MESSAGE("HTTPServer terminating");
            return;
          }
        }
        if (pClient != nullptr) break;
      }
      
      pClient->SetNoSigPipe(true);
      m_pClient.reset(pClient);
    }
    
    
    if (reportDebugData()) IVDA_MESSAGE("HTTPServer Client Connected");
    
    try {
      while (m_pClient->IsConnected() && pContinue()) {
        int8_t datum = 0;
        // receive one byte
        uint32_t const bytes = m_pClient->ReceiveData(&datum, 1, m_iTimeout);
        
        if (pContinue() && bytes > 0) {
          buffer += (char)datum;
          if (buffer.size() > 100000 || processData(buffer)) {
            m_pClient = nullptr;
            return;
          }
        } else {
          if (reportDebugData()) {
            if (!pContinue())
              IVDA_MESSAGE("HTTPServer terminating");
            else
              IVDA_WARNING("HTTPServer timeout");
          }
        }
      }
    } catch (SocketException const& e) {
      if (reportDebugData()) {
        std::stringstream ss;
        ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
        std::string s = ss.str();
        IVDA_ERROR(s);
      }
    }
    
  } catch (SocketException const& e) {
    if (reportDebugData()) {
      std::stringstream ss;
      ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
      std::string s = ss.str();
      IVDA_ERROR(s);
    }
  }
  
}


void HTTPServer::serverThread(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {

  if (reportDebugData()) IVDA_MESSAGE("HTTPServer starting up");
  
  try {
    std::shared_ptr<TCPServer> pServer(new TCPServer());
    pServer->SetNonBlocking(m_iTimeout == INFINITE_TIMEOUT ? false : true);
    pServer->SetNoDelay(false);
    pServer->SetReuseAddress(true);
    pServer->SetNoSigPipe(true);
    pServer->Bind(NetworkAddress(NetworkAddress::Any, m_iPort));
    pServer->Listen();
    pServer->GetLocalPort();
    m_pServer = pServer;
  } catch (SocketException const& e) {
    if (reportDebugData()) {
      std::stringstream ss;
      ss << "SocketException: " << e.what() << " (" << e.where() << " returned with error code " << e.withErrorCode() << ")";
      IVDA_ERROR(ss.str());
    }
  }
  
  while (!pContinue || pContinue()) {
    handleClients(pContinue);
  }
  
  
  if (reportDebugData()) {
    IVDA_MESSAGE("HTTPServer shutdown complete");
  }
}

void HTTPServer::loadWhitelist() {
  if (!m_config || m_config->getHTTPWhitelist().empty()) {
    m_whitelist.clear();
    return;
  }

  try {
    std::ifstream whitelistfile(m_config->getHTTPWhitelist());

    if (! whitelistfile.is_open()) {
      if (reportDebugData()) {
        IVDA_ERROR("Can't load whitelist file " << m_config->getHTTPWhitelist());
        return;
      }
    }

    m_whitelist.clear();
    copy(std::istream_iterator<std::string>(whitelistfile),
         std::istream_iterator<std::string>(),
         std::inserter(m_whitelist,m_whitelist.begin()));
    if (reportDebugData()) {
      IVDA_MESSAGE("Loaded whitelist file "
                   << m_config->getHTTPWhitelist()
                   << " with " << m_whitelist.size() << " entries.");
    }
  } catch (std::exception const&) {
    if (reportDebugData()) {
      IVDA_ERROR("Can't load whitelist file " << m_config->getHTTPWhitelist());
    }
  }
}
