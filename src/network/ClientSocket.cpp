#include "network/ClientSocket.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

ClientSocket::ClientSocket(int client_fd) : _fd(client_fd) {
    if (_fd < 0) {
        throw std::runtime_error("ClientSocket: Invalid file descriptor");
    }

    // Set to non-blocking mode immediately
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("ClientSocket: fcntl() failed to set O_NONBLOCK");
    }
}

ClientSocket::~ClientSocket() {
    if (_fd != -1) {
        close(_fd);
    }
}

int ClientSocket::get_fd() const {
    return _fd;
}

std::string& ClientSocket::get_read_buffer() {
    return _read_buffer;
}

std::string& ClientSocket::get_write_buffer() {
    return _write_buffer;
}
