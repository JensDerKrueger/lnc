#include "ScriptExecuteManager.h"

#include <Tools/SysTools.h>

#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "ScriptExecuteCommand.h"
#include "Variable.h"
#include <Tools/MailReporter.h>
#include <Tools/MailPlotter.h>

#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include <memory>
#include <sstream>
#include <fstream>

ScriptExecuteManager::ScriptExecuteManager(const HAS::HASConfigPtr config)
: m_config(config)
, m_maxQueueSize(100)
, m_actualMaxQueueSize(100)
, m_worker(nullptr)
, m_executor(nullptr)
, m_wi(nullptr)
{
  m_worker = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&ScriptExecuteManager::work, this, std::placeholders::_1, std::placeholders::_2)));
  m_worker->StartThread();
  
  m_executor = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&ScriptExecuteManager::exec, this, std::placeholders::_1, std::placeholders::_2)));
  m_executor->StartThread();
}


ScriptExecuteManager::~ScriptExecuteManager() {
  if (m_worker) {
    m_worker->RequestThreadStop();
    m_worker->JoinThread(5000);
    if (m_worker->IsRunning()) {
      IVDA_WARNING("Killing worker thread");
      m_worker->KillThread();
    }
  }
  
  if (m_executor) {
    m_executor->RequestThreadStop();
    m_executor->JoinThread(5000);
    if (m_executor->IsRunning()) {
      IVDA_WARNING("Killing executor thread");
      m_executor->KillThread();
    }
  }
}

void ScriptExecuteManager::printThreadStatus() {
  if (m_worker && m_worker->IsRunning())
    IVDA_MESSAGE("ScriptExecuteManager worker is running");
  else
    IVDA_WARNING("ScriptExecuteManager worker is NOT running");
}

void ScriptExecuteManager::work(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  while (!pContinue || pContinue() ) {
    std::shared_ptr<ScriptExecuteWorkItem> wi=nullptr;
    
    if (!m_CSWorkerQueue.Lock(2)) {
      IVDA_WARNING("Timeout during worker queue lock.");
      continue;
    }
    if (!m_vWorkerQueue.empty()) {
      wi = m_vWorkerQueue.front();
      m_vWorkerQueue.pop_front();
    }
    m_CSWorkerQueue.Unlock();
    
    
    if (wi == nullptr) {
      threadInterface.Suspend(pContinue);
    } else {
      uint32_t iRetryCounter = 0;
      do {
        if (m_executor && m_executor->IsRunning() && m_CSExecutor.Lock(60)) { // give the executor one minute to complete
          if (m_wi && iRetryCounter < 60) {
            // unprocessed work item found
            m_CSExecutor.Unlock();
            m_executor->Resume();
            delay(1000);
            iRetryCounter++;
            continue;
          } else {
            m_wi = wi;
            wi = nullptr;
            m_CSExecutor.Unlock();
            m_executor->Resume();
          }
        } else {
          if (m_executor && m_executor->IsRunning())
            IVDA_WARNING("Executor Timneout: executing line:" + m_currentLine);
          else
            IVDA_WARNING("Executor not running, restarting ...");
          
          if (m_executor)
            m_executor->RequestThreadStop();
          if (m_executor)
            m_executor->JoinThread(1000);
          
          // if all else fails, kill the thread
          if (m_executor && m_executor->IsRunning()) {
            IVDA_WARNING("Killing executor thread");
            m_executor->KillThread();
          }
          
          
          // if the execcutor stalls we could retry with the last work item:
          // m_wi = wi;
          // or we could discard it
          // m_wi = nullptr;
          // both methods have their pros and cons. Since in most cases the
          // thread would stall again with the same item, I decided to implement
          // the discard method (see below)          
          m_wi = nullptr;
          
          wi = nullptr;
          m_executor = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&ScriptExecuteManager::exec, this, std::placeholders::_1, std::placeholders::_2)));
          m_executor->StartThread();
        }
      } while (wi);
      
      if (iRetryCounter >= 60) {
        IVDA_WARNING("unprocessed work item lost");
      }
    }
  }
}

void ScriptExecuteManager::exec(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  while (!pContinue || pContinue() ) {
    threadInterface.Suspend(pContinue);
    
    m_CSExecutor.Lock();
    if (m_wi) {
      try {
        executeFile(m_wi->filename, m_wi->vas);
      } catch (const ExecuteException& e) {
        IVDA_WARNING("Having trouble executing command. " << e.what());
      }
      m_wi = nullptr;
    } else {
      if (pContinue && pContinue()) // avoid warning when shutting down
        IVDA_WARNING("Empty work item detected");
    }
    m_CSExecutor.Unlock();
  }
}

bool ScriptExecuteManager::execute(const Command* cmd, const VarAssignments& vas)
{
  const ScriptExecuteCommand* tCmd = dynamic_cast<const ScriptExecuteCommand*>(cmd);
  if (tCmd) {
    std::shared_ptr<ScriptExecuteWorkItem> wi(new ScriptExecuteWorkItem(tCmd->getFilename(), vas));
    {
      if (!m_CSWorkerQueue.Lock(2)) {
        IVDA_WARNING("Timeout during worker queue lock, work item discarded.");
        return false;
      }
      size_t queueSize = m_vWorkerQueue.size();
      if (queueSize < m_actualMaxQueueSize) {
        m_vWorkerQueue.push_back(wi);
        
        // if we've temporarily raised the queue size but the situation
        // has now relaxed, set it back to noemal
        if (m_actualMaxQueueSize != m_maxQueueSize && uint32_t(queueSize < m_maxQueueSize*0.5)) {
          m_actualMaxQueueSize = m_maxQueueSize;
          IVDA_MESSAGE("Returned queue size to " <<  m_actualMaxQueueSize << " since the worker has caught up.");
        }
      } else {
        if (m_actualMaxQueueSize != m_maxQueueSize)
          IVDA_WARNING("Worker queue size " << queueSize << " exceeds maximum of temporarily increased " << m_actualMaxQueueSize << ", work item discarded.");
        else
          IVDA_WARNING("Worker queue size " << queueSize << " exceeds maximum of " << m_maxQueueSize << ", work item discarded.");
        
        IVDA_WARNING("Last executing line:" + m_currentLine);
        
        if (m_worker)
          m_worker->RequestThreadStop();
        if (m_worker)
          m_worker->JoinThread(1000);
        // if all else fails, kill the thread
        if (m_worker && m_worker->IsRunning()) {
          IVDA_WARNING("Killing worker thread");
          m_worker->KillThread();
        } else {
          IVDA_MESSAGE("Worker thread successfully stopped. Restarting now.");
        }
        
        if (m_actualMaxQueueSize == m_maxQueueSize) {
          // increase m_maxQueueSize by 2 for the worker to catch up
          m_actualMaxQueueSize = std::max(m_maxQueueSize, m_maxQueueSize * 2); // max is here to catch overflows in
                                                                         // case m_maxQueueSize * 2 is to big for uint32_t
          
          IVDA_MESSAGE("Temporarily increased queue size to " <<  m_actualMaxQueueSize << " for the worker to catch up.");
        } else {
          IVDA_WARNING("Worker never caught up, resetting queue. " << queueSize << " work items are lost." );
          m_vWorkerQueue.clear();
          m_actualMaxQueueSize = m_maxQueueSize;
        }
        
        m_worker = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&ScriptExecuteManager::work, this, std::placeholders::_1, std::placeholders::_2)));
        m_worker->StartThread();
        
      }
      m_CSWorkerQueue.Unlock();
    }
    m_worker->Resume();
    return true;
  }
  return false;
}

void ScriptExecuteManager::executeFile(const std::string& filename, const VarAssignments& vas) const {
  
  std::ifstream file(filename);
  
  if (!file.is_open())
    throw ExecuteException(std::string("error executing command file ") + filename);
  
  std::string line;
  while (std::getline(file, line))
  {
    executeLine(line, vas);
  }
}


void ScriptExecuteManager::executeLine(const std::string& line, const VarAssignments& vas) const {
  m_currentLine = line;
  
  std::string cmdLine = IVDA::SysTools::TrimStrLeft(ParserTools::removeComments(line));
  
  std::vector<std::string> token;
  ParserTools::tokenize(cmdLine, token, " ", false);
  
  if (token.empty()) return;
  
  if (token[0] == "fillIn") {
    executeFillIn(token, vas);
    return;
  }
  
  if (token[0] == "replace") {
    executeReplace(token, vas);
    return;
  }
  
  if (token[0] == "mail") {
    executeMail(token, vas, false);
    return;
  }

  if (token[0] == "htmlmail") {
    executeMail(token, vas, true);
    return;
  }

  if (token[0] == "move") {
    executeMove(token, vas);
    return;
  }
  
  if (token[0] == "copy") {
    executeCopy(token, vas);
    return;
  }
  
  if (token[0] == "dumpXML") {
    executeDumpXML(token, vas);
    return;
  }
  
  if (token[0] == "log") {
    executeLog(token, vas);
    return;
  }
  
  if (token[0] == "system") {
    try {
      executeSystem(token, vas);
    } catch (const ExecuteException& e) {
      IVDA_WARNING(e.what());
    }
    return;
  }
  
  // other commands here
  
  std::stringstream ss;
  ss << "Unknown command " << token[0] << " executing command file. (Parameters:";
  for (size_t i = 1;i<token.size();++i) {
    ss << " " << token[i];
  }
  ss << ")";
  throw ExecuteException(ss.str());
}

static void handleNewlines(std::string& str) {
  IVDA::SysTools::ReplaceAll(str, "\\n", "\n");
}

std::string ScriptExecuteManager::evaluateVars(const std::string& input,
                                               const VarAssignments& vas) const {
  std::string output = input;
  // replace variable in target string
  for (auto va = vas.begin(); va != vas.end(); ++va) {
    std::string strID = std::string("[") + va->var->getRAWName() + std::string("]");
    std::string strValue = IVDA::SysTools::ToString(va->value);
    IVDA::SysTools::ReplaceAll(output, strID, strValue);
  }
  handleNewlines(output);
  return output;
}

void ScriptExecuteManager::executeDumpXML(const std::vector<std::string>& token,
                                          const VarAssignments& vas) const {
  
  if (token.size() != 2) {
    std::stringstream ss;
    ss << "Invalid dumpXML command, invalid parameter count, expected one. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::string targetFile = evaluateVars(token[1], vas);
  std::string tempFile = targetFile + std::string(".tmp");
  
  std::ofstream dst;
  dst.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
  
  try {
    dst.open(tempFile);
    dst << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<HASData>\n";
    for (auto va = vas.begin(); va != vas.end(); ++va) {
      dst << "  <" << va->var->getRAWName() << ">" << va->value
      << "</" << va->var->getRAWName() << ">\n";
    }
    dst << "</HASData>\n";
  }  catch (const std::ofstream::failure& e) {
    throw ExecuteException("Error writing temp XML file");
  }
  
  if ( rename( tempFile.c_str() , targetFile.c_str() ) != 0 ) {
    throw ExecuteException("Error moving temp XML file to target");
  }
}

void ScriptExecuteManager::executeCopy(const std::vector<std::string>& token,
                                       const VarAssignments& vas) const {
  if (token.size() < 3) {
    std::stringstream ss;
    ss << "Invalid copy command, invalid parameter count, expected two. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::string sourceFile = evaluateVars(token[1], vas);
  std::string targetFile = evaluateVars(token[2], vas);
  if (sourceFile == targetFile) return;
  
  std::ifstream src;
  std::ofstream dst;
  
  src.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
  dst.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
  try {
    src.open(sourceFile, std::ios::binary);
    dst.open(targetFile, std::ios::binary);
    dst << src.rdbuf();
  }
  catch (const std::ifstream::failure& e) {
    throw ExecuteException("Error executing copy");
  }
}

void ScriptExecuteManager::executeMove(const std::vector<std::string>& token,
                                       const VarAssignments& vas) const {
  if (token.size() < 3) {
    std::stringstream ss;
    ss << "Invalid move command, invalid parameter count, expected two. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::string sourceFile = evaluateVars(token[1], vas);
  std::string targetFile = evaluateVars(token[2], vas);
  if (sourceFile == targetFile) return;
  
  if ( rename( sourceFile.c_str() , targetFile.c_str() ) != 0 )
    throw ExecuteException("Error moving file");
}


void ScriptExecuteManager::executeMail(const std::vector<std::string>& inToken,
                                       const VarAssignments& vas,
                                       bool bHTMLMail) const {
  
  std::vector<std::string> token = inToken;
  size_t paramStart = 2;
  
  if (token.size() < 3) {
    std::stringstream ss;
    ss << "Invalid mail command: Invalid parameter count, expected two. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::string::size_type findPos = std::string::npos;
  std::string title;
  
  for (size_t i = paramStart;i<token.size();++i) {
    std::string::size_type pos = token[i].find_first_of("|");
    if (std::string::npos != pos) {
      findPos = i;
      break;
    }
  }
  
  
  if (std::string::npos != findPos) {
    title = "";
    
    for (size_t i = paramStart;i<token.size();++i) {
      std::string::size_type pos = token[i].find_first_of("|");
      
      if (std::string::npos == pos) {
        title += (i == paramStart) ? token[i] : std::string(" ") + token[i];
      } else {
        title += std::string(" ") + token[i].substr(0, pos);
        token[i] = token[i].substr(pos+1);
        paramStart = i;
        break;
      }
    }
    
  } else {
    if(m_config->getSystemName().empty()) {
      title = "HAS Message";
    } else {
      title = m_config->getSystemName() + " Message";
    }
  }
  MailReporter m(title);
  
  // check the last parameters if they are attachments
  size_t attachmentPos = token.size();
  for (size_t i = token.size()-1;i>=paramStart+1;--i) {
    if (IVDA::SysTools::FileExists(token[i])) {
      m.AddAttachment(token[i]);
      attachmentPos = i;
    } else {
      break;
    }
  }
  
  // re-assemble everything after the paramStart parameter
  // up to "attachmentPos-1" into one stirng
  std::string targetString = token[paramStart];
  for (size_t i = paramStart+1;i<attachmentPos;++i) {
    targetString += std::string(" ") + token[i];
  }
  targetString = evaluateVars(targetString, vas);
  
  if (bHTMLMail)
    m.SetHTMLText(targetString);
  else
    m.SetText(targetString);
  
  m.SendTo(token[1]);
}


void ScriptExecuteManager::executeReplace(const std::vector<std::string>& token,
                                          const VarAssignments& vas) const {
  
  if (token.size() < 4) {
    std::stringstream ss;
    ss << "Invalid replace command, invalid parameter count, expected three. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::ifstream inFile(token[1]);
  
  if (!inFile.is_open()) {
    std::stringstream ss;
    ss << "Error executing replace command, can't open input file " << token[1];
    throw ExecuteException(ss.str());
  }
  
  std::string fileData;
  std::string line;
  while (std::getline(inFile, line))
  {
    fileData += line + "\n";
  }
  inFile.close();
  
  // re-assemble everything after the 2nd parameter into one stirng
  std::string targetString = token[3];
  for (size_t i = 4;i<token.size();++i) {
    targetString += std::string(" ") + token[i];
  }
  targetString = evaluateVars(targetString, vas);
  
  // replace source string with tartstring in file
  IVDA::SysTools::ReplaceAll(fileData, token[2], targetString);
  
  std::ofstream outFile(token[1]);
  if (!outFile.is_open()) {
    std::stringstream ss;
    ss << "Error executing replace command, can't open output file " << token[2];
    throw ExecuteException(ss.str());
  }
  
  outFile << fileData;
  outFile.close();
}


void ScriptExecuteManager::executeFillIn(const std::vector<std::string>& token,
                                         const VarAssignments& vas) const {
  
  if (token.size() != 3) {
    std::stringstream ss;
    ss << "Invalid fillIn command, invalid parameter, count expected two. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  std::ifstream inFile(token[1]);
  
  if (!inFile.is_open()) {
    std::stringstream ss;
    ss << "Error executing fillIn command, can't open input file " << token[1];
    throw ExecuteException(ss.str());
  }
  
  std::string fileData;
  std::string line;
  while (std::getline(inFile, line))
  {
    fileData += line + "\n";
  }
  inFile.close();
  
  fileData = evaluateVars(fileData, vas);
  
  std::ofstream outFile(token[2]);
  if (!outFile.is_open()) {
    std::stringstream ss;
    ss << "Error executing fillIn command, can't open output file " << token[2];
    throw ExecuteException(ss.str());
  }
  
  outFile << fileData;
  outFile.close();
}

void ScriptExecuteManager::executeLog(const std::vector<std::string>& token,
                                      const VarAssignments& vas) const {
  
  if (token.size() < 5) {
    std::stringstream ss;
    ss << "Invalid log command, invalid parameter count, expected at least four. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  if ((token.size()-2) % 3 != 0) {  // parameters minus type "log" and filename
    std::stringstream ss;
    ss << "Invalid log command, invalid parameter count, expected multiple of three parameters (desc, value, unit). (Parameters:";
    for (size_t i = 2;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  
  std::ofstream logFile(token[1], std::ios::app);
  if (!logFile.is_open()) {
    std::stringstream ss;
    ss << "Error executing log command, can't open output file " << token[1];
    throw ExecuteException(ss.str());
  }
  std::vector<PlotterEntry> values;
  for (size_t i = 2;i<token.size();i+=3) {
    PlotterEntry p = {token[i], evaluateVars(token[i+1], vas), token[i+2]};
    values.push_back(p);
  }
  FilePlotter::logLine(logFile, values);
  
  logFile.close();
}


void ScriptExecuteManager::executeSystem(const std::vector<std::string>& token,
                                         const VarAssignments& vas) const {
  
  if (token.size() < 2) {
    std::stringstream ss;
    ss << "Invalid system command, invalid parameter count, expected at least one. (Parameters:";
    for (size_t i = 1;i<token.size();++i) {
      ss << " " << token[i];
    }
    ss << ")";
    throw ExecuteException(ss.str());
  }
  
  // re-assemble everything after the system keyword into one stirng
  std::string targetString = token[1];
  for (size_t i = 2;i<token.size();++i) {
    targetString += std::string(" ") + token[i];
  }
  targetString = evaluateVars(targetString, vas);
  
  if (!system( NULL )) {
    std::stringstream ss;
    ss << "Unable to execute system calls. No shell is available.";
    throw ExecuteException(ss.str());
  }
  
  int result = system(targetString.c_str());
  if (0 != result && m_config->getPrintScriptSystemErrors()) {
    std::stringstream ss;
    ss << "Nonzero value returned by system command " << targetString << " return value was " << result;
    throw ExecuteException(ss.str());
  }
}



