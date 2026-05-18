#include <iostream>
#include <unistd.h>
#include "network/ListeningSocket.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        std::cout << "Initializing ListeningSocket..." << std::endl;
        ListeningSocket server_socket;
        std::cout << "Socket created successfully with FD: " << server_socket.get_fd() << std::endl;

        std::cout << "Binding to port 8080..." << std::endl;
        server_socket.init(8080);
        std::cout << "Successfully bound to port 8080." << std::endl;

        std::cout << "Press Ctrl+C to exit and test SO_REUSEADDR (immediate restart)." << std::endl;
        while (true) {
            sleep(1);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
