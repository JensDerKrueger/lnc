#include "PulseManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "PulseCommand.h"

#include <sstream>
#include <fstream>

PulseManager::PulseManager() {
}

bool PulseManager::execute(Command* cmd)
{
  PulseCommand*  pCmd = dynamic_cast<PulseCommand*>(cmd);
  if (pCmd) {
    setPulse(pCmd->getExpressionName(),
             pCmd->getEvaluatedExpressionValue());
    return true;
  }
  return false;
}

double PulseManager::getPulse(const std::string& name) {
  return findPulse(name)->get();
}

void PulseManager::setPulse(const std::string& name, double v) {
  return findPulse(name)->set(v);
}

std::string PulseManager::toString() const {
  std::stringstream ss;
  for (std::vector<PulsePtr>::const_iterator i = Pulses.begin();
       i < Pulses.end();
       ++i) {    
    ss << (*i)->toString() << std::endl;
  }
  
  return ss.str();
}

PulsePtr PulseManager::findPulse(const std::string& name) {
  for (std::vector<PulsePtr>::const_iterator i = Pulses.begin();
       i < Pulses.end();
       ++i) {
    if ((*i)->getName() == name) {
      return *i;
    }
  }
  
  PulsePtr t(new Pulse(name));
  Pulses.push_back(t);
  return t;
}


Pulse::Pulse(const std::string& _name) :
name(_name),
value(0.0)
{
}

const std::string& Pulse::getName() const{
  return name;
}

double Pulse::get() {
  double v = value;
  value = 0;
  return v;
}

void Pulse::set(double v) {
  value = v;
}


std::string Pulse::toString() const {
  std::stringstream ss;
  ss << name << " is set to" << value;
  return ss.str();
}