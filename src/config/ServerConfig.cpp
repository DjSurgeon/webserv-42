// Copyright 2026 serjimen vja-nie dlesieur
#include "config/ServerConfig.hpp"

#include <string>
#include <vector>

ServerConfig::ServerConfig() : Context(), _port(8080), _host("127.0.0.1") {}

ServerConfig::ServerConfig(const ServerConfig& other)
    : Context(other),
      _port(other._port),
      _host(other._host),
      _server_names(other._server_names),
      _locations(other._locations) {}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
  if (this != &other) {
    Context::operator=(other);
    _port = other._port;
    _host = other._host;
    _server_names = other._server_names;
    _locations = other._locations;
  }
  return *this;
}

ServerConfig::~ServerConfig() {}

int ServerConfig::get_port() const {
  return _port;
}

const std::string& ServerConfig::get_host() const {
  return _host;
}

const std::vector<std::string>& ServerConfig::get_server_names() const {
  return _server_names;
}

const std::vector<LocationConfig>& ServerConfig::get_locations() const {
  return _locations;
}

void ServerConfig::set_port(int port) {
  _port = port;
}

void ServerConfig::set_host(const std::string& host) {
  _host = host;
}

void ServerConfig::set_server_names(
    const std::vector<std::string>& server_names) {
  _server_names = server_names;
}

void ServerConfig::add_server_name(const std::string& server_name) {
  _server_names.push_back(server_name);
}

void ServerConfig::set_locations(const std::vector<LocationConfig>& locations) {
  _locations = locations;
}

void ServerConfig::add_location(const LocationConfig& loc) {
  _locations.push_back(loc);
}
