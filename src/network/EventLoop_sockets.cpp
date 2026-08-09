// Copyright 2026 raperez- serjimen
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>

#include "network/EventLoop.hpp"

/**
 * @brief Adds a server socket to the monitoring vector.
 *
 * Instantiates a pollfd structure for the given file descriptor, configures
 * it to monitor incoming connections (POLLIN), and appends it to the vector.
 * Also registers the descriptor in the internal server list.
 *
 * @param fd The file descriptor of the server socket.
 */
void EventLoop::addServerSocket(int fd,
                                const std::vector<ServerConfig>& configs) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
  _serverFds.push_back(fd);
  _serverConfigs[fd] = configs;
}

/**
 * @brief Adds a client socket to the monitoring vector.
 *
 * Instantiates a pollfd structure for the given file descriptor, configures
 * it to simultaneously monitor for incoming data (POLLIN) and ability to
 * write data (POLLOUT), and appends it to the vector.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::addClientSocket(int fd) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN | POLLOUT;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
}

/**
 * @brief Removes a socket from the monitoring vector and safely closes it.
 *
 * Finds the descriptor within the _pollfds vector, executes close(fd) to
 * prevent resource leaks, and erases the element using std::vector::erase.
 * Also removes it from the server descriptors list if it was a server,
 * or deletes the associated ClientSocket object if it was a client.
 *
 * @param fd The file descriptor to remove.
 * @throw std::runtime_error If the file descriptor is not found in the loop.
 */
void EventLoop::removeSocket(int fd) {
  for (std::vector<pollfd>::iterator it = _pollfds.begin();
       it != _pollfds.end(); ++it) {
    if (it->fd == fd) {
      _pollfds.erase(it);

      // Remove from server list if present
      for (std::vector<int>::iterator sit = _serverFds.begin();
           sit != _serverFds.end(); ++sit) {
        if (*sit == fd) {
          _serverFds.erase(sit);
          _serverConfigs.erase(fd);
          return;
        }
      }

      // If not a server, it's a client. Remove from clients map and delete
      std::map<int, ClientSocket*>::iterator cit = _clients.find(fd);
      if (cit != _clients.end()) {
        delete cit->second;
        _clients.erase(cit);
      }
      std::map<int, RequestParser*>::iterator pit = _parsers.find(fd);
      if (pit != _parsers.end()) {
        delete pit->second;
        _parsers.erase(pit);
      }
      _clientToServer.erase(fd);

      return;
    }
  }

  throw std::runtime_error("EventLoop: Attempted to remove non-existent fd");
}

/**
 * @brief Checks if a given file descriptor belongs to a server socket.
 *
 * @param fd The file descriptor to check.
 * @return true If the file descriptor is in the server list.
 * @return false If the file descriptor is not in the server list.
 */
bool EventLoop::_isServerSocket(int fd) const {
  for (std::vector<int>::const_iterator it = _serverFds.begin();
       it != _serverFds.end(); ++it) {
    if (*it == fd) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Handles an incoming connection on a server socket.
 *
 * Calls accept() to receive the connection, instantiates a new ClientSocket
 * (which handles O_NONBLOCK automatically), stores it in the clients map,
 * and registers it for monitoring.
 *
 * @param server_fd The file descriptor of the server socket.
 */
void EventLoop::_handleNewConnection(int server_fd) {
  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd >= 0) {
    try {
      int flags = fcntl(client_fd, F_GETFD);
      if (flags != -1) {
          fcntl(client_fd, F_SETFD, flags | FD_CLOEXEC);
      }
      ClientSocket* new_client = new ClientSocket(client_fd);
      _clients[client_fd] = new_client;
      _parsers[client_fd] = new RequestParser();
      _clientToServer[client_fd] = server_fd;
      addClientSocket(client_fd);
    } catch (const std::exception& e) {
      std::cerr << "EventLoop: Failed to accept client: " << e.what() << "\n";
      close(client_fd);
    }
  }
}

/**
 * @brief Añade un FD de pipe CGI al vector _pollfds.
 *
 * @param fd File descriptor del pipe (lectura o escritura).
 * @param events Máscara de eventos (POLLIN o POLLOUT).
 */
void EventLoop::_addCgiFd(int fd, int16_t events) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = events;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
}

/**
 * @brief Elimina un FD de pipe CGI del vector _pollfds.
 * No llama a close(fd) directamente para dar flexibilidad al caller.
 *
 * @param fd File descriptor del pipe a remover.
 */
void EventLoop::_removeCgiFd(int fd) {
  for (std::vector<pollfd>::iterator it = _pollfds.begin();
       it != _pollfds.end(); ++it) {
    if (it->fd == fd) {
      _pollfds.erase(it);
      return;
    }
  }
}
