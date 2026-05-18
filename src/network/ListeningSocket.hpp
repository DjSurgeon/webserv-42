#ifndef LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

# include <sys/socket.h>
# include <unistd.h>
# include <stdexcept>

/**
 * @brief RAII class to manage a listening socket file descriptor.
 * 
 * This class ensures that a socket is created upon instantiation and
 * closed when the object is destroyed, preventing file descriptor leaks.
 * Copying is disabled to prevent double-close errors.
 */
class ListeningSocket {
public:
    ListeningSocket();
    ~ListeningSocket();

    int get_fd() const;

private:
    int _fd;

    // Prevent copying (C++98 style)
    ListeningSocket(const ListeningSocket& other);
    ListeningSocket& operator=(const ListeningSocket& other);
};

#endif
