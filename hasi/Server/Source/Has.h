#pragma once

#ifndef HAS_H
#define HAS_H

#include "HASBasics.h"
#include "HASBus.h"
#include "HASConfig.h"
#include <Tools/DebugOutHandler.h>
#include <Tools/Timer.h>
#include <Tools/Threads.h>
#include <Script/CommandLooper.h>
#include <Network/RemoteListener.h>
#include <Network/HTTPServer.h>


#define HAS_VERSION 3.1

namespace HAS {

class BusDevice {
public:
  BusDevice(HASMember* device);
  uint32_t poll();
  
  enum DeviceType {
    Digital,
    Analog,
    other
  } deviceType;
  HASMember* device;
  
  
};

class ClConnection {
public:
  ClConnection(const std::string& name, HASMember* device, uint32_t channel);

  uint32_t getInChannelCount() const;
  uint32_t getOutChannelCount() const;
  void setValue(double v);
  double getValue();
  void applyValue();

  BusDevice::DeviceType deviceType;

  bool bOutput;
  bool bInput;

  std::string name;
  HASMember* device;
  uint32_t channel;
};

class Has {
public:
  Has(HASConfigPtr config);
  ~Has();

  bool init();
  void run(bool bRunAsDaemon);
  std::string ExecuteCommand(const std::string& cmdLine, bool& bTerminate);
  
  static std::string getVersion();
  static std::string getLogo();

  static double getBaseVersion();
  static uint32_t getRevisionVersion();

private:
  
  IVDA::CriticalSection m_HASCS;
  std::shared_ptr<HAS::HASBus> m_hasBus;

  std::shared_ptr<IVDA::LambdaThread> m_WatchdogPinger;

  IVDA::CriticalSection m_CSCommandExecution;
  std::shared_ptr<IVDA::LambdaThread> m_eventLooper;

  float m_fOverloadRatio;
  float m_fLoadRatio;

  std::shared_ptr<CommandLooper> m_commandLooper;
  std::vector<ClConnection> m_clConnections;
  std::vector<BusDevice> m_clPollDevices;
  std::vector<ClConnection> m_clOutputs;
  
  HASConfigPtr m_config;
  uint32_t m_pcState;
  uint32_t m_pcKeepAlive;
  uint32_t m_WatchdogDelay;
  
  std::string getHostname() const;
  void mailDebugMsg(const std::string& msg) const;
  
  std::shared_ptr<RemoteListener> m_remoteListener;
  std::shared_ptr<HTTPServer> m_httpServer;
  
  bool m_bReInitInProgress;

  bool initBus();
  void shutdownBus();

  void pingWatchdog(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface);

  static std::string GetCommandString(const std::string& prompt);
  void eventLoop(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface);

  bool testScript();
  bool reloadScript();
  bool initCommandLooper();
  void processCommands(IVDA::Predicate pContinue);
  void pollDevices();
  void connectDevices();
  void applyCommands(const VarStrAssignments& va);

  void readShell(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface);
  
  void askAndExecute();
  void execute(std::string cmd, bool bOutput = true);
  void askAndDebug();
  
  void printThreadStatus();

};
}

#endif // HAS_H

/*
 The MIT License

 Copyright (c) 2013-2015 Jens Krueger

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

