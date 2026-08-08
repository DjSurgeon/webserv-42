// Copyright 2026 raperez- serjimen
#include <sys/socket.h>

#include <map>
#include <string>

#include "network/EventLoop.hpp"

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

  size_t len = it->second->getWriteLength();
  if (len > 0) {
    int sent = send(fd, it->second->getWriteData(), len, 0);
    if (sent > 0) {
      it->second->consumeWriteBuffer(sent);
    }
  }

  // Check if we should close the connection after sending all data
  if (it->second->getWriteBuffer().empty() && it->second->getShouldClose()) {
    removeSocket(fd);
  }
}
