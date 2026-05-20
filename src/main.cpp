// Copyright 2026 serjimen vja-nie dlesieur
#include <iostream>

#include "network/ListeningSocket.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        std::cout << "--- Webserv Basic Initialization ---" << std::endl;
        ListeningSocket server_socket;
        server_socket.init(8080);
        std::cout << "Server listening on port 8080..." << std::endl;

        // Next phase: EventLoop implementation
        std::cout << "Webserv started successfully. (Basic mode)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
