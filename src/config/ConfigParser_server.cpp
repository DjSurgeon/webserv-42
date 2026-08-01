// Copyright 2026 raperez- serjimen
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

  std::string host, portStr;
  parseHostPort(tokens[*i], &host, &portStr);
  int port = validatePort(portStr);

  server->setHost(host);
  server->setPort(port);

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
