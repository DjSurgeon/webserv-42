// Copyright 2026 raperez- serjimen
#include "network/EventLoop.hpp"

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "handlers/CgiHandler.hpp"
#include "handlers/FileHandler.hpp"
#include "handlers/StaticRouter.hpp"
#include "http/SessionManager.hpp"

/**
 * @brief Default constructor for EventLoop.
 */
EventLoop::EventLoop()
    : _lastSessionCleanup(std::time(NULL)), _sessionCleanupInterval(5) {}

/**
 * @brief Copy constructor for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 */
EventLoop::EventLoop(const EventLoop& other)
    : _pollfds(other._pollfds),
      _serverFds(other._serverFds),
      _lastSessionCleanup(other._lastSessionCleanup),
      _sessionCleanupInterval(other._sessionCleanupInterval) {
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
    _serverFds = other._serverFds;
    _lastSessionCleanup = other._lastSessionCleanup;
    _sessionCleanupInterval = other._sessionCleanupInterval;
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
  for (std::map<int, RequestParser*>::iterator it = _parsers.begin();
       it != _parsers.end(); ++it) {
    delete it->second;
  }
  _parsers.clear();
}

void EventLoop::setSessionCleanupInterval(time_t interval) {
  _sessionCleanupInterval = interval;
}

/**
 * @brief Main infinite loop for polling events.
 *
 * Queries the Unix kernel about socket activity using poll() and dispatches
 * traffic to the corresponding private handlers based on the revents bitmasks.
 */
void EventLoop::run() {
  while (g_running) {
    time_t now = std::time(NULL);
    if (now - _lastSessionCleanup >= _sessionCleanupInterval) {
      SessionManager::get_instance().clear_expired_sessions();
      _lastSessionCleanup = now;
    }

    if (_pollfds.empty()) continue;

    int ret = poll(&_pollfds[0], _pollfds.size(), POLL_TIMEOUT);

    // Dont show error message when failed due to a signal
    if (!g_running) break;

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
      int16_t revents = _pollfds[i].revents;

      if (revents == 0) continue;

      if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        if (_cgiOutMap.count(current_fd)) {
          _handleCgiRead(current_fd);  // Forzar lectura final
        } else {
          removeSocket(current_fd);
        }
        continue;
      }

      // LECTURA (POLLIN)
      if (revents & POLLIN) {
        if (_isServerSocket(current_fd)) {
          _handleNewConnection(current_fd);
        } else if (_cgiOutMap.count(current_fd)) {
          _handleCgiRead(current_fd);
        } else {
          _handleClientData(current_fd);
        }
      }

      if (!_isServerSocket(current_fd) &&
          _clients.find(current_fd) == _clients.end() &&
          !_cgiInMap.count(current_fd)) {
        continue;
      }

      // ESCRITURA (POLLOUT)
      if (revents & POLLOUT) {
        if (_cgiInMap.count(current_fd)) {
          _handleCgiWrite(current_fd);
        } else if (_clients.count(current_fd)) {
          _handleClientWrite(current_fd);
        }
      }
    }

    //_checkCgiTimeouts(); // Verificar scripts en bucle infinito
  }
}
