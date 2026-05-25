// Copyright 2026 serjimen vja-nie dlesieur
#include "network/EventLoop.hpp"

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "handlers/CgiHandler.hpp"
#include "handlers/FileHandler.hpp"
#include "handlers/StaticRouter.hpp"

/**
 * @brief Default constructor for EventLoop.
 */
EventLoop::EventLoop() {}

/**
 * @brief Copy constructor for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 */
EventLoop::EventLoop(const EventLoop& other)
    : _pollfds(other._pollfds), _server_fds(other._server_fds) {
  // Clients are NOT deep copied because they manage unique file descriptors
  // and copying them violates the RAII strict ownership rules (double-free).
}

/**
 * @brief Copy assignment operator for EventLoop.
 *
 * @param other The EventLoop object to copy from.
 * @return EventLoop& Reference to the updated object.
 */
EventLoop& EventLoop::operator=(const EventLoop& other) {
  if (this != &other) {
    _pollfds = other._pollfds;
    _server_fds = other._server_fds;
    // Clients are NOT deep copied for the same RAII reasons.
  }
  return *this;
}

/**
 * @brief Destructor for EventLoop.
 *
 * Cleans up all client sockets allocated in memory to prevent leaks.
 */
EventLoop::~EventLoop() {
  for (std::map<int, ClientSocket*>::iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    delete it->second;
  }
  _clients.clear();
  for (std::map<int, RequestParser*>::iterator it = _parsers.begin();
       it != _parsers.end(); ++it) {
    delete it->second;
  }
  _parsers.clear();
}

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
  _server_fds.push_back(fd);
  _server_configs[fd] = configs;
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
      if (close(fd) == -1) {
        std::cerr << "EventLoop: Error closing socket FD " << fd << std::endl;
      }
      _pollfds.erase(it);

      // Remove from server list if present
      for (std::vector<int>::iterator sit = _server_fds.begin();
           sit != _server_fds.end(); ++sit) {
        if (*sit == fd) {
          _server_fds.erase(sit);
          _server_configs.erase(fd);
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
      _client_to_server.erase(fd);

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
  for (std::vector<int>::const_iterator it = _server_fds.begin();
       it != _server_fds.end(); ++it) {
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
void EventLoop::_handle_new_connection(int server_fd) {
  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd >= 0) {
    try {
      ClientSocket* new_client = new ClientSocket(client_fd);
      _clients[client_fd] = new_client;
      _parsers[client_fd] = new RequestParser();
      _client_to_server[client_fd] = server_fd;
      addClientSocket(client_fd);
    } catch (const std::exception& e) {
      std::cerr << "EventLoop: Failed to accept client: " << e.what() << "\n";
      close(client_fd);
    }
  }
}

/**
 * @brief Handles incoming data from a client socket.
 *
 * Executes recv() safely. If bytes are received, appends them to the client's
 * read buffer. If recv returns exactly 0 (EOF), disconnects the client.
 * Negative returns are ignored without consulting errno.
 *
 * Feed the parser byte by byte from the read buffer and generate a response
 * if the request is complete.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_data(int fd) {
  std::map<int, ClientSocket*>::iterator client_it = _clients.find(fd);
  std::map<int, RequestParser*>::iterator parser_it = _parsers.find(fd);
  if (client_it == _clients.end() || parser_it == _parsers.end()) {
    return;
  }

  char buffer[8192];
  int bytes = recv(fd, buffer, sizeof(buffer), 0);

  if (bytes > 0) {
    client_it->second->append_to_read_buffer(std::string(buffer, bytes));
    // Process the buffer with the parser
    const std::string& read_buf = client_it->second->get_read_buffer();
    size_t i = 0;
    while (i < read_buf.length()) {
      e_parser_state state = parser_it->second->feed(read_buf[i]);
      i++;
      if (state == STATE_COMPLETE) {
        std::cout << "EventLoop: Request completed from client " << fd << "\n";

        const HttpRequest& req = parser_it->second->get_request();
        HttpResponse res;

        // Find matched ServerConfig (Virtual Hosting based on Host header)
        int parent_server_fd = _client_to_server[fd];
        const std::vector<ServerConfig>& configs =
            _server_configs[parent_server_fd];
        const ServerConfig* matched_server = &configs[0];  // Default to first

        std::map<std::string, std::string>::const_iterator host_it =
            req.get_headers().find("host");
        if (host_it != req.get_headers().end()) {
          std::string host_value = host_it->second;
          size_t colon_pos = host_value.find(':');
          if (colon_pos != std::string::npos) {
            host_value = host_value.substr(0, colon_pos);  // Strip port
          }
          for (size_t k = 0; k < configs.size(); ++k) {
            const std::vector<std::string>& names =
                configs[k].get_server_names();
            bool matched = false;
            for (size_t n = 0; n < names.size(); ++n) {
              if (names[n] == host_value) {
                matched_server = &configs[k];
                matched = true;
                break;
              }
            }
            if (matched) break;
          }
        }

        const LocationConfig* matched_loc =
            matched_server->find_location(req.get_uri());
        std::string physical_path;
        StaticRouter router;

        bool route_ok = router.process_route(req, matched_server, matched_loc,
                                             &res, &physical_path);

        if (route_ok) {
          std::cout << "EventLoop: Resolved physical path: " << physical_path
                    << std::endl;

          bool is_cgi = false;
          if (matched_loc) {
            if (!matched_loc->get_cgi_path().empty()) {
              is_cgi = true;
            } else {
              std::string ext = "";
              size_t dot_pos = physical_path.find_last_of('.');
              if (dot_pos != std::string::npos) {
                ext = physical_path.substr(dot_pos);
              }
              const std::vector<std::string>& cgi_exts =
                  matched_loc->get_cgi_extensions();
              for (size_t k = 0; k < cgi_exts.size(); ++k) {
                if (ext == cgi_exts[k]) {
                  is_cgi = true;
                  break;
                }
              }
            }
          }

          if (is_cgi) {
            CgiHandler cgi;
            // For now, execute_script runs synchronously and might not read the
            // pipe depending on the current CgiHandler implementation, but it
            // triggers fork/exec.
            cgi.execute_script(physical_path, req, matched_loc, &res);
          } else {
            FileHandler file_handler;
            const std::string& method = req.get_method();

            if (method == "GET") {
              struct stat st;
              if (stat(physical_path.c_str(), &st) == 0 &&
                  S_ISDIR(st.st_mode)) {
                const Context* active_ctx =
                    matched_loc ? static_cast<const Context*>(matched_loc)
                                : static_cast<const Context*>(matched_server);
                if (active_ctx->get_autoindex()) {
                  file_handler.generate_autoindex(physical_path, req.get_uri(),
                                                  &res);
                } else {
                  file_handler.serve_file(physical_path, &res);
                }
              } else {
                file_handler.serve_file(physical_path, &res);
              }
            } else if (method == "DELETE") {
              file_handler.delete_file(physical_path, &res);
            } else {
              res.generate_error_response(405);
            }
          }
        }

        bool should_close = false;
        std::map<std::string, std::string>::const_iterator conn_it =
            req.get_headers().find("connection");
        if (conn_it != req.get_headers().end()) {
          // The RequestParser already converts keys to lowercase
          if (conn_it->second == "close") {
            should_close = true;
          }
        } else if (req.get_version() == "HTTP/1.0") {
          should_close = true;
        }
        client_it->second->set_should_close(should_close);

        std::string raw_response = res.to_string();
        client_it->second->append_to_write_buffer(raw_response);
        parser_it->second->reset();
        break;
      } else if (state == STATE_ERROR) {
        std::cerr << "EventLoop: Parser error from client " << fd << "\n";
        client_it->second->append_to_write_buffer(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "\r\n");
        client_it->second->set_should_close(true);
        break;
      }
    }
    client_it->second->consume_read_buffer(i);
  } else if (bytes == 0) {
    removeSocket(fd);
  }
}

/**
 * @brief Handles readiness to write data to a client socket.
 *
 * Flushes available data from the client's write buffer via send().
 * Successfully sent bytes are consumed from the buffer.
 * Negative returns are ignored without consulting errno.
 *
 * @param fd The file descriptor of the client socket.
 */
void EventLoop::_handle_client_write(int fd) {
  std::map<int, ClientSocket*>::iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return;
  }

  const std::string& data = it->second->get_write_buffer();
  if (!data.empty()) {
    int sent = send(fd, data.c_str(), data.length(), 0);
    if (sent > 0) {
      it->second->consume_write_buffer(sent);
    }
  }

  // Check if we should close the connection after sending all data
  if (it->second->get_write_buffer().empty() &&
      it->second->get_should_close()) {
    removeSocket(fd);
  }
}

/**
 * @brief Main infinite loop for polling events.
 *
 * Queries the Unix kernel about socket activity using poll() and dispatches
 * traffic to the corresponding private handlers based on the revents bitmasks.
 */
void EventLoop::run() {
  while (g_running) {
    if (_pollfds.empty()) {
      continue;
    }

    int ret = poll(&_pollfds[0], _pollfds.size(), POLL_TIMEOUT);

    if (ret < 0) {
      std::cerr << "EventLoop: poll() failed" << std::endl;
      continue;
    }

    if (ret == 0) {
      continue;
    }

    // Iterate backwards to allow safe removal of sockets during iteration
    for (int i = static_cast<int>(_pollfds.size()) - 1; i >= 0; --i) {
      int current_fd = _pollfds[i].fd;
      int16_t revents = _pollfds[i].revents;

      if (revents == 0) {
        continue;
      }

      // Check for errors or disconnection
      if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        removeSocket(current_fd);
        continue;
      }

      // Check for incoming data (or new connection)
      if (revents & POLLIN) {
        if (_isServerSocket(current_fd)) {
          _handle_new_connection(current_fd);
        } else {
          _handle_client_data(current_fd);
        }
      }

      // Prevent processing POLLOUT if the socket was closed during POLLIN
      if (!_isServerSocket(current_fd) &&
          _clients.find(current_fd) == _clients.end()) {
        continue;
      }

      // Check for readiness to write
      if (revents & POLLOUT) {
        if (!_isServerSocket(current_fd)) {
          _handle_client_write(current_fd);
        }
      }
    }
  }
}
