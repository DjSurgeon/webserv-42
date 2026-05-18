#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "network/ListeningSocket.hpp"
#include "network/ClientSocket.hpp"

void test_client_socket() {
    std::cout << "\n--- Testing ClientSocket ---" << std::endl;
    
    // Create a dummy socket to simulate an accepted connection
    int dummy_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dummy_fd == -1) {
        throw std::runtime_error("Test: socket() failed");
    }

    try {
        std::cout << "Wrapping dummy FD " << dummy_fd << " in ClientSocket..." << std::endl;
        ClientSocket client(dummy_fd);
        std::cout << "ClientSocket created successfully." << std::endl;

        // Verify O_NONBLOCK flag
        int flags = fcntl(client.get_fd(), F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error("Test: fcntl(F_GETFL) failed");
        }

        if (flags & O_NONBLOCK) {
            std::cout << "✅ SUCCESS: O_NONBLOCK flag is active on ClientSocket FD." << std::endl;
        } else {
            std::cout << "❌ FAILURE: O_NONBLOCK flag is NOT active on ClientSocket FD." << std::endl;
        }
    } catch (const std::exception& e) {
        close(dummy_fd); // Cleanup if ClientSocket constructor fails
        throw;
    }
}

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

        test_client_socket();

        std::cout << "\nAll initialization tests passed." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
