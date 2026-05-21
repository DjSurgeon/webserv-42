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
EventLoop::EventLoop(const EventLoop& other)
    : _pollfds(other._pollfds), _server_fds(other._server_fds) {}

/**
 * @brief Copy assignment operator for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 * @return EventLoop& Reference to the updated object.
 */
EventLoop& EventLoop::operator=(const EventLoop& other) {
  if (this != &other) {
    _pollfds = other._pollfds;
    _server_fds = other._server_fds;
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
 * Also registers the descriptor in the internal server list.
 *
 * @param fd The file descriptor of the server socket.
 */
void EventLoop::addServerSocket(int fd) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
  _server_fds.push_back(fd);
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
 * Also removes it from the server descriptors list if it was a server.
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

      // Remove from server list if present
      for (std::vector<int>::iterator sit = _server_fds.begin();
           sit != _server_fds.end(); ++sit) {
        if (*sit == fd) {
          _server_fds.erase(sit);
          break;
        }
      }
      return;
    }
  }

  throw std::runtime_error("EventLoop: Attempted to remove non-existent fd");
}

/**
 * @brief Checks if a given file descriptor belongs to a server socket.
 *
 * @param fd The file descriptor to check.
 * @return true If the file descriptor is in the server list.
 * @return false If the file descriptor is not in the server list.
 */
bool EventLoop::_isServerSocket(int fd) const {
  for (std::vector<int>::const_iterator it = _server_fds.begin();
       it != _server_fds.end(); ++it) {
    if (*it == fd) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Handles an incoming connection on a server socket.
 *
 * Stub for accepting a new client connection.
 *
 * @param server_fd The file descriptor of the server socket.
 */
void EventLoop::_handle_new_connection(int server_fd) {
  std::cout << "EventLoop: [STUB] New connection on server FD " << server_fd
            << std::endl;
}

/**
 * @brief Handles incoming data from a client socket.
 *
 * Stub for reading data from a client.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_data(int fd) {
  std::cout << "EventLoop: [STUB] Data available on client FD " << fd
            << std::endl;
}

/**
 * @brief Handles readiness to write data to a client socket.
 *
 * Stub for writing data to a client.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_write(int fd) {
  std::cout << "EventLoop: [STUB] Ready to write on client FD " << fd
            << std::endl;
}

/**
 * @brief Main infinite loop for polling events.
 *
 * Queries the Unix kernel about socket activity using poll() and dispatches
 * traffic to the corresponding private handlers based on the revents bitmasks.
 */
void EventLoop::run() {
  while (true) {
    if (_pollfds.empty()) {
      // Optional: Handle empty state if no servers or clients are registered
      continue;
    }

    int ret = poll(&_pollfds[0], _pollfds.size(), POLL_TIMEOUT);

    if (ret < 0) {
      std::cerr << "EventLoop: poll() failed" << std::endl;
      // Depending on robust implementation, could check for EINTR here.
      continue;
    }

    if (ret == 0) {
      // Timeout occurred, loop again
      continue;
    }

    // Iterate backwards to allow safe removal of sockets during iteration
    for (int i = static_cast<int>(_pollfds.size()) - 1; i >= 0; --i) {
      int current_fd = _pollfds[i].fd;
      short revents = _pollfds[i].revents;

      if (revents == 0) {
        continue;
      }

      // Check for errors or disconnection
      if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        std::cerr << "EventLoop: Error or hangup on FD " << current_fd
                  << std::endl;
        removeSocket(current_fd);
        continue;
      }

      // Check for incoming data (or new connection)
      if (revents & POLLIN) {
        if (_isServerSocket(current_fd)) {
          _handle_new_connection(current_fd);
        } else {
          _handle_client_data(current_fd);
        }
      }

      // Check for readiness to write (only applies to clients)
      // Enclosed in try-catch or safe checks if the fd was removed in POLLIN,
      // but in this stub we just execute the handler.
      if (revents & POLLOUT) {
        if (!_isServerSocket(current_fd)) {
          _handle_client_write(current_fd);
        }
      }
    }
  }
}
