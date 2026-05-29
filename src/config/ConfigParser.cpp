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
    removeComments(&line);
    trimWhitespace(&line);
    if (!line.empty()) {
      _rawLines.push_back(line);
    }
  }

  parseTokens();
}

ConfigParser::ConfigParser(const ConfigParser& other)
    : _rawLines(other._rawLines), _servers(other._servers) {}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
  if (this != &other) {
    _rawLines = other._rawLines;
    _servers = other._servers;
  }
  return *this;
}

ConfigParser::~ConfigParser() {}

const std::vector<std::string>& ConfigParser::getRawLines() const {
  return _rawLines;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const {
  return _servers;
}

std::vector<std::string> ConfigParser::_flattenTokens() const {
  std::vector<std::string> allTokens;
  for (size_t i = 0; i < _rawLines.size(); ++i) {
    std::vector<std::string> lineTokens = tokenize(_rawLines[i]);
    allTokens.insert(allTokens.end(), lineTokens.begin(), lineTokens.end());
  }
  return allTokens;
}

void ConfigParser::parseTokens() {
  std::vector<std::string> allTokens = _flattenTokens();
  size_t i = 0;

  while (i < allTokens.size()) {
    if (allTokens[i] == "server") {
      ServerConfig server;
      _parseServerBlock(allTokens, &i, &server);
      _servers.push_back(server);
    } else {
      throw std::runtime_error(
          "Syntax error: expected 'server' block at root level");
    }
  }
}
