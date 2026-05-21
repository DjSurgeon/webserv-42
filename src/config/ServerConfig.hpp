// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_CONFIG_SERVERCONFIG_HPP_
#define SRC_CONFIG_SERVERCONFIG_HPP_

#include <string>
#include <vector>

#include "config/Context.hpp"
#include "config/LocationConfig.hpp"

class ServerConfig : public Context {
 public:
  // Orthodox Canonical Form
  ServerConfig();
  ServerConfig(const ServerConfig& other);
  ServerConfig& operator=(const ServerConfig& other);
  ~ServerConfig();

  // Getters
  int get_port() const;
  const std::string& get_host() const;
  const std::vector<std::string>& get_server_names() const;
  const std::vector<LocationConfig>& get_locations() const;

  // Setters
  void set_port(int port);
  void set_host(const std::string& host);
  void set_server_names(const std::vector<std::string>& server_names);
  void add_server_name(const std::string& server_name);
  void set_locations(const std::vector<LocationConfig>& locations);
  void add_location(const LocationConfig& loc);

 private:
  int _port;
  std::string _host;
  std::vector<std::string> _server_names;
  std::vector<LocationConfig> _locations;
};

#endif  // SRC_CONFIG_SERVERCONFIG_HPP_
