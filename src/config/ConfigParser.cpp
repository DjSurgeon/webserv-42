// Copyright 2026 serjimen vja-nie dlesieur
#include "config/ConfigParser.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"

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

void ConfigParser::_handleLocationDirective(
    const std::vector<std::string>& tokens, size_t* i, ServerConfig* server) {
  LocationConfig loc;
  // Inherit context defaults
  loc.setRoot(server->getRoot());
  loc.setIndexFiles(server->getIndexFiles());
  loc.setErrorPages(server->getErrorPages());
  loc.setClientMaxBodySize(server->getClientMaxBodySize());
  loc.setAutoindex(server->getAutoindex());

  _parseLocationBlock(tokens, i, &loc);
  server->addLocation(loc);
}

void ConfigParser::_handleListenDirective(
    const std::vector<std::string>& tokens, size_t* i, ServerConfig* server) {
  (*i)++;
  if (*i >= tokens.size() || tokens[*i] == ";") {
    throw std::runtime_error("Syntax error: incomplete 'listen' directive");
  }
  std::string listenVal = tokens[*i];
  std::string host = "127.0.0.1";
  std::string portStr;

  size_t colonPos = listenVal.find(':');
  if (colonPos != std::string::npos) {
    host = listenVal.substr(0, colonPos);
    portStr = listenVal.substr(colonPos + 1);
  } else {
    portStr = listenVal;
  }

  // Validate port string
  if (portStr.empty()) {
    throw std::runtime_error(
        "Syntax error: port missing in 'listen' directive");
  }
  for (size_t j = 0; j < portStr.length(); ++j) {
    if (!std::isdigit(static_cast<unsigned char>(portStr[j]))) {
      throw std::runtime_error("Syntax error: invalid port '" + portStr +
                               "' (must be digits)");
    }
  }

  long portVal = std::strtol(portStr.c_str(), NULL, 10);  // NOLINT
  if (portVal < 1 || portVal > 65535) {
    throw std::runtime_error("Syntax error: port '" + portStr +
                             "' out of range (1-65535)");
  }

  server->setHost(host);
  server->setPort(static_cast<int>(portVal));

  (*i)++;
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error("Syntax error: missing ';' after 'listen'");
  }
  (*i)++;
}

void ConfigParser::_handleServerNameDirective(
    const std::vector<std::string>& tokens, size_t* i, ServerConfig* server) {
  (*i)++;
  while (*i < tokens.size() && tokens[*i] != ";") {
    server->addServerName(tokens[*i]);
    (*i)++;
  }
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error("Syntax error: missing ';' after 'server_name'");
  }
  (*i)++;
}

void ConfigParser::_parseServerBlock(const std::vector<std::string>& tokens,
                                     size_t* i, ServerConfig* server) {
  if (!i || !server) {
    return;
  }
  (*i)++;  // Skip "server"
  if (*i >= tokens.size() || tokens[*i] != "{") {
    throw std::runtime_error("Syntax error: expected '{' after server");
  }
  (*i)++;  // Skip "{"

  while (*i < tokens.size() && tokens[*i] != "}") {
    const std::string& directive = tokens[*i];

    if (directive == "location") {
      _handleLocationDirective(tokens, i, server);
    } else if (directive == "listen") {
      _handleListenDirective(tokens, i, server);
    } else if (directive == "server_name") {
      _handleServerNameDirective(tokens, i, server);
    } else if (_parseContextDirectives(tokens, i, server)) {
      continue;
    } else {
      throw std::runtime_error("Syntax error: unknown directive '" + directive +
                               "' in server block");
    }
  }

  if (*i >= tokens.size() || tokens[*i] != "}") {
    throw std::runtime_error("Syntax error: missing '}' to close server block");
  }
  (*i)++;  // Skip "}"
}

void ConfigParser::_handleAllowedMethodsDirective(
    const std::vector<std::string>& tokens, size_t* i,
    LocationConfig* location) {
  (*i)++;
  while (*i < tokens.size() && tokens[*i] != ";") {
    location->addAllowedMethod(tokens[*i]);
    (*i)++;
  }
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error(
        "Syntax error: missing ';' after 'allowed_methods'");
  }
  (*i)++;
}

void ConfigParser::_handleCgiPathDirective(
    const std::vector<std::string>& tokens, size_t* i,
    LocationConfig* location) {
  (*i)++;
  if (*i >= tokens.size() || tokens[*i] == ";") {
    throw std::runtime_error("Syntax error: incomplete 'cgi_path'");
  }
  location->setCgiPath(tokens[*i]);
  (*i)++;
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error("Syntax error: missing ';' after 'cgi_path'");
  }
  (*i)++;
}

void ConfigParser::_handleCgiExtDirective(
    const std::vector<std::string>& tokens, size_t* i,
    LocationConfig* location) {
  (*i)++;
  while (*i < tokens.size() && tokens[*i] != ";") {
    location->addCgiExtension(tokens[*i]);
    (*i)++;
  }
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error("Syntax error: missing ';' after 'cgi_ext'");
  }
  (*i)++;
}

void ConfigParser::_handleRedirectDirective(
    const std::vector<std::string>& tokens, size_t* i,
    LocationConfig* location) {
  (*i)++;
  if (*i >= tokens.size() || tokens[*i] == ";") {
    throw std::runtime_error("Syntax error: incomplete 'redirect/return'");
  }
  location->setRedirect(tokens[*i]);
  (*i)++;
  if (*i >= tokens.size() || tokens[*i] != ";") {
    throw std::runtime_error("Syntax error: missing ';' after redirect");
  }
  (*i)++;
}

void ConfigParser::_parseLocationBlock(const std::vector<std::string>& tokens,
                                       size_t* i, LocationConfig* location) {
  if (!i || !location) {
    return;
  }
  (*i)++;  // Skip "location"
  if (*i >= tokens.size()) {
    throw std::runtime_error("Syntax error: missing path for location");
  }

  location->setPath(tokens[*i]);
  (*i)++;  // Skip path

  if (*i >= tokens.size() || tokens[*i] != "{") {
    throw std::runtime_error("Syntax error: expected '{' after location path");
  }
  (*i)++;  // Skip "{"

  while (*i < tokens.size() && tokens[*i] != "}") {
    const std::string& directive = tokens[*i];

    if (directive == "allowed_methods") {
      _handleAllowedMethodsDirective(tokens, i, location);
    } else if (directive == "cgi_path") {
      _handleCgiPathDirective(tokens, i, location);
    } else if (directive == "cgi_ext") {
      _handleCgiExtDirective(tokens, i, location);
    } else if (directive == "return" || directive == "redirect") {
      _handleRedirectDirective(tokens, i, location);
    } else if (_parseContextDirectives(tokens, i, location)) {
      continue;
    } else {
      throw std::runtime_error("Syntax error: unknown directive '" + directive +
                               "' in location block");
    }
  }

  if (*i >= tokens.size() || tokens[*i] != "}") {
    throw std::runtime_error(
        "Syntax error: missing '}' to close location block");
  }
  (*i)++;  // Skip "}"
}

bool ConfigParser::_parseContextDirectives(
    const std::vector<std::string>& tokens, size_t* i, Context* ctx) {
  if (!i || !ctx || *i >= tokens.size()) {
    return false;
  }

  const std::string& key = tokens[*i];

  if (key == "root") {
    if (*i + 2 >= tokens.size()) {
      throw std::runtime_error("Syntax error: incomplete 'root' directive");
    }
    if (tokens[*i + 2] != ";") {
      throw std::runtime_error("Syntax error: missing ';' after 'root' value");
    }
    ctx->setRoot(tokens[*i + 1]);
    *i += 3;
    return true;
  }

  if (key == "index") {
    (*i)++;
    while (*i < tokens.size() && tokens[*i] != ";") {
      ctx->addIndexFile(tokens[*i]);
      (*i)++;
    }
    if (*i >= tokens.size() || tokens[*i] != ";") {
      throw std::runtime_error(
          "Syntax error: missing ';' after 'index' values");
    }
    (*i)++;
    return true;
  }

  if (key == "autoindex") {
    if (*i + 2 >= tokens.size() || tokens[*i + 2] != ";") {
      throw std::runtime_error("Syntax error: invalid 'autoindex' directive");
    }
    const std::string& val = tokens[*i + 1];
    if (val == "on")
      ctx->setAutoindex(true);
    else if (val == "off")
      ctx->setAutoindex(false);
    else
      throw std::runtime_error("Syntax error: autoindex must be 'on' or 'off'");
    *i += 3;
    return true;
  }

  if (key == "error_page") {
    if (*i + 3 >= tokens.size() || tokens[*i + 3] != ";") {
      throw std::runtime_error("Syntax error: invalid 'error_page' directive");
    }
    int code = std::atoi(tokens[*i + 1].c_str());
    ctx->addErrorPage(code, tokens[*i + 2]);
    *i += 4;
    return true;
  }

  if (key == "client_max_body_size") {
    if (*i + 2 >= tokens.size()) {
      throw std::runtime_error(
          "Syntax error: incomplete 'client_max_body_size' directive");
    }
    if (tokens[*i + 2] != ";") {
      throw std::runtime_error(
          "Syntax error: missing ';' after 'client_max_body_size' value");
    }
    const std::string& value_str = tokens[*i + 1];

    for (size_t j = 0; j < value_str.length(); ++j) {
      if (!std::isdigit(static_cast<unsigned char>(value_str[j]))) {
        throw std::runtime_error(
            "Syntax error: invalid 'client_max_body_size' value (must be pure "
            "bytes)");
      }
    }

    size_t val = static_cast<size_t>(std::strtoul(value_str.c_str(), NULL, 10));
    ctx->setClientMaxBodySize(val);
    *i += 3;
    return true;
  }

  return false;
}

void ConfigParser::trimWhitespace(std::string* line) {
  if (!line) {
    return;
  }
  const std::string whitespace = " \t\r\n\v\f";
  size_t start = line->find_first_not_of(whitespace);

  if (start == std::string::npos) {
    line->clear();
    return;
  }

  size_t end = line->find_last_not_of(whitespace);
  *line = line->substr(start, end - start + 1);
}

void ConfigParser::removeComments(std::string* line) {
  if (!line) {
    return;
  }
  size_t pos = line->find('#');
  if (pos != std::string::npos) {
    line->erase(pos);
  }
}

std::vector<std::string> ConfigParser::tokenize(const std::string& line) {
  std::vector<std::string> tokens;
  std::string currentToken;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];

    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken.clear();
      }
    } else if (c == ';' || c == '{' || c == '}') {
      if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken.clear();
      }
      tokens.push_back(std::string(1, c));
    } else {
      currentToken += c;
    }
  }

  if (!currentToken.empty()) {
    tokens.push_back(currentToken);
  }

  return tokens;
}

void ConfigParser::parseDirective(Context* ctx, const std::string& line) {
  if (!ctx) {
    return;
  }
  std::vector<std::string> tokens;
  std::string currentToken;

  // Manual tokenization to split by spaces, but we don't need full parsing,
  // we just need the first word and the rest of the string before the
  // semicolon. Using the existing tokenize might be easier.
  std::vector<std::string> lineTokens = tokenize(line);

  if (lineTokens.empty()) {
    return;
  }

  if (lineTokens.back() != ";") {
    throw std::runtime_error(
        "Syntax error: missing ';' at the end of directive '" + line + "'");
  }

  std::string key = lineTokens[0];

  if (key == "root") {
    if (lineTokens.size() != 3) {
      throw std::runtime_error("Syntax error: invalid 'root' directive");
    }
    ctx->setRoot(lineTokens[1]);
  } else if (key == "client_max_body_size") {
    if (lineTokens.size() != 3) {
      throw std::runtime_error(
          "Syntax error: invalid 'client_max_body_size' directive");
    }
    const std::string& valueStr = lineTokens[1];
    for (size_t j = 0; j < valueStr.length(); ++j) {
      if (!std::isdigit(static_cast<unsigned char>(valueStr[j]))) {
        throw std::runtime_error(
            "Syntax error: invalid 'client_max_body_size' value (must be pure "
            "bytes)");
      }
    }
    size_t val = static_cast<size_t>(std::strtoul(valueStr.c_str(), NULL, 10));
    ctx->setClientMaxBodySize(val);
  }
}
