#include <iostream>
#include "network/ListeningSocket.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        std::cout << "Initializing ListeningSocket..." << std::endl;
        ListeningSocket server_socket;
        std::cout << "Socket created successfully with FD: " << server_socket.get_fd() << std::endl;
        std::cout << "Exiting scope... Destructor should close the FD automatically." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
