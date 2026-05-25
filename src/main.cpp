// Copyright 2026 serjimen vja-nie dlesieur
#include <csignal>
#include <iostream>
#include "network/EventLoop.hpp"
#include "network/ListeningSocket.hpp"
#include "config/ServerConfig.hpp"

volatile sig_atomic_t g_running = 1;

void sig_handler(int signum) {
  (void)signum;
  g_running = 0;
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  try {
    std::cout << "--- Webserv Basic Initialization ---" << std::endl;
    ListeningSocket server_socket;
    server_socket.init(8080);
    std::cout << "Server listening on port 8080..." << std::endl;
    EventLoop loop;
    
    ServerConfig dummy_server;
    dummy_server.set_root("/var/www/html");
    dummy_server.add_server_name("localhost");
    std::vector<ServerConfig> configs;
    configs.push_back(dummy_server);
    
    loop.addServerSocket(server_socket.get_fd(), configs);
    std::cout << "Webserv started successfully. (EventLoop active)"
              << std::endl;
    loop.run();
  } catch (const std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
