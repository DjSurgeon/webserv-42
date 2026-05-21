// Copyright 2026 serjimen vja-nie dlesieur
#include "network/EventLoop.hpp"

#include <sys/socket.h>
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
    : _pollfds(other._pollfds), _server_fds(other._server_fds) {
  // Clients are NOT deep copied because they manage unique file descriptors
  // and copying them violates the RAII strict ownership rules (double-free).
}

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
    // Clients are NOT deep copied for the same RAII reasons.
  }
  return *this;
}

/**
 * @brief Destructor for EventLoop.
 *
 * Cleans up all client sockets allocated in memory to prevent leaks.
 */
EventLoop::~EventLoop() {
  for (std::map<int, ClientSocket*>::iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    delete it->second;
  }
  _clients.clear();
}

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
 * Also removes it from the server descriptors list if it was a server,
 * or deletes the associated ClientSocket object if it was a client.
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
          return;
        }
      }

      // If not a server, it's a client. Remove from clients map and delete
      std::map<int, ClientSocket*>::iterator cit = _clients.find(fd);
      if (cit != _clients.end()) {
        delete cit->second;
        _clients.erase(cit);
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
 * Calls accept() to receive the connection, instantiates a new ClientSocket
 * (which handles O_NONBLOCK automatically), stores it in the clients map,
 * and registers it for monitoring.
 *
 * @param server_fd The file descriptor of the server socket.
 */
void EventLoop::_handle_new_connection(int server_fd) {
  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd >= 0) {
    try {
      ClientSocket* new_client = new ClientSocket(client_fd);
      _clients[client_fd] = new_client;
      addClientSocket(client_fd);
    } catch (const std::exception& e) {
      std::cerr << "EventLoop: Failed to accept client: " << e.what() << "\n";
      close(client_fd);
    }
  }
}

/**
 * @brief Handles incoming data from a client socket.
 *
 * Executes recv() safely. If bytes are received, appends them to the client's
 * read buffer. If recv returns exactly 0 (EOF), disconnects the client.
 * Negative returns are ignored without consulting errno.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_data(int fd) {
  std::map<int, ClientSocket*>::iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return;
  }

  char buffer[8192];
  int bytes = recv(fd, buffer, sizeof(buffer), 0);

  if (bytes > 0) {
    it->second->append_to_read_buffer(std::string(buffer, bytes));
  } else if (bytes == 0) {
    removeSocket(fd);
  }
}

/**
 * @brief Handles readiness to write data to a client socket.
 *
 * Flushes available data from the client's write buffer via send().
 * Successfully sent bytes are consumed from the buffer.
 * Negative returns are ignored without consulting errno.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_write(int fd) {
  std::map<int, ClientSocket*>::iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return;
  }

  const std::string& data = it->second->get_write_buffer();
  if (!data.empty()) {
    int sent = send(fd, data.c_str(), data.length(), 0);
    if (sent > 0) {
      it->second->consume_write_buffer(sent);
    }
  }
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
      continue;
    }

    int ret = poll(&_pollfds[0], _pollfds.size(), POLL_TIMEOUT);

    if (ret < 0) {
      std::cerr << "EventLoop: poll() failed" << std::endl;
      continue;
    }

    if (ret == 0) {
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

      // Prevent processing POLLOUT if the socket was closed during POLLIN
      if (!_isServerSocket(current_fd) &&
          _clients.find(current_fd) == _clients.end()) {
        continue;
      }

      // Check for readiness to write
      if (revents & POLLOUT) {
        if (!_isServerSocket(current_fd)) {
          _handle_client_write(current_fd);
        }
      }
    }
  }
}
