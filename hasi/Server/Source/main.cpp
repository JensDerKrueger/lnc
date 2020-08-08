#include <iostream>
#ifndef _MSC_VER
#include <execinfo.h>
#endif
#include <csignal>                          // SIGABRT, SIGTERM, etc.
#include "Has.h"
#include "LogWatcher.h"
#include <Tools/DebugOutHandler.h>
#include <Tools/DebugOut/HTMLFileOut.h>

using namespace HAS;

std::shared_ptr<Has> has = nullptr;

void sighandler(int sig)
{
  IVDA_MESSAGE("Caught termination signal " << sig << ". Initiating shutdown.");
  bool bTerminate = false;
  try {
    if (has) has->ExecuteCommand("q", bTerminate);
  } catch (const ParseException& e) {
    IVDA_ERROR("Execution Error:" << e.what());
  }
  if (!bTerminate) {
    IVDA_ERROR("Sighandler sending wrong signal to has, not terminating.");
  }
}


void printStacktrace()
{
#ifndef _MSC_VER
  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  IVDA_ERROR("Stack Trace:");
  for (i = 0; i < frames; ++i) {
    IVDA_ERROR(strs[i]);
  }
  free(strs);
#endif
}

void terminateHandler() {
  std::exception_ptr exptr = std::current_exception();
  try {
    std::rethrow_exception(exptr);
  }
  catch (const std::exception &ex) {
    IVDA_ERROR("Terminated due to exception: " << ex.what());
  }
  catch (...) {
    IVDA_ERROR("Terminated due to unknown exception.");
  }
  printStacktrace();
  std::abort();
}


bool setExecDirToBinaryDir(int argc, char** argv) {
  std::string p;
  if (argc < 3) {
    p = IVDA::SysTools::GetPath(argv[0]);
  } else {
    p = argv[1];
  }
  if (!IVDA::SysTools::Chdir(p)) {
    IVDA_ERROR("Unable to change to has directory.");
    return false;
  }
  IVDA_MESSAGE("Execution directory is: " << p.c_str());
  return true;
}

int main (int argc, char** argv){
#ifndef _MSC_VER
  // don't raise SIGPIPE when sending into broken TCP connections
  ::signal(SIGPIPE, SIG_IGN);
#endif
  
  try {
    // detect from the command line if we are supposed to
    // run as a daemon or not (-d daemonaizes)
    bool bRunAsDaemon = (argc >=2 && std::string(argv[1]) == "-d");
    
    // change execution dir to binary dir when supposed to run as daemon
    if (bRunAsDaemon) {
      if (!setExecDirToBinaryDir(argc, argv)) {
        return EXIT_FAILURE;
      }
    }
    
    // load config
    HASConfigPtr cfg = std::make_shared<HASConfig>("has.cfg");
    if (!cfg) {
      IVDA_ERROR("Unable to initialize config information, terminating.");
      return EXIT_FAILURE;
    }
    
    std::shared_ptr<LogWatcher> l=nullptr;
    l = std::make_shared<LogWatcher>(cfg->getFromAddress(), cfg->getSystemName(), cfg->getToAddress());
    
    if (cfg->getTXTLogLFilename() != "") {
      l->SetFilename(cfg->getTXTLogLFilename());
    } else {
      if (bRunAsDaemon) {
        l->SetFilename("haslog.txt");
      }
    }
    
#ifndef _MSC_VER
    // daemonize when supposed to run as daemon
    if (bRunAsDaemon) {
      
      pid_t pid;
      if ((pid = fork()) < 0) {
        // failed to fork
        IVDA_ERROR("Unable to fork daemon process, terminating.");
        return EXIT_FAILURE;
      } else {
        // forked fine, now terminate parent
        if (pid != 0) { //parent
          return EXIT_SUCCESS;
        }
      }
      
      // from now on only the client (=daemon) executes the code
      setsid();
      
      if (cfg->getHTMLLogFilename() != "" && cfg->getHTMLLogMsgCount() > 0) {
        IVDA::DebugOutHandler::Instance().AddDebugOut(new IVDA::HTMLFileOut(cfg->getHTMLLogFilename(),cfg->getHTMLLogMsgCount()));
      }
      
      if (cfg->getDaemonDelay() > 0) {
        IVDA_MESSAGE("Waiting for " << cfg->getDaemonDelay() << " ms.");
        delay(cfg->getDaemonDelay());
      }
      
    } else
#endif
    {
      IVDA::DebugOutHandler::Instance().AddDebugOut(new IVDA::ConsoleOut());
      if (cfg->getHTMLLogFilename() != "" && cfg->getHTMLLogMsgCount() > 0) {
        IVDA::DebugOutHandler::Instance().AddDebugOut(new IVDA::HTMLFileOut(cfg->getHTMLLogFilename(),cfg->getHTMLLogMsgCount()));
      }
      IVDA::DebugOutHandler::Instance().DebugOut()->SetOutput(true, true, true, true);
    }
    
    
    IVDA_MESSAGE("\n" << Has::getLogo());
    
    // create main HAS object
    has = std::make_shared<Has>(cfg);
    
    // init main HAS object
    if (!has->init()) {
      IVDA_ERROR("Has init failed, terminating.");
      return EXIT_FAILURE;
    }
    
    // register abort handlers
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);
    
    // register global exception handler
    std::set_terminate(terminateHandler);
    
    // start HAS event loop
    has->run(bRunAsDaemon);
    has = nullptr;
    
    return EXIT_SUCCESS;
    
  } catch (const std::runtime_error& e) {
    IVDA_ERROR("Uncaught exception: " << e.what());
    return EXIT_FAILURE;
  }
}

/*
 The MIT License
 
 Copyright (c) 2013-2014 Jens Krueger
 
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
