// Copyright 2026 raperez- serjimen
#include <csignal>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "network/EventLoop.hpp"
#include "network/ListeningSocket.hpp"
#include "utils/FileUtils.hpp"

volatile sig_atomic_t g_running = 1;

void sigHandler(int signum) {
  (void)signum;
  g_running = 0;
}

int main(int argc, char** argv) {
  signal(SIGINT, sigHandler);
  signal(SIGTERM, sigHandler);

  if (argc > 2) {
    std::cerr << "Usage: " << argv[0] << " [configuration_file]" << std::endl;
    return 1;
  }

  std::vector<ListeningSocket*> listening_sockets;

  try {
    std::string config_file;
    if (argc == 2) {
      config_file = argv[1];
    } else {
      config_file = "conf/default.conf";
    }

    if (!FileUtils::isValidExtension(config_file, ".conf")) {
      throw std::runtime_error("Invalid configuration file extension. Must end with '.conf'");
    }

    std::cout << "--- Webserv Initialization ---" << std::endl;
    std::cout << "Loading configuration from: " << config_file << std::endl;

    ConfigParser parser(config_file);
    const std::vector<ServerConfig>& servers = parser.getServers();

    if (servers.empty()) {
      throw std::runtime_error("No server blocks found in configuration.");
    }

    std::map<int, std::vector<ServerConfig> > servers_by_port;
    for (size_t i = 0; i < servers.size(); ++i) {
      servers_by_port[servers[i].getPort()].push_back(servers[i]);
    }

    EventLoop loop;

    for (std::map<int, std::vector<ServerConfig> >::iterator it =
             servers_by_port.begin();
         it != servers_by_port.end(); ++it) {
      int port = it->first;
      ListeningSocket* socket = new ListeningSocket();
      socket->init(port);
      loop.addServerSocket(socket->getFd(), it->second);
      listening_sockets.push_back(socket);
      std::cout << "Server listening on port " << port << " ("
                << it->second.size() << " virtual hosts)" << std::endl;
    }

    std::cout << "Webserv started successfully. (EventLoop active)"
              << std::endl;
    loop.run();

    for (size_t i = 0; i < listening_sockets.size(); ++i) {
      delete listening_sockets[i];
    }
  } catch (const std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    for (size_t i = 0; i < listening_sockets.size(); ++i) {
      delete listening_sockets[i];
    }
    return 1;
  }

  return 0;
}
