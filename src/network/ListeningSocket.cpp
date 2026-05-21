// Copyright 2026 serjimen vja-nie dlesieur
#include "network/ListeningSocket.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

/**
 * @brief Default constructor for ListeningSocket.
 *
 * Sets the file descriptor to -1 to indicate that the socket
 * is in a passive, uninitialized state.
 */
ListeningSocket::ListeningSocket() : _fd(-1) {}

/**
 * @brief Explicit constructor for ListeningSocket.
 *
 * Immediately creates and configures the socket to listen on a specific port.
 *
 * @param port Network port on which the server will listen for connections.
 * @throw std::runtime_error If any error occurs during
 * socket creation or configuration.
 */
ListeningSocket::ListeningSocket(int port) : _fd(-1) {
  init(port);
}

/**
 * @brief Initializes and activates the listening socket.
 *
 * Creates the socket descriptor, applies the address reuse option,
 * binds it to the specified port, and sets it to passive listening mode.
 * If any system call fails, it closes the created socket and resets
 * the descriptor to -1 to ensure exception safety.
 *
 * @param port Network port on which the socket will listen.
 * @throw std::runtime_error If the socket is already
 * initialized or if any system call fails.
 */
void ListeningSocket::init(int port) {
  if (_fd != -1) {
    throw std::runtime_error("ListeningSocket: Already initialized");
  }

  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    throw std::runtime_error("ListeningSocket: socket() failed");
  }

  int opt = 1;
  if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    close(_fd);
    _fd = -1;
    throw std::runtime_error("ListeningSocket: setsockopt() failed");
  }

  struct sockaddr_in address;
  std::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
    close(_fd);
    _fd = -1;
    throw std::runtime_error("ListeningSocket: bind() failed");
  }

  if (listen(_fd, 128) == -1) {
    close(_fd);
    _fd = -1;
    throw std::runtime_error("ListeningSocket: listen() failed");
  }
}

/**
 * @brief Destructor for ListeningSocket.
 *
 * Safely closes the socket descriptor if it is active,
 * preventing file descriptor leaks in the operating system.
 */
ListeningSocket::~ListeningSocket() {
  if (_fd != -1) {
    if (close(_fd) == -1) {
      std::cerr << "Error: Closing socket FD " << _fd << " failed" << std::endl;
    }
  }
}

/**
 * @brief Gets the socket file descriptor.
 *
 * @return int The socket file descriptor.
 */
int ListeningSocket::get_fd() const {
  return _fd;
}
