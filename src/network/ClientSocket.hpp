#ifndef CLIENT_SOCKET_HPP
# define CLIENT_SOCKET_HPP

# include <string>

/**
 * @brief RAII class to manage a client socket file descriptor.
 * 
 * This class encapsulates an accepted client file descriptor, ensures
 * proper resource cleanup (RAII), configures the socket to non-blocking
 * mode upon creation, and provides buffers for asynchronous operations.
 */
class ClientSocket {
public:
    ClientSocket(int client_fd);
    ~ClientSocket();

    int get_fd() const;
    std::string& get_read_buffer();
    std::string& get_write_buffer();

private:
    int _fd;
    std::string _read_buffer;
    std::string _write_buffer;

    // Prevent copying (C++98 style)
    ClientSocket(const ClientSocket& other);
    ClientSocket& operator=(const ClientSocket& other);
};

#endif
