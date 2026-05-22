// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_CONFIG_CONFIGPARSER_HPP_
#define SRC_CONFIG_CONFIGPARSER_HPP_

#include <string>
#include <vector>

class ConfigParser {
 public:
  // Orthodox Canonical Form
  ConfigParser();
  explicit ConfigParser(const std::string& filename);
  ConfigParser(const ConfigParser& other);
  ConfigParser& operator=(const ConfigParser& other);
  ~ConfigParser();

  const std::vector<std::string>& get_raw_lines() const;

 private:
  std::vector<std::string> _raw_lines;

  static void trim_whitespace(std::string& line);
  static void remove_comments(std::string& line);
};

#endif  // SRC_CONFIG_CONFIGPARSER_HPP_
