// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_CONFIG_CONFIGPARSER_HPP_
#define SRC_CONFIG_CONFIGPARSER_HPP_

#include <string>
#include <vector>

#include "config/ServerConfig.hpp"

class ConfigParser {
 public:
  // Orthodox Canonical Form
  ConfigParser();
  explicit ConfigParser(const std::string& filename);
  ConfigParser(const ConfigParser& other);
  ConfigParser& operator=(const ConfigParser& other);
  ~ConfigParser();

  const std::vector<std::string>& get_raw_lines() const;
  const std::vector<ServerConfig>& get_servers() const;

 private:
  std::vector<std::string> _raw_lines;
  std::vector<ServerConfig> _servers;

  void parse_tokens();
  std::vector<std::string> _flatten_tokens() const;
  void _parse_server_block(const std::vector<std::string>& tokens, size_t& i,
                           ServerConfig& server);
  void _parse_location_block(const std::vector<std::string>& tokens, size_t& i,
                             LocationConfig& location);

  static void trim_whitespace(std::string* line);
  static void remove_comments(std::string* line);
  static std::vector<std::string> tokenize(const std::string& line);
};

#endif  // SRC_CONFIG_CONFIGPARSER_HPP_
