// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_NETWORK_EVENTLOOP_HPP_
#define INCLUDE_NETWORK_EVENTLOOP_HPP_

#include <stdint.h>
#include <sys/poll.h>
#include <sys/wait.h>

#include <csignal>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "config/ServerConfig.hpp"
#include "handlers/CgiHandler.hpp"
#include "http/RequestParser.hpp"
#include "network/ClientSocket.hpp"

struct CgiTask {
  int client_fd;
  int pipe_in_fd;
  int pipe_out_fd;
  pid_t pid;
  time_t start_time;
  std::string body_to_write;
  size_t bytes_written;
  std::string cgi_output;
  const LocationConfig* loc;
};

extern volatile sig_atomic_t g_running;

class HttpRequest;
class HttpResponse;

class EventLoop {
 public:
  EventLoop();
  EventLoop(const EventLoop& other);
  EventLoop& operator=(const EventLoop& other);
  ~EventLoop();

  void addServerSocket(int fd, const std::vector<ServerConfig>& configs);
  void addClientSocket(int fd);
  void removeSocket(int fd);
  void setSessionCleanupInterval(time_t interval);
  void run();

 private:
  // Mapas de FDs a CgiTask
  std::map<int, CgiTask*> _cgiOutMap;     // pipe_out_fd -> CgiTask
  std::map<int, CgiTask*> _cgiInMap;      // pipe_in_fd -> CgiTask
  std::map<int, CgiTask*> _clientCgiMap;  // client_fd -> CgiTask

  void _addCgiFd(int fd, int16_t events);
  void _removeCgiFd(int fd);
  void _handleCgiRead(int fd);
  void _handleCgiWrite(int fd);
  void _checkCgiTimeouts();
  void _finishCgiTask(CgiTask* task, bool timed_out);
  std::vector<pollfd> _pollfds;
  std::vector<int> _serverFds;
  std::map<int, ClientSocket*> _clients;
  std::map<int, RequestParser*> _parsers;
  std::map<int, std::vector<ServerConfig> > _serverConfigs;
  std::map<int, int> _clientToServer;

  static const int POLL_TIMEOUT = 1000;

  time_t _lastSessionCleanup;
  time_t _sessionCleanupInterval;

  bool _isServerSocket(int fd) const;
  void _handleNewConnection(int serverFd);
  void _handleClientData(int fd);
  void _handleClientWrite(int fd);

  const ServerConfig* _resolveServerConfig(int clientFd,
                                           const std::string& hostHeader) const;
  void _dispatchRequest(int client_fd, const HttpRequest& req,
                        const ServerConfig* matched_server);
  bool _shouldCloseConnection(const HttpRequest& req) const;

  bool _handleRedirect(const LocationConfig* loc, HttpResponse& res) const;
  bool _isCgiRequest(const std::string& physicalPath,
                     const LocationConfig* loc) const;
  void _executeFileHandler(const HttpRequest& req, const Context* activeCtx,
                           const std::string& physicalPath,
                           HttpResponse& res) const;
};

#endif  // INCLUDE_NETWORK_EVENTLOOP_HPP_

struct CgiState {
  int client_fd;    // FD del cliente que espera respuesta
  int pipe_in_fd;   // FD para ESCRIBIR el body hacia el CGI (STDOUT del server
                    // -> STDIN del CGI)
  int pipe_out_fd;  // FD para LEER la salida del CGI (STDOUT del CGI -> STDIN
                    // del server)
  pid_t cgi_pid;    // PID del proceso hijo
  time_t start_time;  // Para gestionar timeouts

  std::string body_to_write;  // Body de la request a enviar al CGI (ej. POST)
  size_t bytes_written;
  std::string cgi_output;  // Salida leída del CGI acumulada
};
