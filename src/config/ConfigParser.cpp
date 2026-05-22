// Copyright 2026 serjimen vja-nie dlesieur
#include "config/ConfigParser.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

ConfigParser::ConfigParser() {}

ConfigParser::ConfigParser(const std::string& filename) {
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    throw std::runtime_error("Error: Cannot open config file: " + filename);
  }

  std::string line;
  while (std::getline(file, line)) {
    _raw_lines.push_back(line);
  }
}

ConfigParser::ConfigParser(const ConfigParser& other)
    : _raw_lines(other._raw_lines) {}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
  if (this != &other) {
    _raw_lines = other._raw_lines;
  }
  return *this;
}

ConfigParser::~ConfigParser() {}

const std::vector<std::string>& ConfigParser::get_raw_lines() const {
  return _raw_lines;
}
