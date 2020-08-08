#ifndef SCRIPTEXECUTEMANAGER_H
#define SCRIPTEXECUTEMANAGER_H

#include "Expression.h"

#include <string>
#include <vector>
#include <deque>
#include <Tools/Threads.h>
#include "HASConfig.h"

class Command;

struct ScriptExecuteWorkItem {
  ScriptExecuteWorkItem(const std::string s, const VarAssignments& v) :
  filename(s), vas(v) {}
  
  std::string filename;
  VarAssignments vas;
};

class ScriptExecuteManager {
public:
  ScriptExecuteManager(const HAS::HASConfigPtr config);
  ~ScriptExecuteManager();
  bool execute(const Command* cmd, const VarAssignments& vas);
  
  void printThreadStatus();
  void setMaxQueueSize(uint32_t maxQueueSize) {m_maxQueueSize = m_actualMaxQueueSize = maxQueueSize;}
  uint32_t getMaxQueueSize() const {return m_maxQueueSize;}
  
private:
  HAS::HASConfigPtr m_config;
  uint32_t m_maxQueueSize;
  uint32_t m_actualMaxQueueSize;
  
  std::deque<std::shared_ptr<ScriptExecuteWorkItem>> m_vWorkerQueue;
  IVDA::CriticalSection m_CSWorkerQueue;
  std::shared_ptr<IVDA::LambdaThread> m_worker;

  IVDA::CriticalSection m_CSExecutor;
  std::shared_ptr<IVDA::LambdaThread> m_executor;
  std::shared_ptr<ScriptExecuteWorkItem> m_wi;
  
  mutable std::string m_currentLine;
  
  void work(IVDA::Predicate pContinue,
            IVDA::LambdaThread::Interface& threadInterface);
  void exec(IVDA::Predicate pContinue,
            IVDA::LambdaThread::Interface& threadInterface);
  
  std::string evaluateVars(const std::string& input,
                           const VarAssignments& vas) const;
  
  void executeFile(const std::string& filename,
                   const VarAssignments& vas) const;
  void executeLine(const std::string& filename,
                   const VarAssignments& vas) const;
  void executeFillIn(const std::vector<std::string>& token,
                     const VarAssignments& vas) const;
  void executeReplace(const std::vector<std::string>& token,
                      const VarAssignments& vas) const;
  void executeMail(const std::vector<std::string>& token,
                   const VarAssignments& vas,
                   bool bHTMLMail) const;
  void executeCopy(const std::vector<std::string>& token,
                   const VarAssignments& vas) const;
  void executeMove(const std::vector<std::string>& token,
                   const VarAssignments& vas) const;
  void executeDumpXML(const std::vector<std::string>& token,
                      const VarAssignments& vas) const;
  void executeLog(const std::vector<std::string>& token,
                  const VarAssignments& vas) const;
  void executeSystem(const std::vector<std::string>& token,
                     const VarAssignments& vas) const;
};

#endif // SCRIPTEXECUTEMANAGER_H
