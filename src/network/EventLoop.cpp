// Copyright 2026 serjimen vja-nie dlesieur
#include "network/EventLoop.hpp"

#include <unistd.h>

#include <iostream>
#include <stdexcept>

/**
 * @brief Default constructor for EventLoop.
 */
EventLoop::EventLoop() {}

/**
 * @brief Copy constructor for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 */
EventLoop::EventLoop(const EventLoop& other) : _pollfds(other._pollfds) {}

/**
 * @brief Copy assignment operator for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 * @return EventLoop& Reference to the updated object.
 */
EventLoop& EventLoop::operator=(const EventLoop& other) {
  if (this != &other) {
    _pollfds = other._pollfds;
  }
  return *this;
}

/**
 * @brief Destructor for EventLoop.
 */
EventLoop::~EventLoop() {}

/**
 * @brief Adds a server socket to the monitoring vector.
 *
 * Instantiates a pollfd structure for the given file descriptor, configures
 * it to monitor incoming connections (POLLIN), and appends it to the vector.
 *
 * @param fd The file descriptor of the server socket.
 */
void EventLoop::addServerSocket(int fd) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
}

/**
 * @brief Adds a client socket to the monitoring vector.
 *
 * Instantiates a pollfd structure for the given file descriptor, configures
 * it to simultaneously monitor for incoming data (POLLIN) and ability to
 * write data (POLLOUT), and appends it to the vector.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::addClientSocket(int fd) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN | POLLOUT;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
}

/**
 * @brief Removes a socket from the monitoring vector and safely closes it.
 *
 * Finds the descriptor within the _pollfds vector, executes close(fd) to
 * prevent resource leaks, and erases the element using std::vector::erase.
 *
 * @param fd The file descriptor to remove.
 * @throw std::runtime_error If the file descriptor is not found in the loop.
 */
void EventLoop::removeSocket(int fd) {
  for (std::vector<pollfd>::iterator it = _pollfds.begin();
       it != _pollfds.end(); ++it) {
    if (it->fd == fd) {
      if (close(fd) == -1) {
        std::cerr << "EventLoop: Error closing socket FD " << fd << std::endl;
      }
      _pollfds.erase(it);
      return;
    }
  }

  throw std::runtime_error("EventLoop: Attempted to remove non-existent fd");
}
