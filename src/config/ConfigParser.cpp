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
    remove_comments(&line);
    trim_whitespace(&line);
    if (!line.empty()) {
      _raw_lines.push_back(line);
    }
  }

  parse_tokens();
}

ConfigParser::ConfigParser(const ConfigParser& other)
    : _raw_lines(other._raw_lines), _servers(other._servers) {}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
  if (this != &other) {
    _raw_lines = other._raw_lines;
    _servers = other._servers;
  }
  return *this;
}

ConfigParser::~ConfigParser() {}

const std::vector<std::string>& ConfigParser::get_raw_lines() const {
  return _raw_lines;
}

const std::vector<ServerConfig>& ConfigParser::get_servers() const {
  return _servers;
}

std::vector<std::string> ConfigParser::_flatten_tokens() const {
  std::vector<std::string> all_tokens;
  for (size_t i = 0; i < _raw_lines.size(); ++i) {
    std::vector<std::string> line_tokens = tokenize(_raw_lines[i]);
    all_tokens.insert(all_tokens.end(), line_tokens.begin(), line_tokens.end());
  }
  return all_tokens;
}

void ConfigParser::parse_tokens() {
  std::vector<std::string> all_tokens = _flatten_tokens();
  size_t i = 0;

  while (i < all_tokens.size()) {
    if (all_tokens[i] == "server") {
      ServerConfig server;
      _parse_server_block(all_tokens, &i, &server);
      _servers.push_back(server);
    } else {
      throw std::runtime_error(
          "Syntax error: expected 'server' block at root level");
    }
  }
}

void ConfigParser::_parse_server_block(const std::vector<std::string>& tokens,
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
    if (tokens[*i] == "location") {
      LocationConfig loc;
      // Inherit context defaults
      loc.set_root(server->get_root());
      loc.set_index_files(server->get_index_files());
      loc.set_error_pages(server->get_error_pages());
      loc.set_client_max_body_size(server->get_client_max_body_size());
      loc.set_autoindex(server->get_autoindex());

      _parse_location_block(tokens, i, &loc);
      server->add_location(loc);
    } else if (_parse_context_directive(tokens, i, server)) {
      continue;
    } else {
      throw std::runtime_error("Syntax error: unknown directive '" + tokens[*i] +
                               "' in server block");
    }
  }

  if (*i >= tokens.size() || tokens[*i] != "}") {
    throw std::runtime_error("Syntax error: missing '}' to close server block");
  }
  (*i)++;  // Skip "}"
}

void ConfigParser::_parse_location_block(const std::vector<std::string>& tokens,
                                         size_t* i, LocationConfig* location) {
  if (!i || !location) {
    return;
  }
  (*i)++;  // Skip "location"
  if (*i >= tokens.size()) {
    throw std::runtime_error("Syntax error: missing path for location");
  }

  location->set_path(tokens[*i]);
  (*i)++;  // Skip path

  if (*i >= tokens.size() || tokens[*i] != "{") {
    throw std::runtime_error("Syntax error: expected '{' after location path");
  }
  (*i)++;  // Skip "{"

  while (*i < tokens.size() && tokens[*i] != "}") {
    if (_parse_context_directive(tokens, i, location)) {
      continue;
    }
    throw std::runtime_error("Syntax error: unknown directive '" + tokens[*i] +
                             "' in location block");
  }

  if (*i >= tokens.size() || tokens[*i] != "}") {
    throw std::runtime_error(
        "Syntax error: missing '}' to close location block");
  }
  (*i)++;  // Skip "}"
}

bool ConfigParser::_parse_context_directive(
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
    ctx->set_root(tokens[*i + 1]);
    *i += 3;
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
            "Syntax error: invalid 'client_max_body_size' value (must be pure bytes)");
      }
    }

    size_t val = static_cast<size_t>(std::strtoul(value_str.c_str(), NULL, 10));
    ctx->set_client_max_body_size(val);
    *i += 3;
    return true;
  }

  return false;
}

void ConfigParser::trim_whitespace(std::string* line) {
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

void ConfigParser::remove_comments(std::string* line) {
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
  std::string current_token;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];

    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!current_token.empty()) {
        tokens.push_back(current_token);
        current_token.clear();
      }
    } else if (c == ';' || c == '{' || c == '}') {
      if (!current_token.empty()) {
        tokens.push_back(current_token);
        current_token.clear();
      }
      tokens.push_back(std::string(1, c));
    } else {
      current_token += c;
    }
  }

  if (!current_token.empty()) {
    tokens.push_back(current_token);
  }

  return tokens;
}
