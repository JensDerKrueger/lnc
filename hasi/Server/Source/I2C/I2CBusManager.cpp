#include "I2CBusManager.h"

namespace I2C {
  
  I2CBusManager::I2CBusManager(uint8_t busID, uint8_t i2cAddress,
                               const std::string& devID, const std::string& hrname,
                               const HAS::HASConfigPtr config) :
  I2CMember(busID, i2cAddress, devID, hrname, config, nullptr),
  m_iCurrentBus(-1)
  {
  }
  
  int16_t I2CBusManager::getActiveBus() const {
    return m_iCurrentBus;
  }
  
  std::string I2CBusManager::getDesc() const {
    return "Unknown I2C Multiplexer";
  }
}