#include <iostream>
#include <memory>
#include <vector>

#include "GGS.h"

static void globalExceptionHandler () {
  std::cerr << "terminate called after throwing an instance of ";
  try
  {
      std::rethrow_exception(std::current_exception());
  } catch (const std::exception &ex) {
      std::cerr << typeid(ex).name() << std::endl;
      std::cerr << "  what(): " << ex.what() << std::endl;
  } catch (...) {
      std::cerr << typeid(std::current_exception()).name() << std::endl;
      std::cerr << " ...something, not an exception, dunno what." << std::endl;
  }
#ifdef _WIN32
  std::cerr << "errno: " << errno << std::endl;
#else
  std::cerr << "errno: " << errno << ": " << std::strerror(errno) << std::endl;
#endif // _WIN32
  std::abort();
}

int main(int argc, char ** argv) {
  std::set_terminate (globalExceptionHandler);
    
  GGS s{serverPort};
  s.start();
  std::cout << "starting ...";
  while (s.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << std::endl;
  if (s.isOK()) {
    std::cout << "running ..." << std::endl;
    std::string user{""};
    do {
      std::cout << "(? for help) >";
      std::cin >> user;
      
      switch (user[0]) {
        case '?' :
          std::cout << "? : help\n";
          std::cout << "l : list active users\n";
          std::cout << "w : toggle logging to file\n";
          std::cout << "s : statisics\n";
          std::cout << "e : toggle message error display\n";
          std::cout << "q : quit server\n";
          break;
        case 'l' : {
          const std::vector<ClientInfo> ci = s.getClientInfo();
          for (const ClientInfo& client : ci) {
            std::cout << "ID:" << client.id << " Name:" << client.name << " Game ID:" << uint32_t(client.gameID) << " Level:" << client.level << "\n";
          }
          break;
        }
        case 's' : {
          std::cout << "Server Uptime: "<< s.uptime() << " sec.\n";
          break;
        }
        case 'w' : {
          if (s.getLogFile().empty()) {
            s.setLogFile("server.log");
            std::cout << "Logging is enabled\n";
          } else {
            s.setLogFile("");
            std::cout << "Logging is disabled\n";
          }
          break;
        }
        case 'e' : {
          s.showErrors = !s.showErrors;
          std::cout << "Error Display set to : "<< s.showErrors << "\n";
          break;
        }
      }
      std::cout << std::endl;
      
    } while (user != "q");
    
    std::cout << "Shutting down server ..." << std::endl;

    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
