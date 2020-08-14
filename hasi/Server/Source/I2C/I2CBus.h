#pragma once

#ifndef I2CBUS_H
#define I2CBUS_H

#include <sstream>

#include "InputBlock.h"
#include "ThermoHKLM75.h"
#include "HKAnalogPCF8591.h"
#include "ArduPiDAC.h"
#include "LuminosityTSL2561.h"
#include "ProxiVCNL4000.h"
#include "RelayBlock.h"
#include "BlinkM.h"
#include "Mcp4725DAC.h"
#include "BoschBMP.h"
#include "HIH-6130.h"
#include "OLED.h"
#include "MB1242.h"
#include "BV4627.h"
#include "AI418S.h"
#include "TMP006.h"

#include "TCA9548ABusManager.h"

#include "../Tools/KeyValueFileParser.h"
#include "../Tools/SysTools.h"

#include <vector>
#include <map>
#include <memory>


namespace I2C {    
  class BusData {
  public:
    BusData() {}
    ~BusData();

    void init();
    bool usesBus(uint8_t id) const;

    // blocks
    std::map<const std::string, I2CMember*> m_vpAllBlocks;
    std::vector<InputBlock*> m_vpInputBlocks;
    std::vector<ThermoHKLM75*> m_vpThermoHKLM75s;
    std::vector<ArduPiDAC*> m_vpArduPiDACs;
    std::vector<HKAnalogPCF8591*> m_vpHKAnalogPCF8591s;
    std::vector<LuminosityTSL2561*> m_vpLumi2561s;
    std::vector<ProxiVCNL4000*> m_vpProxi4000s;
    std::vector<BoschBMP*> m_vpBoschBMPs;
    std::vector<HIH6130*> m_vpHIH6130s;
    std::vector<RelayBlock*> m_vpRelayBlocks;
    std::vector<Mcp4725DAC*> m_vpMcp4725DACs;
    std::vector<BlinkM*> m_vpBlinkMs;
    std::vector<OLED*> m_vpOLEDs;
    std::vector<MB1242*> m_vpMB1242s;
    std::vector<BV4627*> m_vpBV4627s;
    std::vector<AI418S*> m_vpAI418Ss;
    std::vector<TMP006*> m_vpTMP006s;
    
    std::vector<std::shared_ptr<I2CBusManager>> m_vI2CBusManager;
  };

  class I2CBus {
  public:
    I2CBus();
    ~I2CBus();
    void ParseDevices(HAS::HASConfigPtr config);
    void init();
    std::string toString() const;
    I2CMember* getDevice(const std::string& strID) const;

  protected:
    std::shared_ptr<BusData> m_BusData;

    void initSecondaryRPiI2CBus() const;

    template <class T> size_t addBlocks(KeyValueFileParser& parser,
                                        HAS::HASConfigPtr config,
                                        std::shared_ptr<BusData>& busData,
                                        std::vector<T*>& vec,
                                        const std::string& strID);
    
    template <class T> void addBusManager(KeyValueFileParser& parser,
                                            HAS::HASConfigPtr config,
                                            std::shared_ptr<BusData>& busData,
                                            const std::string& strID);
    
  };
}

#endif // I2CBUS_H

/*
 The MIT License
 
 Copyright (c) 2013 Jens Krueger
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

