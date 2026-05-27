// Copyright 2026 serjimen vja-nie dlesieur
#include "network/EventLoop.hpp"

#include <sys/socket.h>

/**
 * @brief Handles readiness to write data to a client socket.
 *
 * Flushes available data from the client's write buffer via send().
 * Successfully sent bytes are consumed from the buffer.
 * Negative returns are ignored without consulting errno.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handleClientWrite(int fd) {
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

  // Check if we should close the connection after sending all data
  if (it->second->get_write_buffer().empty() &&
      it->second->get_should_close()) {
    removeSocket(fd);
  }
}
