#include <iostream>
#include <future>
#include <chrono>

#include "EchoServer.h"

static std::string getAnswer() {
  std::string answer;
  std::cout << "> " << std::flush;
  std::cin >> answer;
  return answer;
}

int main(int argc, char ** argv) {
  typedef std::chrono::high_resolution_clock Clock;
  std::chrono::time_point<Clock> restorePointTime = Clock::now();

  EchoServer server(2000,800,800,{"dungeon.bmp", "mist.bmp", "paint.bmp"});
  server.start();

  std::cout << "Starting ";
  while (server.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (server.isOK()) {
    std::cout << "\nRunning" << std::endl;
    std::string answer;
    std::future<std::string> future = std::async(getAnswer);
    
    do {
      std::chrono::milliseconds timeout(10);
      if (future.wait_for(timeout) == std::future_status::ready) {
        answer = future.get();
        if (answer != "q") future = std::async(getAnswer);
      }
      
      auto currentTime = Clock::now();
      if (std::chrono::duration_cast<std::chrono::minutes>(currentTime-restorePointTime).count() > 60) {
        restorePointTime = currentTime;
        server.savePaintLayers();
      }
      
    } while (answer != "q");
    std::cout << "Shutting down server ..." << std::endl;
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
  
}

