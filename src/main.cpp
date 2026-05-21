// Copyright 2026 serjimen vja-nie dlesieur
#include <iostream>

#include "network/EventLoop.hpp"
#include "network/ListeningSocket.hpp"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  try {
    std::cout << "--- Webserv Basic Initialization ---" << std::endl;
    
    ListeningSocket server_socket;
    server_socket.init(8080);
    std::cout << "Server listening on port 8080..." << std::endl;

    EventLoop loop;
    loop.addServerSocket(server_socket.get_fd());
    
    std::cout << "Webserv started successfully. (EventLoop active)" << std::endl;
    loop.run();

  } catch (const std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
