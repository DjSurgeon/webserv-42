// Copyright 2026 serjimen vja-nie dlesieur
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"

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
