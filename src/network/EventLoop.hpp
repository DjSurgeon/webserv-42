// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_NETWORK_EVENTLOOP_HPP_
#define SRC_NETWORK_EVENTLOOP_HPP_

#include <sys/poll.h>

#include <csignal>
#include <ctime>
#include <map>
#include <vector>

#include "config/ServerConfig.hpp"
#include "http/RequestParser.hpp"
#include "network/ClientSocket.hpp"

extern volatile sig_atomic_t g_running;

class EventLoop {
 public:
  EventLoop();
  EventLoop(const EventLoop& other);
  EventLoop& operator=(const EventLoop& other);
  ~EventLoop();

  void addServerSocket(int fd, const std::vector<ServerConfig>& configs);
  void addClientSocket(int fd);
  void removeSocket(int fd);
  void set_session_cleanup_interval(time_t interval);
  void run();

 private:
  std::vector<pollfd> _pollfds;
  std::vector<int> _server_fds;
  std::map<int, ClientSocket*> _clients;
  std::map<int, RequestParser*> _parsers;
  std::map<int, std::vector<ServerConfig> > _server_configs;
  std::map<int, int> _client_to_server;

  static const int POLL_TIMEOUT = 1000;

  time_t _last_session_cleanup;
  time_t _session_cleanup_interval;

  bool _isServerSocket(int fd) const;
  void _handle_new_connection(int server_fd);
  void _handle_client_data(int fd);
  void _handle_client_write(int fd);
};

#endif  // SRC_NETWORK_EVENTLOOP_HPP_
