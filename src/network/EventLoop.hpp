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
};

#endif  // SRC_NETWORK_EVENTLOOP_HPP_
