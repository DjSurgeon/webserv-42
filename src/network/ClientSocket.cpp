// Copyright 2026 serjimen vja-nie dlesieur
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
ClientSocket::ClientSocket(int client_fd) : _fd(client_fd) {
    if (_fd < 0) {
        throw std::runtime_error("ClientSocket: Invalid file descriptor");
    }

    // Set to non-blocking mode immediately
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
        close(_fd);
        throw std::runtime_error(
            "ClientSocket: fcntl() failed to set O_NONBLOCK");
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
int ClientSocket::get_fd() const {
    return _fd;
}

/**
 * @brief Gets the read buffer as a read-only constant reference.
 * 
 * @return const std::string& The constant reference to the read buffer.
 */
const std::string& ClientSocket::get_read_buffer() const {
    return _read_buffer;
}

/**
 * @brief Gets the write buffer as a read-only constant reference.
 * 
 * @return const std::string& The constant reference to the write buffer.
 */
const std::string& ClientSocket::get_write_buffer() const {
    return _write_buffer;
}

/**
 * @brief Appends new raw data received from the network to the read buffer.
 * 
 * @param data The data string to append.
 */
void ClientSocket::append_to_read_buffer(const std::string& data) {
    _read_buffer.append(data);
}

/**
 * @brief Appends response data to the write buffer to be sent later.
 * 
 * @param data The data string to append.
 */
void ClientSocket::append_to_write_buffer(const std::string& data) {
    _write_buffer.append(data);
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
void ClientSocket::consume_read_buffer(size_t bytes) {
    if (bytes >= _read_buffer.size()) {
        _read_buffer.clear();
    } else {
        _read_buffer.erase(0, bytes);
    }
}

/**
 * @brief Clears the write buffer completely.
 */
void ClientSocket::clear_write_buffer() {
    _write_buffer.clear();
}
