// Copyright 2026 raperez- serjimen
#include <sys/socket.h>
#include <sys/stat.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "handlers/CgiHandler.hpp"
#include "handlers/FileHandler.hpp"
#include "handlers/StaticRouter.hpp"
#include "handlers/AuthHandler.hpp"
#include "network/EventLoop.hpp"

const ServerConfig* EventLoop::_resolveServerConfig(
    int client_fd, const std::string& host_header) const {
  std::map<int, int>::const_iterator parent_it =
      _clientToServer.find(client_fd);
  if (parent_it == _clientToServer.end()) {
    return NULL;
  }

  int parent_server_fd = parent_it->second;
  std::map<int, std::vector<ServerConfig> >::const_iterator configs_it =
      _serverConfigs.find(parent_server_fd);
  if (configs_it == _serverConfigs.end() || configs_it->second.empty()) {
    return NULL;
  }

  const std::vector<ServerConfig>& configs = configs_it->second;
  const ServerConfig* matched_server = &configs[0];  // Default to first

  std::string host_value = host_header;
  size_t colon_pos = host_value.find(':');
  if (colon_pos != std::string::npos) {
    host_value = host_value.substr(0, colon_pos);  // Strip port
  }

  for (size_t k = 0; k < configs.size(); ++k) {
    const std::vector<std::string>& names = configs[k].getServerNames();
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

  return matched_server;
}

bool EventLoop::_handleRedirect(const LocationConfig* loc,
                                HttpResponse& res) const {
  if (loc && !loc->getRedirect().empty()) {
    res.set_status(301, "Moved Permanently");
    res.add_header("Location", loc->getRedirect());
    std::string body =
        "<html><body><h1>301 Moved Permanently</h1></body></html>";
    res.set_body(body);
    std::stringstream ss;
    ss << body.length();
    res.add_header("Content-Type", "text/html");
    res.add_header("Content-Length", ss.str());
    return true;
  }
  return false;
}

bool EventLoop::_isCgiRequest(const std::string& physical_path,
                              const LocationConfig* loc) const {
  if (!loc) return false;

  const std::vector<std::string>& cgi_exts = loc->getCgiExtensions();

  if (cgi_exts.empty()) return false;

  std::size_t dot_pos = physical_path.find_last_of('.');
  if (dot_pos == std::string::npos) return false;

  std::string ext = physical_path.substr(dot_pos);

  for (std::size_t i = 0; i < cgi_exts.size(); ++i) {
    if (ext == cgi_exts[i]) return true;
  }

  return false;
}

void EventLoop::_executeFileHandler(const HttpRequest& req,
                                    const Context* active_ctx,
                                    const std::string& physical_path,
                                    HttpResponse& res) const {
  FileHandler file_handler;
  const std::string& method = req.get_method();

  if (method == "GET") {
    struct stat st;
    if (stat(physical_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
      if (active_ctx->getAutoindex()) {
        file_handler.generate_autoindex(physical_path, req.get_uri(), &res,
                                        active_ctx, &req);
      } else {
        file_handler.serve_file(physical_path, &res, active_ctx, &req);
      }
    } else {
      file_handler.serve_file(physical_path, &res, active_ctx, &req);
    }
  } else if (method == "POST") {
    file_handler.upload_file(physical_path, &res, active_ctx, &req);
  } else if (method == "DELETE") {
    file_handler.delete_file(physical_path, &res, active_ctx, &req);
  } else {
    res.generate_error_response(405, active_ctx, &req);
  }
}

void EventLoop::_dispatchRequest(const HttpRequest& req,
                                 const ServerConfig* matched_server,
                                 HttpResponse& res) {
  if (!matched_server) {
    return;
  }

  const LocationConfig* matched_loc =
      matched_server->findLocation(req.get_uri());
  const Context* active_ctx = matched_loc
                                  ? static_cast<const Context*>(matched_loc)
                                  : static_cast<const Context*>(matched_server);

  if (_handleRedirect(matched_loc, res)) {
    return;
  }

  std::string physical_path;
  StaticRouter router;

  AuthHandler auth;
  if (auth.handle_auth_request(req.get_uri(), req, &res)) {
    return;
  }

  if (!router.process_route(req, matched_server, matched_loc, &res,
                            &physical_path, active_ctx)) {
    return;
  }

  std::cout << "EventLoop: Resolved physical path: " << physical_path
            << std::endl;

  if (_isCgiRequest(physical_path, matched_loc)) {
    CgiHandler cgi;
    cgi.execute_script(physical_path, req, matched_loc, &res);
  } else {
    _executeFileHandler(req, active_ctx, physical_path, res);
  }
}

bool EventLoop::_shouldCloseConnection(const HttpRequest& req) const {
  std::map<std::string, std::string>::const_iterator conn_it =
      req.get_headers().find("connection");
  if (conn_it != req.get_headers().end()) {
    // The RequestParser already converts keys to lowercase
    if (conn_it->second == "close") {
      return true;
    }
  } else if (req.get_version() == "HTTP/1.0") {
    return true;
  }
  return false;
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
void EventLoop::_handleClientData(int fd) {
  std::map<int, ClientSocket*>::iterator client_it = _clients.find(fd);
  std::map<int, RequestParser*>::iterator parser_it = _parsers.find(fd);
  if (client_it == _clients.end() || parser_it == _parsers.end()) {
    return;
  }

  char buffer[8192];
  int bytes = recv(fd, buffer, sizeof(buffer), 0);
  // std::cout << "Received " << bytes << " bytes:\n";
  // std::cout.write(buffer, bytes);
  // std::cout << "\n----------------------\n";

  if (bytes > 0) {
    client_it->second->appendToReadBuffer(std::string(buffer, bytes));
    // Process the buffer with the parser
    const std::string& read_buf = client_it->second->getReadBuffer();
    size_t i = 0;
    while (i < read_buf.length()) {
      e_parser_state state = parser_it->second->feed(read_buf[i]);
      i++;
      if (state == STATE_COMPLETE) {
        std::cout << "EventLoop: Request completed from client " << fd << "\n";

        const HttpRequest& req = parser_it->second->get_request();
        HttpResponse res;

        std::string host_header = "";
        std::map<std::string, std::string>::const_iterator host_it =
            req.get_headers().find("host");
        if (host_it != req.get_headers().end()) {
          host_header = host_it->second;
        }

        const ServerConfig* matched_server =
            _resolveServerConfig(fd, host_header);

        _dispatchRequest(req, matched_server, res);

        bool should_close = _shouldCloseConnection(req);
        client_it->second->setShouldClose(should_close);

        if (req.get_method() == "HEAD") {
          res.set_body("");
        }

        std::string raw_response = res.to_string();
        client_it->second->appendToWriteBuffer(raw_response);
        parser_it->second->reset();
        break;
      } else if (state == STATE_ERROR) {
        std::cerr << "EventLoop: Parser error from client " << fd << "\n";
        client_it->second->appendToWriteBuffer(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "\r\n");
        client_it->second->setShouldClose(true);
        break;
      }
    }
    client_it->second->consumeReadBuffer(i);
  } else if (bytes == 0) {
    removeSocket(fd);
  }
}
