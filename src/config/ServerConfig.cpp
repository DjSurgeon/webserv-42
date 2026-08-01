// Copyright 2026 raperez- serjimen
#include "config/ServerConfig.hpp"

#include <string>
#include <vector>

ServerConfig::ServerConfig() : Context(), _port(8080), _host("127.0.0.1") {}

ServerConfig::ServerConfig(const ServerConfig& other)
    : Context(other),
      _port(other._port),
      _host(other._host),
      _serverNames(other._serverNames),
      _locations(other._locations) {}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
  if (this != &other) {
    Context::operator=(other);
    _port = other._port;
    _host = other._host;
    _serverNames = other._serverNames;
    _locations = other._locations;
  }
  return *this;
}

ServerConfig::~ServerConfig() {}

int ServerConfig::getPort() const {
  return _port;
}

const std::string& ServerConfig::getHost() const {
  return _host;
}

const std::vector<std::string>& ServerConfig::getServerNames() const {
  return _serverNames;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
  return _locations;
}

void ServerConfig::setPort(int port) {
  _port = port;
}

void ServerConfig::setHost(const std::string& host) {
  _host = host;
}

void ServerConfig::setServerNames(const std::vector<std::string>& serverNames) {
  _serverNames = serverNames;
}

void ServerConfig::addServerName(const std::string& serverName) {
  _serverNames.push_back(serverName);
}

void ServerConfig::setLocations(const std::vector<LocationConfig>& locations) {
  _locations = locations;
}

void ServerConfig::addLocation(const LocationConfig& loc) {
  _locations.push_back(loc);
}

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
  const LocationConfig* best_match = NULL;
  size_t max_match_length = 0;

  for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
       it != _locations.end(); ++it) {
    const std::string& path = it->getPath();

    if (uri.find(path) == 0) {
      bool is_clean_match = false;

      if (path == "/" || path.length() == uri.length()) {
        is_clean_match = true;
      } else if (uri[path.length()] == '/') {
        is_clean_match = true;
      } else if (path.length() > 0 && path[path.length() - 1] == '/') {
        is_clean_match = true;
      }

      if (is_clean_match && path.length() > max_match_length) {
        max_match_length = path.length();
        best_match = &(*it);
      }
    }
  }

  return best_match;
}
