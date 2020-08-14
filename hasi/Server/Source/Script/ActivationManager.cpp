#include "ActivationManager.h"
#include "ActivationCommand.h"

ActivationManager::ActivationManager(const HAS::HASConfigPtr config)
: m_config(config)
{  
}

bool ActivationManager::execute(const Command* cmd) {
  const ActivationCommand* aCmd = dynamic_cast<const ActivationCommand*>(cmd);
  if (aCmd) {
    return setActivation(aCmd->getDeviceName(),
                         aCmd->getCmdType() == ActivationCommand::activate);
  } else {
    return false;
  }
}

void ActivationManager::connectActivations(std::weak_ptr<HAS::HASBus> hasBus) {
  m_hasBus = hasBus;
}

bool ActivationManager::setActivation(const std::string& deviceName, bool bActive) {
  if (auto hasBus = m_hasBus.lock()) {
    try {
      HAS::HASMember* device = hasBus->getDevice(deviceName);
      device->setIsActive(bActive);
      return true;
    } catch (const HAS::EDeviceNotFound& ) {
      return false;
    }
  } else {
    return false;
  }
}

