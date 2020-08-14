#include "Has.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <system_error>
#include <cstring> // memset

#ifndef _MSC_VER
  #include <netdb.h> // hostname
#else
  #define NOMINMAX
  #include <windows.h>
#endif


#include <Tools/Watchdog.h>
#include <Tools/SysTools.h>
#include <Tools/MailReporter.h>
#include <Script/ParserTools.h>

#include "other-devices/SysInfo.h" // getBoardName


using namespace HAS;
using namespace IVDA;

Has::Has(HASConfigPtr config) :
m_hasBus(nullptr),
m_WatchdogPinger(nullptr),
m_eventLooper(nullptr),
m_fOverloadRatio(-1.0f),
m_fLoadRatio(-1.0f),
m_commandLooper(nullptr),
m_config(config),
m_pcState(0),
m_pcKeepAlive(1),
m_WatchdogDelay(0),
m_remoteListener(nullptr),
m_httpServer(nullptr),
m_bReInitInProgress(false)
{
  m_remoteListener = std::make_shared<RemoteListener>();
  m_hasBus = std::make_shared<HAS::HASBus>();
}

Has::~Has() {
}


void Has::shutdownBus() {
  m_hasBus = std::make_shared<HAS::HASBus>();
}

bool Has::initBus() {
  if (m_config == nullptr) {
    IVDA_ERROR("InitBus called without a valid configuration");
    return false;
  }
  
  try {
    m_hasBus->parseDevices(m_config);
  } catch (const HAS::EDeviceParser& e) {
    IVDA_ERROR("Error reading device file: " << e.what());
    return false;
  }
  
  try {
    m_hasBus->init();
  } catch (const HAS::EDeviceInit& e) {
    IVDA_ERROR("Error during device init: " << e.what());
    return false;
  }
  return true;
}

void Has::connectDevices() {
  m_clConnections.clear();
  m_clOutputs.clear();
  m_clPollDevices.clear();

  std::vector<std::string> clStrConnections = m_commandLooper->getConnections();

  for (auto c = clStrConnections.begin();
       c != clStrConnections.end();
       c++) {

    if (c->empty()) {
      throw ParseException("empty ID found");
    }

    std::vector< std::string > token = SysTools::Tokenize(*c, IVDA::SysTools::PM_CUSTOM_DELIMITER, '.');
    if (token.size() > 2) {
      throw ParseException(std::string("too many dots in id ") + *c);
    }

    std::string deviceName = token[0];

    int deviceChannel = (token.size() > 1) ? atoi(token[1].c_str())-1 : 0;

    HASMember* devicePtr;

    try {
      devicePtr = m_hasBus->getDevice(deviceName);
    } catch (const HAS::EDeviceNotFound& ) {
      throw ParseException(std::string("unknown device ") + deviceName + std::string(" found in id ") + *c);
    }

    if (devicePtr == NULL) {
      throw ParseException(std::string("unknown device ") + deviceName + std::string(" found in id ") + *c);
    }

    // if this is a new device -> add it to the poll device list
    bool bFound = false;
    for (auto d = m_clPollDevices.begin();
         d != m_clPollDevices.end();
         d++) {
      if (d->device == devicePtr) {
        bFound =true;
        break;
      }
    }
    if (!bFound) {
      m_clPollDevices.push_back(BusDevice(devicePtr));
    }

    ClConnection connection(*c, devicePtr, deviceChannel);
    int iChannelCount = connection.getInChannelCount();
    if (deviceChannel < 0 || deviceChannel >= iChannelCount) {
      std::stringstream ss;
      ss << "invalid input device channel count for " << *c << " (min:1, max:" << iChannelCount << ")";
      throw ParseException(ss.str());
    }

    m_clConnections.push_back(connection);
  }

  std::vector<std::string> clStrOutputs = m_commandLooper->getOutputs();
  for (auto o = clStrOutputs.begin();
       o != clStrOutputs.end();
       o++) {
    if (o->empty()) {
      throw ParseException("empty ID found");
    }

    std::vector< std::string > token = SysTools::Tokenize(*o, IVDA::SysTools::PM_CUSTOM_DELIMITER, '.');
    if (token.size() > 2) {
      throw ParseException(std::string("too many dots in id ") + *o);
    }

    std::string deviceName = token[0];
    int deviceChannel = (token.size() > 1) ? atoi(token[1].c_str())-1 : 0;
    HASMember* devicePtr = m_hasBus->getDevice(deviceName);
    if (devicePtr == NULL) {
      throw ParseException(std::string("unknown device ") + deviceName + std::string(" found in id ") + *o);
    }
    ClConnection connection(*o, devicePtr, deviceChannel);

    int iChannelCount = connection.getOutChannelCount();
    if (deviceChannel < 0 || deviceChannel >= iChannelCount) {
      throw ParseException(std::string("invalid output device channel count for ") + *o);
    }

    m_clOutputs.push_back(connection);
  }

  m_commandLooper->connectActivations(m_hasBus);
}

bool Has::initCommandLooper() {
  try {
    m_commandLooper = std::shared_ptr<CommandLooper>(new CommandLooper(m_config));
    IVDA_MESSAGE("Script parsed ok");
  } catch (const StrException& e) {
    IVDA_ERROR("Error during command init: " << e.what());
    return false;
  }

  try {
    connectDevices();
    IVDA_MESSAGE("Script connected ok");
  } catch(const ParseException& e) {
    IVDA_ERROR("Loaded script successfully, but device connection failed: " << e.what());
    return false;
  } catch(const EDeviceNotFound& e) {
    IVDA_ERROR("Loaded script successfully, but unknown device was detected: " << e.what());
    return false;
  }
  return true;
}

bool Has::testScript() {
  try {
    m_commandLooper->reParse(false);
  } catch (const StrException& e) {
    IVDA_ERROR("Error reloading script: " << e.what());
    return false;
  }
  return true;
}

bool Has::reloadScript() {
  try {
    m_commandLooper->reParse(true);
    if (m_httpServer) m_httpServer->reloadWhitelist();
  } catch (const StrException& e) {
    IVDA_ERROR("Error reloading script: " << e.what());
    return false;
  }

  try {
    connectDevices();
  } catch(const ParseException& e) {
    IVDA_ERROR("Loaded script successfully, but device connection failed: " << e.what());
    return false;
  } catch(const EDeviceNotFound& e) {
    IVDA_ERROR("Loaded script successfully, but unknown device was detected: " << e.what());
    return false;
  }
  
  return true;
}

bool Has::init() {
  std::string boardname = SysInfo::getBoardName();
  
#ifdef NDEBUG
  IVDA_MESSAGE("Home automation server initializing on " << boardname);
#else
  IVDA_MESSAGE("Home automation server initializing " << boardname << " (DEBUG BUILD)");
#endif

  IVDA_MESSAGE("Initializing bus ...");
  if (!initBus()) return false;

  IVDA_MESSAGE("Initializing command script ...");
  if (!initCommandLooper()) return false;

  IVDA_MESSAGE("Initialization successfully completed");
  return true;
}

std::string Has::GetCommandString(const std::string& prompt) {
  std::string cmdLine;
  std::cout << prompt;
  std::getline(std::cin, cmdLine);
  
  if (std::cin.eof()) {
    IVDA_WARNING("Caught Ctrl-D (EOF) resetting input. "
                 "To terminate HAS press Ctrl-C.");
    std::cin.clear();
  }
  
  return IVDA::SysTools::TrimStr(cmdLine);
}

void Has::mailDebugMsg(const std::string& msg) const {
  if (m_config->getFromAddress().empty() || m_config->getToAddress().empty()) {
    IVDA_WARNING("no email address specified in config");
    return;
  }
  
  std::stringstream title;
  if(m_config->getSystemName().empty()) {
    title << "HAS Debug Event";
  } else {
    title << "Has " << m_config->getSystemName() << " Debug Event";
  }
  
  MailReporter rep(title.str(), msg);
  rep.SetFrom(m_config->getFromAddress(),m_config->getSystemName());
  rep.SendTo(m_config->getToAddress());
}

void Has::askAndDebug() {
  if (!m_commandLooper) {
    IVDA_ERROR("Command looper not active (yet).");
    return;
  }
  
  m_commandLooper->printValueDebug(false);
  std::string varName = ParserTools::removeSpaces(ParserTools::removeComments(GetCommandString("value name (without brackets or prefix)>")));
  {
    SCOPEDLOCK(m_HASCS);
    const std::string msg = m_commandLooper->setValueDebug(varName);
    IVDA_MESSAGE(msg);
    
    if (m_commandLooper->getMailValueDebug()) {
      mailDebugMsg(msg);
    }
    
  }
  m_commandLooper->printValueDebug(true);
  
}

void Has::askAndExecute() {
  if (!m_commandLooper) {
    IVDA_ERROR("Command looper not active (yet).");
    return;
  }
  
  std::string cmd = GetCommandString("command>");
  execute(cmd);
}

void Has::execute(std::string cmd, bool bOutput) {
  cmd = ParserTools::removeSpaces(ParserTools::removeComments(cmd));
  
  CommandVec cv;
  try {
    ExpressionParser::ParseRHS(cmd, cv, m_commandLooper->getCurrentVas());
  } catch(const ParseException& e) {
    IVDA_ERROR("Error reading command: " << e.what());
    return;
  } catch (const ExecuteException& e) {
    IVDA_ERROR("Evaluation executing command: " << e.what());
    return;
  }

  {
    SCOPEDLOCK(m_HASCS);
    try {
      VarStrAssignments vaNew = m_commandLooper->evaluateCommands(cv);
      applyCommands(vaNew);
    } catch (const ExecuteException& e) {
      IVDA_ERROR("Error executing command: " << e.what());
      return;
    }
    if (bOutput) IVDA_MESSAGE("Command " << cmd << " executed sucesfully");
  }
}

void Has::printThreadStatus() {
  if (m_WatchdogPinger && m_WatchdogPinger->IsRunning())
    IVDA_MESSAGE("WatchdogPinger is running");
  else
    IVDA_WARNING("WatchdogPinger is NOT running");

  if (m_eventLooper && m_eventLooper->IsRunning())
    IVDA_MESSAGE("eventLooper is running");
  else
    IVDA_WARNING("eventLooper is NOT running");

  if (m_httpServer && m_httpServer->isRunning())
    IVDA_MESSAGE("http server is running");
  else
    IVDA_WARNING("http server is NOT running");
  
  m_commandLooper->printThreadStatus();
}

std::string Has::ExecuteCommand(const std::string& cmdLine, bool& bTerminate) {
  SCOPEDLOCK(m_CSCommandExecution);
  std::stringstream ss;
  bTerminate = false;
    if (cmdLine.length() > 0) {
      switch (IVDA::SysTools::ToLowerCase(cmdLine)[0]) {
        case '?' : {
          ss << "q: quit\n"
             << "h: display has load\n"
             << "l: list bus devices\n"
             << "d: dump states into \"state_dump.txt\" file\n"
             << "r: report changes of a specific variable\n"
             << "o: toggle screen event-logging\n"
             << "e: toggle file event-logging\n"
             << "c: enter hasi command\n"
             << "i: re-initialize bus\n"
             << "s: update script\n"
             << "w: write expanded script to disk\n"
             << "m: toggle email variable reports\n"
             << "a: list active remotes\n"
             << "x: restart remote listener\n"
             << "+: increase event delay\n"
             << "-: decrease event delay";
          break;
        }
        case 'q' : {
          if (m_eventLooper->IsRunning())
            m_eventLooper->RequestThreadStop();
          bTerminate = true;
          break;
        }
        case 'h' : {
          ss << "Has load is " << m_fLoadRatio*100 << " %, overload ratio "
          << m_fOverloadRatio*100 << " %, event delay is set to "
          << m_config->getEventLoopDelay() << ((m_config->getEventLoopDelayMode() != HASConfig::dm_manual) ? " ms automatically" : " ms");
          break;
        }
        case '+' : {
          m_config->setEventLoopDelay(m_config->getEventLoopDelay()*2);
          ss << "Event delay set to " << m_config->getEventLoopDelay() << " ms";
          break;
        }
        case '-' : {
          m_config->setEventLoopDelay(m_config->getEventLoopDelay()/2);
          ss << "Event delay set to " << m_config->getEventLoopDelay() << " ms";
          break;
        }
        case 't' : {
          if (!m_commandLooper) break;
          printThreadStatus();
          break;
        }
        case 'o' : {
          if (!m_commandLooper) break;
          
          if (m_commandLooper->getEventLogging() == CommandLooper::LT_off) {
            m_commandLooper->setEventLogging(CommandLooper::LT_screen);
          } else {
            m_commandLooper->setEventLogging(CommandLooper::LT_off);
          }
          
          ss << "Event logging: ";
          if (m_commandLooper->getEventLogging() != CommandLooper::LT_off)
          ss << "enabled\n";
          else
          ss << "disabled\n";
          break;
        }
        case 'e' : {
          if (!m_commandLooper) break;

          if (m_commandLooper->getEventLogging() == CommandLooper::LT_off) {
            m_commandLooper->setEventLogging(CommandLooper::LT_file);
          } else {
            m_commandLooper->setEventLogging(CommandLooper::LT_off);
          }

          ss << "Event logging: ";
          if (m_commandLooper->getEventLogging() != CommandLooper::LT_off)
            ss << "enabled\n";
          else
            ss << "disabled\n";
          break;
        }
        case 'd' : {
          if (!m_commandLooper) break;
          m_commandLooper->dumpStates("state_dump.txt");
          break;
        }
        case 'w' : {
          if (!m_commandLooper) break;
          if (!m_commandLooper->writeParsedScript(std::string("expanded_")+m_config->getHASIFile())) {
            ss << "Error writing expanded script\n";
          }
          break;
        }
        case 'l' : {
          ss << "Listing HAS-Bus:\n" << m_hasBus->toString();
          break;
        }
        case 'a' : {
          ss << "Listing active remotes:\n" << m_remoteListener->toString();
          break;
        }
        case 'm' : {
          const bool toggledReport = !m_commandLooper->getMailValueDebug();
          m_commandLooper->setMailValueDebug(toggledReport);
          
          if (toggledReport) {
            ss << "Reporting via console AND email";
          } else {
            ss << "Reporting ONLY via console";
          }
          
          mailDebugMsg("status changed: " + ss.str());
          
          break;
        }
        case 'x' : {
          SCOPEDLOCK(m_HASCS);
          m_remoteListener->shutdown();
          m_remoteListener = nullptr;
          
          m_remoteListener = std::make_shared<RemoteListener>();
          m_remoteListener->init(m_config);
          break;
        }
        case 'i' : {
          SCOPEDLOCK(m_HASCS);
          
          m_bReInitInProgress= true;
          shutdownBus();
          init();
          m_bReInitInProgress = false;
          
          break;
        }
        case 's' : {
          SCOPEDLOCK(m_HASCS);
          
          if (!testScript())
            break;
          
          if (reloadScript()) {
            ss << "Script reload successful";
          } else  {
            if (m_eventLooper->IsRunning())
              m_eventLooper->RequestThreadStop();
            bTerminate = true;
          }
          break;
        }
        case 'c' : {
          askAndExecute();
          break;
        }
        case 'r' : {
          askAndDebug();
          break;
        }
       default:
          ss << "Unknown command " << cmdLine;
          throw ParseException(ss.str());
      }
    }

  return ss.str();
}


void Has::pingWatchdog(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  uint32_t iStuckCounter = 0;
  try {
    Watchdog w;
    
    uint32_t interval;
    if (m_config->getWatchdogInterval() != 0 && w.getInterval() != m_config->getWatchdogInterval()) {

      IVDA_MESSAGE("Adjusting watchdog timeout to " << m_config->getWatchdogInterval() << " seconds");
      w.setTimeout(m_config->getWatchdogInterval());

      interval = uint32_t(0.6f*m_config->getWatchdogInterval());
      if (w.getSaveInterval() < int32_t(interval)) {
        IVDA_WARNING("The specified Watchdog interval (" << m_config->getWatchdogInterval() <<
                     ") is longer than the inferred save interval " << w.getSaveInterval() << ". The system may reboot unexpectedly at any time.");
      }

    } else {
      if (w.getSaveInterval() <= 0) {
        IVDA_WARNING("Watchdog interval is invalid, skipping watchdog pinger initialization.");
        return;
      }
      interval = uint32_t(w.getSaveInterval());
    }
    
    if (interval == 0) {
      IVDA_WARNING("Watchdog interval is invalid, skipping watchdog pinger initialization.");
      return;
    }
    
    // getInterval returns a safe interval in seconds
    // how long to wait inbetween watchdog pings
    m_WatchdogDelay = interval*1000;
    
    // to avoid long waiting at shutdown split the
    // m_WatchdogDelay up into subinervals of around 100 ms
    uint32_t subIntervalCount = std::max<uint32_t>(1,m_WatchdogDelay/100);
    uint32_t subIntervalLength = std::max<uint32_t>(1,m_WatchdogDelay/subIntervalCount);
    
    /*
    IVDA_MESSAGE("Watchdog kicking of " << m_WatchdogDelay/1000.0f << "s is split up into "
                 << subIntervalCount << " subintervals of " << subIntervalLength << " ms length.");
    */
    
    const uint32_t iStuckCounterMax = 60/interval; // number of ping-iterations in one minute
    
    while (!pContinue || pContinue()) {
      w.sendKeepalive();
      
      if (m_pcKeepAlive != 0 && !m_bReInitInProgress) {
        iStuckCounter = 0;
        m_pcKeepAlive = 0;
      } else {
        if (iStuckCounter > 0) // don't show a warning if this happens only once 
          IVDA_WARNING("processCommands took unusually long for state " << m_pcState << " (Warning " << iStuckCounter+1 << " of " << iStuckCounterMax+2 << ").");
        
        if (iStuckCounter > iStuckCounterMax) {
          // savety switch: freeze this pinger, so the system will reset
          //                but before doing so record the m_pcState
          IVDA_ERROR("processCommands has not made progress in the last " << (iStuckCounter+1)*interval
                      << " seconds. It seems to be stuck in state "
                      << m_pcState <<". Reseting via the watchdog.");
          
          w.sendKeepalive(); // make sure we do not restart during the following sync command
          do {
          } while (system("sync") != 0); // make sure all buffers are written to disk, before restarting
          delay(w.getInterval()*1000*4); // four times the max interval -> this should trigger the watchdog
        }
        iStuckCounter++;
      }
      
      for (uint32_t subInterval = 0;subInterval<subIntervalCount;++subInterval) {
        delay(subIntervalLength);
        if (pContinue && !pContinue()) break;
      }
    }
  } catch (const std::system_error& e) {
    IVDA_WARNING("Unable to start watchdog, system running unguarded (" << e.what() << ")");
  }
}


void Has::readShell(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  IVDA_MESSAGE("Shell reader running");
  bool bTerminate = false;
  while (!bTerminate && (!pContinue || pContinue() )) {
    std::string cmdLine = GetCommandString("Enter command (enter ? for help): \n");
    try {
      std::string result = ExecuteCommand(cmdLine, bTerminate);
      if (!result.empty()) std::cout << result << std::endl;
    } catch (const ParseException& e) {
      IVDA_WARNING(e.what());
    }
  }
  IVDA_MESSAGE("Shell reader stopped");
  
  if (m_eventLooper->IsRunning()) {
    // if at this point the event looper is still running
    // this means the shutdown was initiated from this reader
    // e.g. by pressing the "quit"-key in that case the
    // event looper should have reaceived the terminate command
    // already.
    // Now, we give it another 30 seconds to quit and forecfully
    // shut it down otherwise
    IVDA_MESSAGE("Waiting for event looper to finish");
    if (!m_eventLooper->JoinThread(30000) && m_eventLooper->IsRunning()) {
      IVDA_WARNING("Timout on event (current state: " << m_pcState
                  << ") looper termination, killing thread");
      m_eventLooper->KillThread();
    }
  }
}

std::string Has::getHostname() const {

#ifndef _MSC_VER
  struct addrinfo hints, *info=nullptr, *p=nullptr;
  int gai_result;
  
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;
  
  if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
    return "";
  }
  
  std::string name;
  if (p) {
    name = std::string(p->ai_canonname); // simply return first
  } else {
    name = std::string(hostname);
  }
  freeaddrinfo(info);
  
  return name;
#else
  char Name[150];
  DWORD bufCharCount = 150;
  memset(Name, 0, 150);
  if( !GetComputerNameA( Name, &bufCharCount ) ) {
    return "";
  } else {
    return Name;
  }
#endif
}

void Has::run(bool bRunAsDaemon) {

#ifndef NWDOG
  // startup watchdog
  m_WatchdogPinger = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&Has::pingWatchdog, this, std::placeholders::_1, std::placeholders::_2)));
  m_WatchdogPinger->StartThread();
#endif
  
  m_eventLooper = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&Has::eventLoop, this, std::placeholders::_1, std::placeholders::_2)));
  m_eventLooper->StartThread();

  // give the event loop some time to start
  delay(500);

  std::shared_ptr<IVDA::LambdaThread> shellReader = nullptr;

  if (!bRunAsDaemon) {
    shellReader = std::shared_ptr<IVDA::LambdaThread>(new IVDA::LambdaThread(std::bind(&Has::readShell, this, std::placeholders::_1, std::placeholders::_2)));
    shellReader->StartThread();
  } else {
    IVDA_MESSAGE("Skipped shell reader in daemon mode");
  }

  if (m_config->getHTTPPort() != 0) {
    IVDA_MESSAGE("Starting HTTP Server");
    m_httpServer = std::make_shared<HTTPServer>(m_config);
  } else {
    IVDA_MESSAGE("HTTP Server Disabled");
  }
  
  m_remoteListener->init(m_config);
  
  // wait until event looper termintates
  m_eventLooper->JoinThread();

  if (m_httpServer) m_httpServer->requestStop();

  // shutdown watchdog
  if (m_WatchdogPinger) {
    IVDA_MESSAGE("Waiting for watchdog to finish");

    m_WatchdogPinger->RequestThreadStop();
    m_WatchdogPinger->JoinThread(uint32_t(m_WatchdogDelay*1.5));

    if (m_WatchdogPinger->IsRunning()) {
      IVDA_WARNING("Watchdog not responding, killing thread.");
      m_WatchdogPinger->KillThread();
    }
  }

  // shutdown the command reader thread
  if (shellReader && !shellReader->JoinThread(1000)) {
    if (shellReader->IsRunning()) {
      IVDA_MESSAGE("Terminating shell reader");
      // shell reader is most likely sitting in the blocking readline
      // waiting for user input, so we have to kill it
      shellReader->KillThread();
    }
  }

  m_remoteListener->shutdown();
  
  IVDA_MESSAGE("Home automation event loop has terminated");
}

void Has::eventLoop(IVDA::Predicate pContinue, IVDA::LambdaThread::Interface& threadInterface) {
  if (!m_config->getFromAddress().empty() && !m_config->getToAddress().empty()) {
    std::stringstream title;
    const std::string hostname = getHostname();
    if(hostname.empty()) {
      title << "Has has started.";
    } else {
      title << "Has " << hostname << " has started.";
    }
    MailReporter rep(m_config->getSystemName() + " Startup Event",title.str());
    rep.SetFrom(m_config->getFromAddress(),m_config->getSystemName());
    rep.SendTo(m_config->getToAddress());
  } else {
    IVDA_WARNING("Start notification not send due to missing mail information.");
  }

  IVDA_MESSAGE("Home automation server running");
  
  switch (m_config->getEventLoopDelayMode()) {
    case HASConfig::dm_basedOnLoad:
      IVDA_MESSAGE("Delay will be set to keep the load below " << m_config->getAutoEventLoopDelayMax()*100 << " %");
      IVDA_MESSAGE("Initial delay value is " << m_config->getEventLoopDelay() << " ms");
      break;
    case HASConfig::dm_basedOnOverload:
      IVDA_MESSAGE("Delay will be set to keep the overload below " << m_config->getAutoEventLoopDelayMax()*100 << " %");
      IVDA_MESSAGE("Initial delay value is " << m_config->getEventLoopDelay() << " ms");
      break;
    case HASConfig::dm_manual:
    default:
      IVDA_MESSAGE("Delay manually set to " << m_config->getEventLoopDelay() << " ms");
      break;
  }
  
  int64_t sumDelay = 0;
  uint32_t normalIter = 0;
  uint32_t overloadIter =0;
  
  IVDA::Timer runTimer;
  IVDA::Timer restoreTimer;

  if (m_config->getRestorePointInterval() > 0)
    restoreTimer.Start();
  
  while (!pContinue || pContinue() ) {
    runTimer.Start();
    try {
      processCommands(pContinue);
    } catch (const HAS::EDeviceNotFound& e) {
      IVDA_ERROR("Unable to get handle to device " << e.what());
      return;
    }

    
    uint32_t elapsedMS = uint32_t(runTimer.Elapsed());
    if (m_config->getEventLoopDelay() > elapsedMS) {
      delay(m_config->getEventLoopDelay() - elapsedMS);
      normalIter++;
    } else {
      overloadIter++;
    }
    sumDelay += int64_t(m_config->getEventLoopDelay()) - int64_t(elapsedMS);

    
    // update every 5000 iterarions
    if (normalIter + overloadIter > 5000) {
      if (normalIter == 0) {
        normalIter = 1; // avoid NaN in the next line
        overloadIter--;
      }
      m_fOverloadRatio = float(overloadIter)/float(normalIter);
      m_fLoadRatio = float((double(m_config->getEventLoopDelay())-(double(sumDelay)/double(normalIter + overloadIter)))/double(m_config->getEventLoopDelay()));
      m_commandLooper->setSysValues(m_fLoadRatio,m_fOverloadRatio,m_config->getEventLoopDelay());
      
      switch (m_config->getEventLoopDelayMode()) {
        case HASConfig::dm_basedOnLoad:
          if (m_config->getEventLoopDelay() > 1 && m_fLoadRatio < m_config->getAutoEventLoopDelayMax() ) {
            m_config->setEventLoopDelay(m_config->getEventLoopDelay()-1);
          }
          if(m_fLoadRatio > m_config->getAutoEventLoopDelayMax()) {
            m_config->setEventLoopDelay(m_config->getEventLoopDelay()+1);
          }
          break;
        case HASConfig::dm_basedOnOverload:
          if (m_config->getEventLoopDelay() > 1 && m_fOverloadRatio < m_config->getAutoEventLoopDelayMax() ) {
            m_config->setEventLoopDelay(m_config->getEventLoopDelay()-1);
          }
          if(m_fOverloadRatio > m_config->getAutoEventLoopDelayMax()) {
            m_config->setEventLoopDelay(m_config->getEventLoopDelay()+1);
          }
          break;
        case HASConfig::dm_manual:
        default:
          break;
      }
      
      sumDelay = 0;
      normalIter = 0;
      overloadIter = 0;
    }
    
    
    if (m_config->getRestorePointInterval() > 0) {
      uint64_t restoreMS = uint64_t(restoreTimer.Elapsed());
      if (restoreMS > uint64_t(m_config->getRestorePointInterval())*1000) {
        m_commandLooper->saveState();
        restoreTimer.Start();
      }
    }
    
  }

  IVDA_MESSAGE("Home automation server terminating");
}

void Has::applyCommands(const VarStrAssignments& vaNew) {
  for (auto newVar = vaNew.begin();
       newVar != vaNew.end();
       newVar++) {

    const std::string name = newVar->name;
    const double value = newVar->value;
    bool bFound = false;

    for (auto var = m_clOutputs.begin();
         var != m_clOutputs.end();
         var++) {
      if (name == var->name) {
        var->setValue(value);
        bFound = true;
        break;
      }
    }
    if (!bFound)
      IVDA_WARNING(std::string("Cannot find output ")+name);
  }

  for (auto var = m_clOutputs.begin();
       var != m_clOutputs.end();
       var++) {
    var->applyValue();
  }
}

void Has::pollDevices() {
  uint32_t delayms = 0;
  
  for (auto dev = m_clPollDevices.begin();
       dev != m_clPollDevices.end();
       dev++) {
    delayms = std::max<uint32_t>(delayms,
                                 dev->poll());
  }
  
  if (delayms > 0) {
    if (delayms > 1000) {
      IVDA_WARNING("Caught large poll delay (" << delayms << " ms), waiting the maxmimum time of 1 second.");
      delayms = 1000;
    }
    
    delay(delayms);
  }
}

void Has::processCommands(IVDA::Predicate pContinue) {
  m_pcKeepAlive++;
  m_pcState = 1;

  SCOPEDLOCK(m_HASCS);
  
  m_pcState = 2;

  try {
    std::vector<std::string> remoteCommands = m_remoteListener->getCommands();
    for (const std::string& s : remoteCommands) {
      execute(s, false);
    }
  } catch (const ExecuteException& e) {
    IVDA_ERROR("Error executing remote command: " << e.what());
    m_pcState = 0;
    return;
  }

  m_pcState = 3;

  if (m_httpServer && m_httpServer->isRunning()) {
    try {
      std::string cmd = m_httpServer->syncState(m_commandLooper->getCurrentVas());
      execute(cmd, false);
    } catch (const ExecuteException& e) {
      IVDA_ERROR("Error executing remote command: " << e.what());
      m_pcState = 0;
      return;
    }
  }

  m_pcState = 4;
  
  if (pContinue && !pContinue()) {
    m_pcState = 0;
    return;
  }

  m_pcState = 5;

  pollDevices();
  
  m_pcState = 6;
  
  VarStrAssignments va;
  va.reserve(m_clConnections.size());
  size_t iCurrentIndex = 0;
  for (auto var = m_clConnections.begin();
       var != m_clConnections.end();
       var++) {
    
    // copy new value only if this variable is active and if the
    // last vars contain valid data (i.e. always copy on first execution)
    if (var->device->getIsActive() || m_clConnections.size() > m_commandLooper->getCurrentVas().size())
      va.push_back(VarStrAssignment(var->name,var->getValue()));
    else
      va.push_back(VarStrAssignment(var->name,m_commandLooper->getCurrentVas()[iCurrentIndex].value));
    
    ++iCurrentIndex;
  }

  if (m_remoteListener->getRemoteCount() > 0) {
    static uint32_t updateCounter = m_config->getRemoteUpdateSkip();
    if (updateCounter >= m_config->getRemoteUpdateSkip()) {
      m_remoteListener->update(m_commandLooper->getCurrentStrVas());
      updateCounter = 0;
    } else {
      updateCounter++;
    }
  }

  m_pcState = 7;
  
  try {
    VarStrAssignments vaNew = m_commandLooper->execute(va);
    m_pcState = 8;
    applyCommands(vaNew);
  } catch (const ExecuteException& e) {
    m_pcState = 9;
    IVDA_ERROR("Error during script execution: " << e.what());
  }
  
  m_pcState = 10;

}

ClConnection::ClConnection(const std::string& _name, HASMember* _device,
                           uint32_t _channel) :
  deviceType(BusDevice::other),
  name(_name),
  device(_device),
  channel(_channel)
{
  IDigitalIn* dIn = dynamic_cast<IDigitalIn*>(device);
  IDigitalOut* dOut = dynamic_cast<IDigitalOut*>(device);

  if (dIn != NULL || dOut != NULL) {
    deviceType = BusDevice::Digital;
    bInput= dIn != NULL;
    bOutput= dOut != NULL;
  } else {
    IAnalogIn* aIn = dynamic_cast<IAnalogIn*>(device);
    IAnalogOut* aOut = dynamic_cast<IAnalogOut*>(device);

    deviceType = BusDevice::Analog;
    bInput= aIn != NULL;
    bOutput= aOut != NULL;
  }
}


double Has::getBaseVersion() {
  return HAS_VERSION;
}

uint32_t Has::getRevisionVersion() {
#ifdef SVN_REVISION
  return SVN_REVISION;
#endif
  return 0;
}

std::string Has::getVersion() {
  std::stringstream ss;
  ss << getBaseVersion() << "." << getRevisionVersion();
  return ss.str();
}

std::string Has::getLogo() {
  std::stringstream ss;
  ss << "X    X      X      XXXXX   XXX\n";
  ss << "X    X     XXX    X     X   X\n";
  ss << "X    X    X   X   X         X\n";
  ss << "XXXXXX   X     X   XXXXX    X       VERSION: " << Has::getVersion() << "\n";
  ss << "X    X   XXXXXXX        X   X\n";
  ss << "X    X   X     X  X     X   X\n";
  ss << "X    X   X     X   XXXXX   XXX";
  return ss.str();
}


uint32_t ClConnection::getInChannelCount() const {
  if (!bInput) {
    IVDA_WARNING("Called getInChannelCount on a connection that is no input " << name);
    return 0;
  }

  switch (deviceType) {
    case BusDevice::Digital: {
      IDigitalIn* iptr = dynamic_cast<IDigitalIn*>(device);
      return iptr->getDigitalInChannelCount();
    }
    case BusDevice::Analog: {
      IAnalogIn* aptr = dynamic_cast<IAnalogIn*>(device);
      return aptr->getAnalogInChannelCount();
    }
    default:
      IVDA_WARNING("Called getInChannelCount on a connection that is no valid input " << name);
      return 0;
  }
}


uint32_t ClConnection::getOutChannelCount() const {
  if (!bOutput) {
    IVDA_WARNING("Called getOutChannelCount on a connection that is no output " << name);
    return 0;
  }

  switch (deviceType) {
    case BusDevice::Digital:{
      IDigitalOut* iptr = dynamic_cast<IDigitalOut*>(device);
      return iptr->getDigitalOutChannelCount();
    }
    case BusDevice::Analog: {
      IAnalogOut* aptr = dynamic_cast<IAnalogOut*>(device);
      return aptr->getAnalogOutChannelCount();
    }
    default:
      IVDA_WARNING("Called getOutChannelCount on a connection that is no valid output " << name);
      return 0;
  }
}


void ClConnection::applyValue() {
  if (!bOutput) {
    IVDA_WARNING("Called applyValue on a connection that is not an output " << name);
    return;
  }

  switch (deviceType) {
    case BusDevice::Digital: {
      IDigitalOut* iptr = dynamic_cast<IDigitalOut*>(device);
      if (iptr->digitalOutNeedsUpdate()) {
        iptr->applyDigitalOut();
      }
      break;
    }
    case BusDevice::Analog: {
      IAnalogOut* aptr = dynamic_cast<IAnalogOut*>(device);
      if (aptr->analogOutNeedsUpdate()) {
        aptr->applyAnalogOut();
      }
      break;
    }
    default:
      IVDA_WARNING("Called applyValue on a connection that is not a valid output " << name);
      break;
  }
}


void ClConnection::setValue(double v) {
  if (!bOutput) {
    IVDA_WARNING("Called setValue on a connection that is no output " << name);
    return;
  }
  
  try {
    switch (deviceType) {
      case BusDevice::Digital: {
        IDigitalOut* iptr = dynamic_cast<IDigitalOut*>(device);
        iptr->setDigital(channel, v != 0.0 ? BitManip::ON : BitManip::OFF);
        //      iptr->applyDigitalOut();
        break;
      }
      case BusDevice::Analog: {
        IAnalogOut* aptr = dynamic_cast<IAnalogOut*>(device);
        aptr->setAnalog(channel, float(v));
        //      aptr->applyAnalogOut();
        break;
      }
      default:
        IVDA_WARNING("Called setValue on a connection that is no valid output " << name);
        break;
    }
  } catch (const std::out_of_range& e) {
    IVDA_ERROR("Error in setValue call: " << e.what());
  }
}

double ClConnection::getValue() {
  if (!bInput) {
    IVDA_WARNING("Called getValue on a connection that is no input " << name);
    return 0.0;
  }
  
  switch (deviceType) {
    case BusDevice::Digital: {
      IDigitalIn* iptr = dynamic_cast<IDigitalIn*>(device);
      return double(iptr->getDigital(channel));
    }
    case BusDevice::Analog: {
      IAnalogIn* aptr = dynamic_cast<IAnalogIn*>(device);
      return aptr->getAnalog(channel);
    }
    default:
      IVDA_WARNING("Called getValue on a connection that is no valid input " << name);
      return 0.0;
  }
}


BusDevice::BusDevice(HASMember* _device) :
  device(_device)
{
  IDigitalIn* dIn = dynamic_cast<IDigitalIn*>(device);

  if (dIn !=NULL) {
    deviceType = Digital;
  } else {
    deviceType = Analog;
  }
}

uint32_t BusDevice::poll() {
  uint32_t delayms = 0;
  switch (deviceType) {
    case Digital: {
      IDigitalIn* iptr = dynamic_cast<IDigitalIn*>(device);
      iptr->pollDigitalIn();
      break;
    }
    case Analog: {
      IAnalogIn* aptr = dynamic_cast<IAnalogIn*>(device);
      delayms = aptr->pollAnalogIn();
      break;
    }
    default:
      IVDA_WARNING("Called poll on a device that is not valid " << device->getName());
      break;
  }

  return delayms;
}

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

