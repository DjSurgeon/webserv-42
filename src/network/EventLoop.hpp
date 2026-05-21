// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_NETWORK_EVENTLOOP_HPP_
#define SRC_NETWORK_EVENTLOOP_HPP_

#include <sys/poll.h>

#include <vector>

class EventLoop {
 public:
  EventLoop();
  EventLoop(const EventLoop& other);
  EventLoop& operator=(const EventLoop& other);
  ~EventLoop();

  void addServerSocket(int fd);
  void addClientSocket(int fd);
  void removeSocket(int fd);
  void run();

 private:
  std::vector<pollfd> _pollfds;
  std::vector<int> _server_fds;

  static const int POLL_TIMEOUT = 1000;

  bool _isServerSocket(int fd) const;
  void _handle_new_connection(int server_fd);
  void _handle_client_data(int fd);
  void _handle_client_write(int fd);
};

#endif  // SRC_NETWORK_EVENTLOOP_HPP_
