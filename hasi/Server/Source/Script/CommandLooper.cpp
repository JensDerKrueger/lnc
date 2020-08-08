#include "CommandLooper.h"
#include "Variable.h"

#include "StateCommand.h"
#include "PulseCommand.h"
#include "OutputCommand.h"
#include "TimerCommand.h"
#include "StopWatchCommand.h"

#include <Tools/MailReporter.h>

#include <Tools/DebugOut/AbstrDebugOut.h>

#include <memory>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <sstream>

#include <ctime>
#define DTTMFMT "%Y-%m-%d %H:%M:%S "
#define DTTMSZ 21
static char *getDtTm (char *buff) {
  time_t t = time (0);
  strftime (buff, DTTMSZ, DTTMFMT, localtime (&t));
  return buff;
}

CommandLooper::CommandLooper(const HAS::HASConfigPtr config)
: m_config(config)
, logging(LT_off)
, activationManager(config)
, scriptExecuteManager(config)
, expressionParser(config->getHASIFile())
, m_strDumpVariables("")
, m_bPrintValueDebug(false)
, m_bMailValueDebug(false)
, m_fHASOverloadRatio(-1.0f)
, m_fHASLoadRatio(-1.0f)
{
  timerManager.load(m_config->getTimerSerialFile());
  stopWatchManager.load(m_config->getStopWatchFile());
  stateManager.load(m_config->getStateSerialFile());
  computeConnections();
}

CommandLooper::~CommandLooper()
{
  saveState();
}

void CommandLooper::saveState() const {
  timerManager.save(m_config->getTimerSerialFile());
  stopWatchManager.save(m_config->getStopWatchFile());
  stateManager.save(m_config->getStateSerialFile());
}


void CommandLooper::dumpStates(const std::string& filename) {
  m_strDumpVariables = filename;
}

void CommandLooper::setEventLogging(LogType _logging) {
  logging = _logging;
}

CommandLooper::LogType CommandLooper::getEventLogging() const {
  return logging;
}

void CommandLooper::reParse(bool bApply) {
  expressionParser.reParse(bApply);
  if (bApply) computeConnections();
}

void CommandLooper::printThreadStatus() {
  scriptExecuteManager.printThreadStatus();
}

bool CommandLooper::writeParsedScript(const std::string& filename) {
  
  std::ofstream outFile(filename.c_str());
  if (outFile.is_open()) {
    std::string s = expressionParser.toString();
    outFile << s;
    outFile.close();
    return true;
  } else {
    return false;
  }
}

static void findCrossRefs(const std::vector<VariablePtr>& source,
                          const VarAssignments& vas,
                          std::vector<size_t>& target) {
  
  for (std::vector<VariablePtr>::const_iterator v = source.begin();
       v != source.end();
       v++) {
    const std::string& n = (*v)->getName();

    size_t index = vas.size();
    for (size_t i = 0; i<vas.size();++i) {
      if (vas[i].var->getName() == n) {
        index = i;
        break;
      }
    }
    
    if (index < vas.size()) {
      target.push_back(index);
    } else {
      throw ParseException(std::string("Can't find basic variable \"") +
                           (*v)->getBasicName() +
                           std::string("\" for modified variable \"") +
                           (*v)->getRAWName() +
                           std::string("\"")) ;
    }
  }
}


const std::vector<std::string> CommandLooper::getOutputs() const {
  std::vector<std::string> vo;

  Outputs os = expressionParser.getOutputs();
  for (Outputs::const_iterator o = os.begin();
       o != os.end();
       o++) {
    vo.push_back(*o);
  }
  return vo;
}

void CommandLooper::validateSystemCall(const std::string& name) {
  if (name != "init" &&
      name != "load" &&
      name != "overload" &&
      name != "delay")
    throw ParseException(std::string("Invalid system parameter ")+name);
  
}

void CommandLooper::insertSyscalls(VarAssignments& v) const {
  VarAssignment vaInit(VariablePtr(new Variable("[sys_init]")), 0);
  VarAssignment vaLoad(VariablePtr(new Variable("[sys_load]")), m_fHASLoadRatio);
  VarAssignment vaOverload(VariablePtr(new Variable("[sys_overload]")), m_fHASOverloadRatio);
  VarAssignment vaDelay(VariablePtr(new Variable("[sys_delay]")), float(m_HASEventDelay));

  v.push_back(vaInit);
  v.push_back(vaLoad);
  v.push_back(vaOverload);
  v.push_back(vaDelay);
}

void CommandLooper::setSyscalls(VarAssignments& v, size_t& i) const {
  v[i++].value = lastVarAssignments.empty();
  v[i++].value = m_fHASLoadRatio;
  v[i++].value = m_fHASOverloadRatio;
}

void CommandLooper::computeConnections() {
  Variables vars = expressionParser.getVariables();
  
  // first make sure that for every special var (on/off/change) we have a
  // corresponding basic var in the list of active variables
  Variables basics;
  for (Variables::const_iterator var = vars.begin();
       var != vars.end();
       var++) {
    switch (var->getSpecial()) {
      case Variable::special_on :
      case Variable::special_off :
      case Variable::special_change :
        basics.insert(Variable(std::string("[")+var->getBasicName()+std::string("]")));
        break;
      default:
        break;
    }
  }
  vars.insert(basics.begin(), basics.end());
  
  connections.clear();
  value_ons.clear();
  value_offs.clear();
  value_changes.clear();
  lastVarAssignments.clear();
  vas.clear();
  
  std::vector<VariablePtr> states;
  std::vector<VariablePtr> pulses;
  std::vector<VariablePtr> timers;
  std::vector<VariablePtr> stopWatches;
  std::vector<VariablePtr> clocks;
  std::vector<VariablePtr> randoms;
  
  for (Variables::const_iterator var = vars.begin();
       var != vars.end();
       var++) {
    
    switch (var->getSpecial()) {
      case Variable::special_on :
        value_ons.push_back(VariablePtr(new Variable(*var)));
        break;
      case Variable::special_off :
        value_offs.push_back(VariablePtr(new Variable(*var)));
        break;
      case Variable::special_change :
        value_changes.push_back(VariablePtr(new Variable(*var)));
        break;
      case Variable::special_none :
        switch (var->getType()) {
          case Variable::input :
            break;
          case Variable::state :
            states.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::pulse :
            pulses.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::timer :
            timers.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::stopWatch :
            stopWatches.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::clock :
            clocks.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::random :
            randoms.push_back(VariablePtr(new Variable(*var)));
            break;
          case Variable::system :
            // there is no vector for system calls, just validate them here
            validateSystemCall(var->getName());
            break;
        }
        break;
    }
    
    if (Variable::input == var->getType() &&
        std::find(connections.begin(), connections.end(),
                  var->getName())==connections.end()) {
      connections.push_back(var->getName());
    }
    
  }
  
  // create "real" variable assignements
  for (std::vector<std::string>::const_iterator a = connections.begin();
       a != connections.end();
       a++) {
    
    std::stringstream ss;
    ss << "[" << (*a) << "]";
    VarAssignment va(VariablePtr(new Variable(ss.str())), 0);
    vas.push_back(va);
  }
  
  // add assignments from state, pulses, stopWatches, clock, and random
  for (std::vector<VariablePtr>::const_iterator s = states.begin();
       s != states.end();
       s++) {
    VarAssignment va(*s, stateManager.getState((*s)->getName()));
    vas.push_back(va);
  }
  for (std::vector<VariablePtr>::const_iterator s = pulses.begin();
       s != pulses.end();
       s++) {
    VarAssignment va(*s, pulseManager.getPulse((*s)->getName()));
    vas.push_back(va);
  }
  for (std::vector<VariablePtr>::const_iterator t = timers.begin();
       t != timers.end();
       t++) {
    VarAssignment va(*t, timerManager.getTimer((*t)->getName()));
    vas.push_back(va);
  }
  for (std::vector<VariablePtr>::const_iterator w = stopWatches.begin();
       w != stopWatches.end();
       w++) {
    VarAssignment va(*w, stopWatchManager.getStopWatch((*w)->getName()));
    vas.push_back(va);
  }
  for (std::vector<VariablePtr>::const_iterator c = clocks.begin();
       c != clocks.end();
       c++) {
    VarAssignment va(*c, clockManager.getClock((*c)->getName()));
    vas.push_back(va);
  }
  for (std::vector<VariablePtr>::const_iterator r = randoms.begin();
       r != randoms.end();
       r++) {
    VarAssignment va(*r, randomManager.getRandom((*r)->getName()));
    vas.push_back(va);
  }
  
  insertSyscalls(vas);

  crossref_value_ons.clear();
  crossref_value_offs.clear();
  crossref_value_changes.clear();
  findCrossRefs(value_changes,vas, crossref_value_changes);
  findCrossRefs(value_ons,    vas, crossref_value_ons);
  findCrossRefs(value_offs,   vas, crossref_value_offs);
 
  clockCount = clocks.size();
  timerCount = timers.size();
  stopWatchCount = stopWatches.size();
  stateCount = states.size();
  pulseCount = pulses.size();
  randomCount = randoms.size();
}

VarStrAssignments CommandLooper::getCurrentStrVas() const {
  VarStrAssignments vsa;
  vsa.reserve(vas.size());
  
  for (auto va = vas.begin(); va != vas.end(); ++va) {
    vsa.push_back(VarStrAssignment(va->var->getRAWName(), va->value));
  }
  
  return vsa;
}

VarStrAssignments CommandLooper::evaluateCommands(const std::vector<CommandPtr>& cmds) {
  VarStrAssignments outputChanges;
  
  // apply state changes
  for (std::vector<CommandPtr>::const_iterator cmd = cmds.begin();
       cmd != cmds.end();
       cmd++) {
    
    // gather output changes
    OutputCommand* oCmd = dynamic_cast<OutputCommand*>(cmd->get());
    if (oCmd) {
      outputChanges.push_back(VarStrAssignment(oCmd->getExpressionName(),
                                               oCmd->getEvaluatedExpressionValue()));
      continue;
    }
    
    // execute other commands
    if (stateManager.execute(cmd->get())) continue;
    if (pulseManager.execute(cmd->get())) continue;
    if (timerManager.execute(cmd->get())) continue;
    if (stopWatchManager.execute(cmd->get())) continue;
    if (scriptExecuteManager.execute(cmd->get(), vas)) continue;
    if (activationManager.execute(cmd->get())) continue;
    
    throw ExecuteException("invalid command found in return from execute "+(*cmd)->toString());
  }
  return outputChanges;
}

void CommandLooper::mailReport(const std::string& msg) const {
  if (m_config->getFromAddress().empty() || m_config->getToAddress().empty()) return;
  
  std::stringstream title;
  if(m_config->getSystemName().empty()) {
    title << "HAS Debug Event";
  } else {
    title << "HAS " << m_config->getSystemName() << " Debug Event";
  }

  MailReporter rep(title.str(), msg);
  rep.SetFrom(m_config->getFromAddress(),m_config->getSystemName());
  rep.SendTo(m_config->getToAddress());
}


VarStrAssignments CommandLooper::execute(const VarStrAssignments& inputAssignments) {
  // fill "real" variable assignments
  size_t i;
  for (i = 0;i < inputAssignments.size(); i++) {
    if (vas[i].var->getName() != inputAssignments[i].name) {
      throw ExecuteException(std::string("\"vas\" and \"inputAssignments\" are out of sync (")  + vas[i].var->getName() + std::string("!=") + inputAssignments[i].name + std::string(")"));
    }
    vas[i].value = inputAssignments[i].value;
  }

  // copy current state, pulses, stopWatch, clock, and random values into assignments
  for (size_t j = 0;j < stateCount; j++) {
    vas[i].value = stateManager.getState(vas[i].var->getName());
    i++;
  }
  for (size_t j = 0;j < pulseCount; j++) {
    vas[i].value = pulseManager.getPulse(vas[i].var->getName());
    i++;
  }
  for (size_t j = 0;j < timerCount; j++) {
    vas[i].value = timerManager.getTimer(vas[i].var->getName());
    i++;
  }
  for (size_t j = 0;j < stopWatchCount; j++) {
    vas[i].value = stopWatchManager.getStopWatch(vas[i].var->getName());
    i++;
  }
  for (size_t j = 0;j < clockCount; j++) {
    vas[i].value = clockManager.getClock(vas[i].var->getName());
    i++;
  }
  for (size_t j = 0;j < randomCount; j++) {
    vas[i].value = randomManager.getRandom(vas[i].var->getName());
    i++;
  }
  setSyscalls(vas, i);
  
  
  // find state changes between lastVarAssignments and vas
  Variables triggered;
  
  if (lastVarAssignments.empty()) {
    // this only happens the first time after (re)-parsing the scripts
    // at this point "everything" is a trigger except the "changes"
    for (VarAssignments::const_iterator v = vas.begin();
         v != vas.end();
         v++) {
      triggered.insert(*(v->var));
      lastVarAssignments.push_back(v->value);
    }

    for (size_t j = 0;j < value_changes.size(); j++) {
      vas.push_back(VarAssignment(value_changes[j], 0));
    }
    for (size_t j = 0;j < value_ons.size(); j++) {
      vas.push_back(VarAssignment(value_ons[j], 0));
    }
    for (size_t j = 0;j < value_offs.size(); j++) {
      vas.push_back(VarAssignment(value_offs[j], 0));
    }
    
  } else {
    
    // var direct
    for (i = 0;i < lastVarAssignments.size(); i++) {
      if (vas[i].value != lastVarAssignments[i]) {
        
        if (m_bPrintValueDebug && !m_debugValues.empty()) {
          if (std::find(m_debugValues.begin(), m_debugValues.end(), vas[i].var->getName())!=m_debugValues.end()) {
            std::stringstream ss;
            ss << IVDA::AbstrDebugOut::getTodayStr() << " " << vas[i].var->toString() << " : " << vas[i].value << std::endl;
            std::cout << ss.str();
            
            if (m_bMailValueDebug) {
              mailReport("variable changed: " + ss.str());
            }
          }
        }
        
        triggered.insert(*(vas[i].var));
      }
    }
    
    // change
    for (size_t j = 0;j < value_changes.size(); j++) {
      const size_t k = crossref_value_changes[j];
      
      if (vas[k].value != lastVarAssignments[k]) {
        
        vas[i].value = vas[k].value-lastVarAssignments[k];
        triggered.insert(*(vas[i].var));
        
        if (m_bPrintValueDebug && !m_debugValues.empty()) {
          if (std::find(m_debugValues.begin(), m_debugValues.end(), vas[i].var->getName())!=m_debugValues.end()) {
            
            std::stringstream ss;
            ss << IVDA::AbstrDebugOut::getTodayStr() << " " << vas[i].var->toString() << " : " << vas[i].value << std::endl;
            std::cout << ss.str();
            
            if (m_bMailValueDebug) {
              mailReport("change variable changed: " + ss.str());
            }
          }
        }
        
      } else {
        vas[i].value = 0;
      }

      i++;
    }
    
    // on
    for (size_t j = 0;j < value_ons.size(); j++) {
      const size_t k = crossref_value_ons[j];
      const bool value = (vas[k].value != lastVarAssignments[k]) && vas[k].value != 0;
      vas[i].value = value;
      
      if (m_bPrintValueDebug && !m_debugValues.empty()) {
        if (std::find(m_debugValues.begin(), m_debugValues.end(), vas[i].var->getName())!=m_debugValues.end()) {

          std::stringstream ss;
          ss << IVDA::AbstrDebugOut::getTodayStr() << " " << vas[i].var->toString() << " : " << vas[i].value << std::endl;
          std::cout << ss.str();
          
          if (m_bMailValueDebug) {
            mailReport("on variable changed: " + ss.str());
          }
          
        }
      }
      
      if (value) triggered.insert(*(vas[i].var));
      i++;
    }
    
    // off
    for (size_t j = 0;j < value_offs.size(); j++) {
      const size_t k = crossref_value_offs[j];
      const bool value = (vas[k].value != lastVarAssignments[k]) && vas[k].value == 0;
      vas[i].value = value;
      
      if (value && m_bPrintValueDebug && !m_debugValues.empty()) {
        if (std::find(m_debugValues.begin(), m_debugValues.end(), vas[i].var->getName())!=m_debugValues.end()) {

          std::stringstream ss;
          ss << IVDA::AbstrDebugOut::getTodayStr() << " " << vas[i].var->toString() << " : " << vas[i].value << std::endl;
          std::cout << ss.str();
          
          if (m_bMailValueDebug) {
            mailReport("off variable changed: " + ss.str());
          }
        
        }
      }
      
      if (value) triggered.insert(*(vas[i].var));
      i++;
    }

    for (i = 0;i < lastVarAssignments.size(); i++) {
      lastVarAssignments[i] = vas[i].value;
    }
  
  }

  if (!m_strDumpVariables.empty()) {
    std::ofstream outFile(m_strDumpVariables.c_str());
    if (outFile.is_open()) {
      outFile << "Variable assignements:\n";
      for (VarAssignments::const_iterator v = vas.begin();
           v != vas.end();
          v++) {
        outFile << " " << v->toString() << "\n";
      }
      outFile.close();
    }
    m_strDumpVariables = "";
  }

  
  if (logging != LT_off && !triggered.empty()) {
    char buff[DTTMSZ];
    
    std::ostream* output;
    std::ofstream foutput;
    if (logging == LT_file) {
      foutput.open( m_config->getLogFile().c_str(), std::ios::out | std::ios::app );
      output = &foutput;
    } else {
      output = &(std::cout);
    }
    
    *output << getDtTm (buff) << " Triggers:\n";
    for (Variables::const_iterator t = triggered.begin();
         t != triggered.end();
         t++) {

      *output << " " << t->toString();

      for (VarAssignments::const_iterator v = vas.begin();
           v != vas.end();
           v++) {
        if (*t == *(v->var)) {
           *output << " = " << v->value;
           break;
        }
      }
      *output << "\n";
    }
    *output << std::endl;
  }

  
  
  if (triggered.empty()) {
    VarStrAssignments outputChanges;
    return outputChanges;
  }
  
  // execute triggered commands
  std::vector<CommandPtr> cmds = expressionParser.execute(triggered, vas);

  
  if (logging != LT_off && !cmds.empty()) {
    char buff[DTTMSZ];

    std::ostream* output;
    std::ofstream foutput;
    if (logging == LT_file) {
      foutput.open( m_config->getLogFile().c_str(), std::ios::out | std::ios::app );
      output = &foutput;
    } else {
      output = &(std::cout);
    }
    
    *output << getDtTm (buff) << " Resulting internal Commands:" << std::endl;
    for (std::vector<CommandPtr>::const_iterator c = cmds.begin();
         c != cmds.end();
         c++) {
      *output << " " << (*c)->toString() << std::endl;
    }
    *output << std::endl;
  }
  
  return evaluateCommands(cmds);
}


std::string CommandLooper::setValueDebug(std::string varName) {
  
  if (varName.empty()) {
    m_debugValues.clear();
    return "cleared debug list";
  }
  
  std::stringstream ss;
  auto pos = std::find(m_debugValues.begin(), m_debugValues.end(), varName);
  if (pos == m_debugValues.end()) {
    m_debugValues.push_back(varName);
    ss << "Added " << varName << " to debug list. ";
  } else {
    m_debugValues.erase(pos);
    ss << "Removed " << varName << " from debug list. ";
  }
  
  if (m_debugValues.empty()) {
    ss << "The list is now empty.";
  } else {
    ss << "The list now contains: ";
    for (std::string elem : m_debugValues) {
      ss << elem << " ";
    }
  }
  
  return ss.str();
}


void CommandLooper::connectActivations(std::shared_ptr<HAS::HASBus> hasBus) {
  activationManager.connectActivations(hasBus);
}
