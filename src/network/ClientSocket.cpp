// Copyright 2026 raperez- serjimen
#include "network/ClientSocket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

/**
 * @brief Construct a new Client Socket object.
 *
 * Validates the file descriptor, checks if it is open, and configures
 * it to non-blocking mode (O_NONBLOCK) immediately.
 *
 * @param client_fd Raw file descriptor from accept().
 * @throw std::runtime_error If the file descriptor is invalid or fcntl() fails.
 */
ClientSocket::ClientSocket(int client_fd)
    : _fd(client_fd), _shouldClose(false) {
  if (_fd < 0) {
    throw std::runtime_error("ClientSocket: Invalid file descriptor");
  }

  // Set to non-blocking mode immediately
  if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
    close(_fd);
    throw std::runtime_error("ClientSocket: fcntl() failed to set O_NONBLOCK");
  }
}

/**
 * @brief Destroy the Client Socket object.
 *
 * Safely closes the associated file descriptor.
 */
ClientSocket::~ClientSocket() {
  if (_fd != -1) {
    close(_fd);
  }
}

/**
 * @brief Gets the socket file descriptor.
 *
 * @return int The file descriptor.
 */
int ClientSocket::getFd() const {
  return _fd;
}

/**
 * @brief Gets the read buffer as a read-only constant reference.
 *
 * @return const std::string& The constant reference to the read buffer.
 */
const std::string& ClientSocket::getReadBuffer() const {
  return _readBuffer;
}

/**
 * @brief Gets the write buffer as a read-only constant reference.
 *
 * @return const std::string& The constant reference to the write buffer.
 */
const std::string& ClientSocket::getWriteBuffer() const {
  return _writeBuffer;
}

/**
 * @brief Appends new raw data received from the network to the read buffer.
 *
 * @param data The data string to append.
 */
void ClientSocket::appendToReadBuffer(const std::string& data) {
  _readBuffer.append(data);
}

/**
 * @brief Appends response data to the write buffer to be sent later.
 *
 * @param data The data string to append.
 */
void ClientSocket::appendToWriteBuffer(const std::string& data) {
  _writeBuffer.append(data);
}

/**
 * @brief Consumes a specified number of bytes
 * from the beginning of the read buffer.
 *
 * Removes processed bytes from the buffer. If bytes to consume is larger than
 * the buffer size, the buffer is cleared to avoid undefined behavior.
 *
 * @param bytes Number of bytes to consume.
 */
void ClientSocket::consumeReadBuffer(size_t bytes) {
  if (bytes >= _readBuffer.size()) {
    _readBuffer.clear();
  } else {
    _readBuffer.erase(0, bytes);
  }
}

/**
 * @brief Clears the write buffer completely.
 */
void ClientSocket::clearWriteBuffer() {
  _writeBuffer.clear();
}

/**
 * @brief Consumes a specified number of bytes from the write buffer.
 *
 * Removes processed bytes from the buffer after they have been sent.
 *
 * @param bytes Number of bytes to consume.
 */
void ClientSocket::consumeWriteBuffer(size_t bytes) {
  if (bytes >= _writeBuffer.size()) {
    _writeBuffer.clear();
  } else {
    _writeBuffer.erase(0, bytes);
  }
}

/**
 * @brief Sets the closure flag for the connection.
 *
 * @param close True if the connection should be closed after writing.
 */
void ClientSocket::setShouldClose(bool close) {
  _shouldClose = close;
}

/**
 * @brief Gets the closure flag for the connection.
 *
 * @return true If the connection is marked for closure.
 */
bool ClientSocket::getShouldClose() const {
  return _shouldClose;
}
