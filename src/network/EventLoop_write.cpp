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

  ClientSocket* client = it->second;
  size_t len = client->getWriteLength();
  
  if (len > 0) {
    ssize_t sent = send(fd, client->getWriteData(), len, 0);
    
    if (sent > 0) {
      client->consumeWriteBuffer(static_cast<size_t>(sent));
    }
  }

  if (client->getWriteBuffer().empty() && client->getShouldClose()) {
    removeSocket(fd);
  }
}
