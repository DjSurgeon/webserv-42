// Copyright 2026 serjimen vja-nie dlesieur
#include <stdexcept>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"
#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"

void ConfigParser::_handleLocationDirective(
    const std::vector<std::string>& tokens, size_t* i, ServerConfig* server) {
  LocationConfig loc;

  loc.setClientMaxBodySize(server->getClientMaxBodySize());
  loc.setAutoindex(server->getAutoindex());

  _parseLocationBlock(tokens, i, &loc);

  // Inherit context defaults
  if (loc.getRoot().empty())
    loc.setRoot(server->getRoot());
  if (loc.getIndexFiles().empty())
    loc.setIndexFiles(server->getIndexFiles());
  if (loc.getErrorPages().empty())
    loc.setErrorPages(server->getErrorPages());

  server->addLocation(loc);
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
