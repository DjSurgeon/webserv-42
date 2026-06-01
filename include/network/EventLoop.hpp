// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_NETWORK_EVENTLOOP_HPP_
#define SRC_NETWORK_EVENTLOOP_HPP_

#include <sys/poll.h>

#include <csignal>
#include <ctime>
#include <map>
#include <vector>

#include "config/ServerConfig.hpp"
#include "http/RequestParser.hpp"
#include "network/ClientSocket.hpp"

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
  void _dispatchRequest(const HttpRequest& req, const ServerConfig* server,
                        HttpResponse& res);
  bool _shouldCloseConnection(const HttpRequest& req) const;

  bool _handleRedirect(const LocationConfig* loc, HttpResponse& res) const;
  bool _isCgiRequest(const std::string& physicalPath,
                     const LocationConfig* loc) const;
  void _executeFileHandler(const HttpRequest& req, const Context* activeCtx,
                           const std::string& physicalPath,
                           HttpResponse& res) const;
};

#endif  // SRC_NETWORK_EVENTLOOP_HPP_
