#include "network/ListeningSocket.hpp"
#include <iostream>

ListeningSocket::ListeningSocket() {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        throw std::runtime_error("ListeningSocket: socket() failed");
    }
}

ListeningSocket::~ListeningSocket() {
    if (_fd != -1) {
        if (close(_fd) == -1) {
            std::cerr << "Error: Closing socket FD " << _fd << " failed" << std::endl;
        }
    }
}

int ListeningSocket::get_fd() const {
    return _fd;
}
