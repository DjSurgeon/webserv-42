// Copyright 2026 serjimen vja-nie dlesieur
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"

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
