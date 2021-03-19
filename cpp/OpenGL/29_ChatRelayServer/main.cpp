#include "ChatRelayServer.h"

#include <future>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

std::chrono::time_point<Clock> lastPingTime = Clock::now();

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

static std::string getAnswer()
{
    std::string answer;
    std::cin >> answer;
    return answer;
}

int main(int argc, char ** argv) {
  std::set_terminate (globalExceptionHandler);

  ChatRelayServer server;
  server.start();
  
  std::cout << "Starting ChatTris Server...";
  while (server.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << std::endl;
  if (server.isOK()) {
    std::cout << "running ...\n" << std::flush;

    std::string answer;
    std::future<std::string> future = std::async(getAnswer);
    do {
            
      std::chrono::milliseconds timeout(10);
      if (future.wait_for(timeout) == std::future_status::ready) {
        answer = future.get();
        if (answer != "q") future = std::async(getAnswer);
      }
      
      
      auto currentTime = Clock::now();
      if (std::chrono::duration_cast<std::chrono::seconds>(currentTime-lastPingTime).count() > 60) {
        lastPingTime = currentTime;
        server.sendKeepAlivePing();
      }
      
    } while (answer != "q");
    std::cout << "Shutting down server ..." << std::endl;
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
