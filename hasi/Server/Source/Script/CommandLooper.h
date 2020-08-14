#ifndef COMMANDLOOPER_H
#define COMMANDLOOPER_H

#include "ExpressionParser.h"
#include "TimerManager.h"
#include "ActivationManager.h"
#include "StopWatchManager.h"
#include "StateManager.h"
#include "PulseManager.h"
#include "ClockManager.h"
#include "RandomManager.h"
#include "ScriptExecuteManager.h"
#include "Variable.h"
#include "HASConfig.h"

class CommandLooper {
public:
  CommandLooper(const HAS::HASConfigPtr config);
  ~CommandLooper();

  void reParse(bool bApply);

  const std::vector<std::string>& getConnections() const {return connections;}
  const std::vector<std::string> getOutputs() const;
  VarStrAssignments execute(const VarStrAssignments& inputAssignments);
  VarStrAssignments evaluateCommands(const std::vector<CommandPtr>& cmds);
  
  VarStrAssignments getCurrentStrVas() const;
  const VarAssignments& getCurrentVas() const {return vas;}
  
  enum LogType {
    LT_off,
    LT_screen,
    LT_file
  };
  
  void setEventLogging(LogType logging);
  LogType getEventLogging() const;
  void dumpStates(const std::string& filename);
  bool writeParsedScript(const std::string& filename);
  void printThreadStatus();
  
  void printValueDebug(bool bPrintValueDebug) {
    m_bPrintValueDebug = bPrintValueDebug;
  }

  void setMailValueDebug(bool bMailValueDebug) {
    m_bMailValueDebug = bMailValueDebug;
  }
  
  bool getMailValueDebug() {
    return m_bMailValueDebug;
  }

  std::string setValueDebug(std::string varName);
  
  
  void setSysValues(float loadRatio, float overloadRatio, uint32_t eventDelay) {
    m_fHASOverloadRatio = overloadRatio;
    m_fHASLoadRatio = loadRatio;
    m_HASEventDelay = eventDelay;
  }
  
  void saveState() const;
  void connectActivations(std::shared_ptr<HAS::HASBus> hasBus);

private:
  HAS::HASConfigPtr m_config;
  
  LogType logging;  

  std::vector<double> lastVarAssignments;
  std::vector<std::string> connections;
  std::vector<VariablePtr> value_ons;
  std::vector<VariablePtr> value_offs;
  std::vector<VariablePtr> value_changes;
  std::vector<size_t> crossref_value_ons;
  std::vector<size_t> crossref_value_offs;
  std::vector<size_t> crossref_value_changes;
  size_t clockCount;
  size_t timerCount;
  size_t stopWatchCount;
  size_t stateCount;
  size_t pulseCount;
  size_t randomCount;
  VarAssignments vas;

  TimerManager timerManager;
  ActivationManager activationManager;
  StopWatchManager stopWatchManager;
  StateManager stateManager;
  PulseManager pulseManager;
  ClockManager clockManager;
  RandomManager randomManager;
  ScriptExecuteManager scriptExecuteManager;
  ExpressionParser expressionParser;

  std::string m_strDumpVariables;
  
  bool m_bPrintValueDebug;
  bool m_bMailValueDebug;
  std::vector<std::string> m_debugValues;

  float m_fHASOverloadRatio;
  float m_fHASLoadRatio;
  uint32_t m_HASEventDelay;
  
  void validateSystemCall(const std::string& name);
  void insertSyscalls(VarAssignments& v) const;
  void setSyscalls(VarAssignments& v, size_t& i) const;
  void computeConnections();
  
  void mailReport(const std::string& msg) const;
};

#endif // COMMANDLOOPER_H
