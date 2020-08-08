#pragma once

#ifndef TCA9548A_BUSMANAGER_H
#define TCA9548A_BUSMANAGER_H

#include "I2CBusManager.h"

namespace I2C {
  
  class TCA9548ABusManager : public I2CBusManager {
  public:
    TCA9548ABusManager(uint8_t busID, uint8_t i2cAddress,
                       const std::string& devID,
                       const std::string& hrname,
                       const HAS::HASConfigPtr config);
    
    virtual void setActiveBus(uint8_t iBusID);
    
  protected:
    virtual std::string getDesc() const;
    
  };
  
  typedef std::shared_ptr<TCA9548ABusManager> TCA9548ABusManagerPtr;
  
}
#endif // TCA9548A_BUSMANAGER_H

/*
 Copyright (c) 2016 Jens Krueger
 
 due to Gordon's code being LGPL this code is also LGPL
 
 this is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.
 
 wiringPi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this file.
 If not, see <http://www.gnu.org/licenses/>.
 */
