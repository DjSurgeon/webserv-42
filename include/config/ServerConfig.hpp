// Copyright 2026 serjimen vja-nie dlesieur
#ifndef INCLUDE_CONFIG_SERVERCONFIG_HPP_
#define INCLUDE_CONFIG_SERVERCONFIG_HPP_

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
  int getPort() const;
  const std::string& getHost() const;
  const std::vector<std::string>& getServerNames() const;
  const std::vector<LocationConfig>& getLocations() const;
  const LocationConfig* findLocation(const std::string& uri) const;

  // Setters
  void setPort(int port);
  void setHost(const std::string& host);
  void setServerNames(const std::vector<std::string>& serverNames);
  void addServerName(const std::string& serverName);
  void setLocations(const std::vector<LocationConfig>& locations);
  void addLocation(const LocationConfig& loc);

 private:
  int _port;
  std::string _host;
  std::vector<std::string> _serverNames;
  std::vector<LocationConfig> _locations;
};

#endif  // INCLUDE_CONFIG_SERVERCONFIG_HPP_
