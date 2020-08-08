#include "TCA9548ABusManager.h"

namespace I2C {
  
  
  TCA9548ABusManager::TCA9548ABusManager(uint8_t busID, uint8_t i2cAddress,
                                         const std::string& devID, const std::string& hrname,
                                         const HAS::HASConfigPtr config) :
  I2CBusManager(busID, i2cAddress, devID, hrname, config)
  {
  }
  
  void TCA9548ABusManager::setActiveBus(uint8_t iBusID) {
    if (iBusID > 7 || m_iCurrentBus == int16_t(iBusID)) return;
    
    I2CBase::write(m_DeviceHandle, 1 << iBusID);
    m_iCurrentBus = iBusID;
  }
  
  
  std::string TCA9548ABusManager::getDesc() const {
    return "Adafruit TCA9548A 1-to-8 I2C Multiplexer Breakout";
  }
}
