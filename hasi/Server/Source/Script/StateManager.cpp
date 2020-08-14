#include "StateManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "StateCommand.h"

#include <sstream>
#include <fstream>


StateManager::StateManager() {
}

bool StateManager::execute(Command* cmd)
{
  StateCommand*  sCmd = dynamic_cast<StateCommand*>(cmd);
  if (sCmd) {
    setState(sCmd->getExpressionName(),
             sCmd->getEvaluatedExpressionValue());
    return true;
  }
  return false;
}

void StateManager::save(const std::string& filename) const {
  std::ofstream file(filename);

  for (std::vector<StatePtr>::const_iterator i = States.begin();
       i < States.end();
       ++i) {
    file << (*i)->save() << std::endl;
  }
}

void StateManager::load(const std::string& filename) {
  States.clear();
  
  std::ifstream file(filename);
  std::string line;
  while (std::getline(file, line))
  {
    StatePtr t(new State());
    t->load(line);
    States.push_back(t);
  }
}

double StateManager::getState(const std::string& name) {
  return findState(name)->get();
}

void StateManager::setState(const std::string& name, double v) {
  return findState(name)->set(v);
}

std::string StateManager::toString() const {
  std::stringstream ss;
  for (std::vector<StatePtr>::const_iterator i = States.begin();
       i < States.end();
       ++i) {    
    ss << (*i)->toString() << std::endl;
  }
  
  return ss.str();
}

StatePtr StateManager::findState(const std::string& name) {
  for (std::vector<StatePtr>::const_iterator i = States.begin();
       i < States.end();
       ++i) {
    if ((*i)->getName() == name) {
      return *i;
    }
  }
  
  StatePtr t(new State(name));
  States.push_back(t);
  return t;
}


State::State(const std::string& _name) :
name(_name),
value(0.0)
{
}

const std::string& State::getName() const{
  return name;
}

double State::get() const {
  return value;
}

void State::set(double v) {
  value = v;
}


std::string State::toString() const {
  std::stringstream ss;
  ss << name << " is set to" << get();
  return ss.str();
}

std::string State::save() const {
  std::stringstream ss;
  ss << name << " " << get();
  return ss.str();
}

void State::load(const std::string& data) {
  std::vector<std::string> token;
  
  ParserTools::tokenize(data, token, " ");
  
  if (token.size() != 2)
    throw ParseException(std::string("Can't load state from line ") + data);
  
  name = token[0];
  value = atof(token[1].c_str());
}
