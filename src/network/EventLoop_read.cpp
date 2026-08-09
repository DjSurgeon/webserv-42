// Copyright 2026 raperez- serjimen
#include <sys/socket.h>
#include <sys/stat.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "handlers/AuthHandler.hpp"
#include "handlers/CgiHandler.hpp"
#include "handlers/FileHandler.hpp"
#include "handlers/StaticRouter.hpp"
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

  if (method == "GET" || method == "HEAD") {
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

  char buffer[65536];
  int bytes = recv(fd, buffer, sizeof(buffer), 0);

  if (bytes > 0) {
    client_it->second->appendToReadBuffer(buffer, bytes);

    while (true) {
      const std::string& read_buf = client_it->second->getReadBuffer();
      if (read_buf.empty()) break;

      size_t consumed = 0;
      e_parser_state state = parser_it->second->feed_buffer(
          read_buf.c_str(), read_buf.length(), consumed);

      client_it->second->consumeReadBuffer(consumed);

      if (state == STATE_HEADERS_COMPLETE) {
        const HttpRequest& req = parser_it->second->get_request();
        std::string host_header = "";
        std::map<std::string, std::string>::const_iterator host_it =
            req.get_headers().find("host");
        if (host_it != req.get_headers().end()) {
          host_header = host_it->second;
        }

        const ServerConfig* matched_server =
            _resolveServerConfig(fd, host_header);

        if (matched_server) {
          const LocationConfig* matched_loc =
              matched_server->findLocation(req.get_uri());
          const Context* active_ctx =
              matched_loc ? static_cast<const Context*>(matched_loc)
                          : static_cast<const Context*>(matched_server);

          std::map<std::string, std::string>::const_iterator cl_it =
              req.get_headers().find("content-length");
          if (cl_it != req.get_headers().end()) {
            size_t cl = 0;
            std::stringstream ss(cl_it->second);
            if (ss >> cl && cl > active_ctx->getClientMaxBodySize()) {
              std::cerr << "EventLoop: Payload too large from client " << fd
                        << "\n";
              client_it->second->appendToWriteBuffer(
                  "HTTP/1.1 413 Payload Too Large\r\n"
                  "Connection: close\r\n"
                  "\r\n");
              client_it->second->setShouldClose(true);
              break;
            }
          }
        }
        parser_it->second->resume_body_parsing();
        continue;
      }

      if (state == STATE_COMPLETE) {
        std::cout << "EventLoop: Request completed from client " << fd << "\n";

        HttpRequest& req = parser_it->second->get_request_mut();

        std::string host_header = "";
        std::map<std::string, std::string>::const_iterator host_it =
            req.get_headers().find("host");
        if (host_it != req.get_headers().end()) {
          host_header = host_it->second;
        }

        const ServerConfig* matched_server =
            _resolveServerConfig(fd, host_header);

        _dispatchRequest(fd, req, matched_server);

        bool should_close = _shouldCloseConnection(req);
        client_it->second->setShouldClose(should_close);

        parser_it->second->reset();
        // Continue loop to parse next request in buffer
      } else if (state == STATE_ERROR) {
        std::cerr << "EventLoop: Parser error from client " << fd << "\n";
        client_it->second->appendToWriteBuffer(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "\r\n");
        client_it->second->setShouldClose(true);
        break;
      } else {
        // Need more data (STATE_BODY, STATE_HEADER, etc.)
        break;
      }
    }
  } else if (bytes == 0) {
    removeSocket(fd);
  }
}

void EventLoop::_dispatchRequest(int client_fd, HttpRequest& req,
                                 const ServerConfig* matched_server) {
  if (!matched_server) return;

  const LocationConfig* matched_loc =
      matched_server->findLocation(req.get_uri());
  const Context* active_ctx = matched_loc
                                  ? static_cast<const Context*>(matched_loc)
                                  : static_cast<const Context*>(matched_server);

  HttpResponse res;

  // Manejo de Redirección
  if (_handleRedirect(matched_loc, res)) {
    if (req.get_method() == "HEAD") res.set_body("");
    std::string res_str = res.to_string();
    _clients[client_fd]->swapWriteBuffer(res_str);
    return;
  }

  // Manejo de Autenticación
  AuthHandler auth;
  if (auth.handle_auth_request(req.get_uri(), req, &res)) {
    if (req.get_method() == "HEAD") res.set_body("");
    std::string res_str = res.to_string();
    _clients[client_fd]->swapWriteBuffer(res_str);
    return;
  }

  std::string physical_path;
  StaticRouter router;
  if (!router.process_route(req, matched_server, matched_loc, &res,
                            &physical_path, active_ctx)) {
    if (req.get_method() == "HEAD") res.set_body("");
    std::string res_str = res.to_string();
    _clients[client_fd]->swapWriteBuffer(res_str);
    return;
  }

  if (_isCgiRequest(physical_path, matched_loc)) {
    CgiHandler cgi;
    CgiProcess proc;
    if (cgi.start_script(physical_path, req, matched_loc, proc, _cgiOutMap,
                         _cgiInMap)) {
      CgiTask* task = new CgiTask();
      task->client_fd = client_fd;
      task->pipe_in_fd = proc.pipe_in;
      task->pipe_out_fd = proc.pipe_out;
      task->pid = proc.pid;
      task->start_time = std::time(NULL);
      req.take_body(task->body_to_write);
      task->bytes_written = 0;
      task->loc = matched_loc;

      _clientCgiMap[client_fd] = task;
      _cgiOutMap[proc.pipe_out] = task;
      _addCgiFd(proc.pipe_out, POLLIN);

      if (proc.pipe_in != -1) {
        _cgiInMap[proc.pipe_in] = task;
        _addCgiFd(proc.pipe_in, POLLOUT);
      }
    } else {
      res.generate_error_response(500, active_ctx, &req);
      if (req.get_method() == "HEAD") res.set_body("");
      std::string res_str = res.to_string();
      _clients[client_fd]->swapWriteBuffer(res_str);
    }
  } else {
    _executeFileHandler(req, active_ctx, physical_path, res);
    if (req.get_method() == "HEAD") res.set_body("");
    std::string res_str = res.to_string();
    _clients[client_fd]->swapWriteBuffer(res_str);
  }
}

void EventLoop::_handleCgiWrite(int fd) {
  CgiTask* task = _cgiInMap[fd];
  size_t remaining = task->body_to_write.size() - task->bytes_written;
  ssize_t bytes =
      write(fd, task->body_to_write.data() + task->bytes_written, remaining);

  if (bytes > 0) {
    task->bytes_written += bytes;
    if (task->bytes_written >= task->body_to_write.size()) {
      _removeCgiFd(fd);
      close(fd);
      _cgiInMap.erase(fd);
      task->pipe_in_fd = -1;
    }
  } else {
    _removeCgiFd(fd);
    close(fd);
    _cgiInMap.erase(fd);
    task->pipe_in_fd = -1;
  }
}

void EventLoop::_handleCgiRead(int fd) {
  CgiTask* task = _cgiOutMap[fd];
  char buffer[65536];
  ssize_t bytes = read(fd, buffer, sizeof(buffer));

  if (bytes > 0) {
    task->cgi_output.append(buffer, bytes);
  } else {  // EOF detectado
    _finishCgiTask(task, false);
  }
}

void EventLoop::_finishCgiTask(CgiTask* task, bool timed_out) {
  if (task->pipe_in_fd != -1) {
    _removeCgiFd(task->pipe_in_fd);
    close(task->pipe_in_fd);
    _cgiInMap.erase(task->pipe_in_fd);
  }
  if (task->pipe_out_fd != -1) {
    _removeCgiFd(task->pipe_out_fd);
    close(task->pipe_out_fd);
    _cgiOutMap.erase(task->pipe_out_fd);
  }

  if (timed_out) {
    kill(task->pid, SIGKILL);
  }
  waitpid(task->pid, NULL, WNOHANG);

  if (_clients.count(task->client_fd)) {
    HttpResponse res;
    if (timed_out) {
      res.generate_error_response(504, task->loc);
    } else {
      CgiHandler cgi;
      if (!cgi.parse_cgi_output(task->cgi_output, &res)) {
        res.generate_error_response(502, task->loc);
      }
    }
    std::string res_str = res.to_string();
    _clients[task->client_fd]->swapWriteBuffer(res_str);
  }

  _clientCgiMap.erase(task->client_fd);
  delete task;
}

void EventLoop::_checkCgiTimeouts() {
  time_t now = std::time(NULL);
  std::map<int, CgiTask*>::iterator it = _clientCgiMap.begin();
  while (it != _clientCgiMap.end()) {
    CgiTask* task = it->second;
    ++it;
    if (now - task->start_time > 10) {  // Timeout de 10 segs
      _finishCgiTask(task, true);
    }
  }
}
