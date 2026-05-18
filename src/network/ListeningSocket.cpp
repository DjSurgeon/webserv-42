#include "network/ListeningSocket.hpp"
#include <iostream>
#include <netinet/in.h>
#include <cstring>

ListeningSocket::ListeningSocket() {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        throw std::runtime_error("ListeningSocket: socket() failed");
    }
}

void ListeningSocket::init(int port) {
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("ListeningSocket: setsockopt() failed");
    }

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        throw std::runtime_error("ListeningSocket: bind() failed");
    }

    if (listen(_fd, 128) == -1) {
        throw std::runtime_error("ListeningSocket: listen() failed");
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
