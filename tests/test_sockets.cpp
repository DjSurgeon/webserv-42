#include "network/ListeningSocket.hpp"
#include "network/ClientSocket.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cassert>

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

// -----------------------------------------------------------------------------
// ListeningSocket Tests
// -----------------------------------------------------------------------------

void test_privileged_port() {
    std::cout << "[Test] Binding to privileged port 80 (root required)..." << std::endl;
    ListeningSocket ls;
    try {
        ls.init(80);
        print_result("test_privileged_port", false); // Should not reach here if not root
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
        print_result("test_privileged_port", true);
    }
}

void test_double_bind() {
    std::cout << "[Test] Double bind on the same ListeningSocket instance..." << std::endl;
    ListeningSocket ls;
    try {
        ls.init(8081);
        ls.init(8082);
        print_result("test_double_bind", false); 
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
        print_result("test_double_bind", true);
    }
}

void test_invalid_port() {
    std::cout << "[Test] Binding to an out-of-range port (999999)..." << std::endl;
    ListeningSocket ls;
    try {
        // htons will overflow, but bind should still fail or behave predictably
        ls.init(999999);
        print_result("test_invalid_port", true); // bind might succeed with truncated port, but we check if it crashes
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        print_result("test_invalid_port", true);
    }
}

// -----------------------------------------------------------------------------
// ClientSocket Tests
// -----------------------------------------------------------------------------

void test_invalid_fd() {
    std::cout << "[Test] Initializing ClientSocket with FD -1..." << std::endl;
    try {
        ClientSocket cs(-1);
        print_result("test_invalid_fd", false);
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
        print_result("test_invalid_fd", true);
    }
}

void test_closed_fd() {
    std::cout << "[Test] Initializing ClientSocket with non-existent FD 9999..." << std::endl;
    try {
        ClientSocket cs(9999);
        print_result("test_closed_fd", false);
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception (fcntl should fail): " << e.what() << std::endl;
        print_result("test_closed_fd", true);
    }
}

void test_file_fd() {
    std::cout << "[Test] Initializing ClientSocket with a regular file FD..." << std::endl;
    int fd = open("test_file.txt", O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        std::cerr << "Failed to create test file" << std::endl;
        return;
    }
    try {
        ClientSocket cs(fd); // ClientSocket takes ownership and will close it
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags & O_NONBLOCK) {
            std::cout << "O_NONBLOCK set on regular file FD." << std::endl;
        }
        print_result("test_file_fd", true);
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        close(fd);
        print_result("test_file_fd", false);
    }
    unlink("test_file.txt");
}

int main() {
    std::cout << "=== STARTING SOCKET EDGE CASE TESTS ===\n" << std::endl;

    test_privileged_port();
    std::cout << std::endl;
    test_double_bind();
    std::cout << std::endl;
    test_invalid_port();
    std::cout << std::endl;
    test_invalid_fd();
    std::cout << std::endl;
    test_closed_fd();
    std::cout << std::endl;
    test_file_fd();

    std::cout << "\n=== TESTS COMPLETED ===" << std::endl;
    return 0;
}
