#pragma once

#include <string>
#include <vector>

#include "Expression.h"
#include "HASConfig.h"
#include "HASBus.h"

class Command;

class ActivationManager {
public:
  ActivationManager(const HAS::HASConfigPtr config);
  bool execute(const Command* cmd);
  void connectActivations(std::weak_ptr<HAS::HASBus> hasBus);




private:
  HAS::HASConfigPtr m_config;
  std::weak_ptr<HAS::HASBus>m_hasBus;
  
  bool setActivation(const std::string& deviceName, bool bActive);
  
};


